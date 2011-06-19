#include <pci/io.h>
#include <string.h>
#include <malloc.h>
#include <xenon_smc/xenon_smc.h>
#include <xenon_smc/xenon_gpio.h>
#include <stdio.h>
#include <ppc/cache.h>

static int snd_base = 0xea001600, wptr, buffer_len;
static void *buffer;

void xenon_sound_init(void)
{
	// reset DAC (init from scratch)
	xenon_gpio_control(5,0x1000,0x1000);
	xenon_gpio_control(0,0x1000,0x1000);
	xenon_gpio_control(4,0x1000,0x1000);
	
	static unsigned char smc_snd[32] = {0x8d, 1, 1};
	xenon_smc_send_message(smc_snd);

	uint32_t *descr = memalign(256, 0x20 * 8);
	int descr_base = ((int)descr) & 0x1FFFFFFF;
	
	buffer_len = 64*1024;
	buffer = memalign(256, buffer_len);
	memset(buffer, 0, buffer_len);
	memdcbst(buffer, buffer_len);
	
	int buffer_base = ((int)buffer) & 0x1fffffff;
	
	int i;
	for (i = 0; i < 0x20; ++i)
	{
		descr[i * 2] = __builtin_bswap32(buffer_base + (buffer_len/0x20) * i);
		descr[i * 2 + 1] = __builtin_bswap32(0x80000000 | (buffer_len/0x20));
	}

	memdcbst(descr, 0x20 * 2 * 4);

	write32(snd_base + 8, 0);
	write32(snd_base + 8, 0x2000000);
	write32(snd_base + 0, descr_base);
	write32(snd_base + 8, 0x1d08001c);
	write32(snd_base + 0xC, 0x1c);
//	write32(snd_base + 8, read32(snd_base) | 0x1000000);
	
	wptr = 0;
}

void xenon_sound_submit(void *data, int len)
{
	int i = 0;
	while (len)
	{
		int av = buffer_len - wptr;
		if (av > len)
			av = len;
		
		memcpy(buffer + wptr, data + i, av);
		memdcbst(buffer + wptr, av);
		
		i += av;
		wptr += av;
		len -= av;
		if (wptr == buffer_len)
			wptr = 0;
	}
	int cur_descr = wptr / (buffer_len/0x20);
	
	write32(snd_base + 4, cur_descr << 8);
	write32(snd_base + 8, read32(snd_base) | 0x1000000);
}

int xenon_sound_get_free(void)
{
	uint32_t reg = read32(snd_base + 4);
	
	int rptr_descr = reg & 0x1f;
	int last_valid_descr = (reg & 0x1f00) >> 8;
	int cur_len = (reg >> 16) & 0xFFFF;
	
	if (rptr_descr == last_valid_descr && !cur_len)
		return buffer_len;

	int rptr = rptr_descr * (buffer_len/0x20);
	int av = rptr - wptr;
	if (av < 0)
		av += buffer_len;
	return av;
}

int xenon_sound_get_unplayed(void)
{
	uint32_t reg = read32(snd_base + 4);
	
	int rptr_descr = reg & 0x1f;
	int last_valid_descr = (reg & 0x1f00) >> 8;
	int cur_len = (reg >> 16) & 0xFFFF;
	
	int l = last_valid_descr - rptr_descr;
	if (l < 0)
		l += 0x20;
	l *= (buffer_len/0x20);
	l += cur_len;
	
	return l;
}
