#include <stdio.h>
#include <xenon_soc/xenon_power.h>
#include <xenon_soc/xenon_io.h>
#include <ppc/register.h>
#include <time/time.h>
#include <pci/io.h>

extern volatile int wait[3];
extern volatile int secondary_lock, secondary_alive;

static volatile int thread_state[6];

void xenon_yield(void)
{
}

static inline void wakeup_secondary(void)
{
	mtspr(152, 0xc00000);  // CTRL.TE{0,1} = 11
}

void wakeup_cpus(void)
{
	void *irq_cntrl = (void*)0x20050000;

	std(irq_cntrl + 0x2070, 0x7c);
	std(irq_cntrl + 0x2008, 0);
	std(irq_cntrl + 0x2000, 4);
	
	std(irq_cntrl + 0x4070, 0x7c);
	std(irq_cntrl + 0x4008, 0);
	std(irq_cntrl + 0x4000, 0x10);
	
	std(irq_cntrl + 0x10, 0x140078);
	wakeup_secondary();
}

void prepare_sleep(uint64_t *save)
{
	void *irq = (void*)0x20050000;
	int id = mfspr(1023); // PIR

	std(irq + id * 0x1000 + 8, 0x7c);
	mtdec(0x7fffffff);
	if (!(id & 1))
		while (mfspr(136) & (1 << 22)); /* wait until second thread dies */
	mtspr(310, 0x7fffffff);
	mtspr(921, (mfspr(921) & ~0x700000) | ((id & 1) ? 0 : (1<<20))); // WEXT for secondary threads
}

extern void cpusleep(void), secondary_wakeup(void);

void xenon_thread_sleep(void)
{
	int pir = mfspr(1023);
	uint64_t save[6];
	prepare_sleep(save);
	thread_state[pir] = 1;
	cpusleep();
	thread_state[pir] = 0;
	if (!(pir & 1))
		wakeup_secondary();
}

void xenon_set_speed(int new_speed)
{
	void *cpuregs = (void*)0x20061000;
	void *cpuregs2 = (void*)0x20060000;
	void *irq = (void*)0x20050000;

	std(cpuregs2 + 0xb58, ld(cpuregs2 + 0xb58) | 0x8000000000000000ULL);
	std(cpuregs + 0x50, ld(cpuregs + 0x50) &~ 0x200000000ULL);
	std(cpuregs + 0x60, ld(cpuregs + 0x60) | (1ULL << 33));

//	write32(0xe1020004, read32(0xe1020004) | 1);

	udelay(1);

	std(irq + 0x6020, 0x1047c);
	while (ld(irq + 0x6020) & 0x2000);
	std(irq + 8, 0x78);

	uint64_t v = ld(cpuregs + 0x188);

	int base_vid = (v >> 48) & 0x3F;
	
	v &= ~0xC007ULL;
	v |= new_speed & 7;
	v |= 0xFF0000;
	v |= 8;
	std(cpuregs + 0x188, v);
	
	v = ld(cpuregs + 0x188);
	
	/*
		The CPU VID outputs connect directly to the 
		ADP3190A chip. This allows a Vcore voltage
		from 0.83V to 1.6V.

		The formula to calculate the output voltage is:

		if VID <= 0x14:
			Vcore = 0.8375V + (0x14 - VID) * 0.0125V
		else:
			Vcore = 1.1000V + (0x3D - VID) * 0.0125V

		To simplify this, we define a "normalized VID"

		if VID < 0x15:
			nVID = VID + 0x3E
		else:
			nVID = VID

		Each chip has a (fuse-burned?) VID. An offset to 
		that VID is stored in the configuration data, and 
		is added to this value.

		nVID_final = nVID_fused + VID_delta

		Then nVID has to be converted back to a VID, and 
		written.

		An example from a box running the KK hack:

		The 0x61188 value says: 382c00000000b001

		The "Base VID" (=VID_fused) here is 0x2C, 
		the "current VID" (=VID_final) is 0x30.
		The speed ratio is 1 (=full speed).

		The Delta data stored in the flash config is 

		Delta = 0x80, 0x84

		So:

		Current_VID = Base_VID + Delta[1] - 0x80

		It seems that the CPU runs stable with much less voltage, but
		this would require a bit more effort to make this sure.
	*/

	int vlt = 11000 + (0x3D - ((base_vid < 0x15) ? base_vid + 0x3E : base_vid)) * 125;

	printf(" * default VID: %02x (%d.%04dV)\n", base_vid, vlt / 10000, vlt % 10000);
	
	if (base_vid < 0x15)
		base_vid += 0x3e;
	
	int delta = 0x84; /* should come from config area */

	int new_vid = base_vid + delta - 0x80;
	
	if (new_vid >= 0x3e)
		new_vid -= 0x3e;

	vlt = 11000 + (0x3D - ((new_vid < 0x15) ? new_vid + 0x3E : new_vid)) * 125;
	
	printf(" * using new VID: %02x (%d.%04dV)\n", new_vid, vlt / 10000, vlt % 10000);

	v &= ~0xBF08ULL;
	v |= new_vid << 8;
	v |= 0x4000;
	v |= 0xFF << 24;

	std(cpuregs + 0x188, v);

	cpusleep();

	std(cpuregs + 0x188, ld(cpuregs + 0x188) | 0x8000);

	std(irq + 0x50, ld(irq + 0x60));
	std(irq + 8, 0x7c);
	std(irq + 0x6020, 0);
}

int xenon_run_thread_task(int thread, void *stack, void *task)
{
	if (wait[thread * 2])
		return -1; // busy
	wait[thread * 2 + 1] = (int)stack;
	wait[thread * 2] = (int)task;
	return 0;
}

static unsigned char stack[5 * 0x1000];

void xenon_make_it_faster(int speed)
{
	if (secondary_alive != 0x3f)
		xenon_thread_startup();
	printf(" * Make it faster by making it sleep...\n");
	int i;
	for (i = 1; i < 6; ++i)
		while (xenon_run_thread_task(i, stack + i * 0x1000 - 0x100, xenon_thread_sleep));

	for (i = 1; i < 6; ++i)
		while (!thread_state[i])
			xenon_yield();

	uint64_t save[6];
	prepare_sleep(save);

	printf(" * Make it faster by making it consume more power...\n");

		/* TODO: we need to get the VID from the config block, and set it. */
	xenon_set_speed(speed);

	printf(" * Make it faster by awaking the other cores...\n");
	wakeup_cpus();

	for (i = 1; i < 6; ++i)
		while (thread_state[i])
			xenon_yield();
	printf(" * fine, they all came back.\n");
}

void xenon_thread_startup(void)
{
	secondary_lock = 4 | 16; /* start primary threads */
	while (secondary_alive != 0x3f); /* wait until all are alive */
}

void xenon_sleep_thread(int thread)
{
	xenon_run_thread_task(thread, stack + thread * 0x1000 - 0x100, xenon_thread_sleep);
}
