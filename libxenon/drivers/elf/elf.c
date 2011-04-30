#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <xenon_soc/xenon_power.h>
#include <ppc/cache.h>
#include <ppc/timebase.h>
#include <time/time.h>

#include "elf_abi.h"

#define ELF_CODE_RELOC_START ((void*)0x87FF0000)
#define ELF_DATA_RELOC_START ((void*)0x88000000)
#define ELF_GET_RELOCATED(x) (ELF_CODE_RELOC_START+((unsigned long)(x)-(unsigned long)elfldr_start))

extern char elfldr_start[], elfldr_end[];
extern void elf_run(unsigned long entry);
extern void elf_hold_thread();

extern volatile unsigned long elf_secondary_hold_addr;

static inline void elf_putch(unsigned char c)
{
	while (!((*(volatile uint32_t*)(0xea001000 + 0x18))&0x02000000));
	*(volatile uint32_t*)(0xea001000 + 0x14) = (c << 24) & 0xFF000000;
}

static inline void elf_memset(unsigned char *dst, int v, int l)
{
	while (l--) *dst++ = v;
}

static inline void elf_memcpy(unsigned char *dst, const unsigned char *src, int l)
{
	while (l--)
		*dst++ = *src++;
}

#if 1
#define LINESIZE 128
static inline void elf_sync(volatile void *addr)
{
	asm volatile ("dcbst 0, %0" : : "b" (addr));
	asm volatile ("sync");
	asm volatile ("icbi 0, %0" : : "b" (addr));
	asm volatile ("isync");
}
      
static inline void elf_sync_before_exec(unsigned char *dst, int l)
{
	dst=(unsigned char *)((unsigned long)dst&(~(LINESIZE-1)));
	
	l+=LINESIZE;
	l&=~(LINESIZE-1);
	
	while (l > 0)
	{
		elf_sync(dst);
		dst += LINESIZE;
		l -= LINESIZE;
	}
}
#else
#define LINESIZE 16
static inline void elf_sync(volatile void *addr)
{
	asm volatile ("dcbst 0, %0" : : "b" (addr));
	asm volatile ("sync");
	asm volatile ("icbi 0, %0" : : "b" (addr));
	asm volatile ("isync");
}
      
static inline void elf_sync_before_exec(unsigned char *dst, int l)
{
	while (l > LINESIZE)
	{
		elf_sync(dst);
		dst += LINESIZE;
		l -= LINESIZE;
	}
}
#endif

static void __attribute__ ((section (".elfldr"))) elf_prepare_run (void *addr)
{
	Elf32_Ehdr *ehdr;
	Elf32_Shdr *shdr;
	unsigned char *strtab = 0;
	int i;

	ehdr = (Elf32_Ehdr *) addr;

	shdr = (Elf32_Shdr *) (addr + ehdr->e_shoff + (ehdr->e_shstrndx * sizeof (Elf32_Shdr)));

	if (shdr->sh_type == SHT_STRTAB)
		strtab = (unsigned char *) (addr + shdr->sh_offset);

	for (i = 0; i < ehdr->e_shnum; ++i)
	{
		shdr = (Elf32_Shdr *) (addr + ehdr->e_shoff +
				(i * sizeof (Elf32_Shdr)));

		if (!(shdr->sh_flags & SHF_ALLOC) || shdr->sh_size == 0)
			continue;

		void *target = (void*)(((unsigned long)0x80000000) | shdr->sh_addr);

		if (shdr->sh_type == SHT_NOBITS) {
			elf_memset (target, 0, shdr->sh_size);
		} else {
			elf_memcpy ((void *) target,
				(unsigned char *) addr + shdr->sh_offset,
				shdr->sh_size);
		}
		elf_sync_before_exec (target, shdr->sh_size);
	}

	*(volatile unsigned long *)ELF_GET_RELOCATED(&elf_secondary_hold_addr) = ehdr->e_entry + 0x60;
	
	// call elf_run()
	void(*call)(unsigned long) = ELF_GET_RELOCATED(elf_run);
	call(ehdr->e_entry);
}

void elf_runFromMemory (void *addr, int size)
{
	int i;

	// relocate code
	memcpy(ELF_CODE_RELOC_START,elfldr_start,elfldr_end-elfldr_start); 
	memicbi(ELF_CODE_RELOC_START,elfldr_end-elfldr_start);

	// relocate elf data
	memcpy(ELF_DATA_RELOC_START,addr,size);

	// get all threads to be on hold in the relocated zone
	xenon_thread_startup();
	for (i = 1; i < 6; ++i)
		while (xenon_run_thread_task(i, NULL, ELF_GET_RELOCATED(elf_hold_thread)));
		
	mdelay(200); // TODO: remove this and detect that all threads are on hold

	// call elf_prepare_run()
	void(*call)(void*) = ELF_GET_RELOCATED(elf_prepare_run);
	call(ELF_DATA_RELOC_START);
	
	// never called, make sure the function survives linking
	elf_prepare_run(NULL);
}

int elf_runFromDisk (char *filename)
{
	int f = open(filename, O_RDONLY);
	if (f < 0)
	{
		return f;
	}

	struct stat s;
	fstat(f, &s);

	int size = s.st_size;
	void * buf=malloc(size);

	int r = read(f, buf, size);
	if (r < 0)
	{
		close(f);
		return r;
	}

	elf_runFromMemory(buf, r);

	return 0;
}