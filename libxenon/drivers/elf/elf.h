#ifndef ELF_H
#define	ELF_H

#ifdef	__cplusplus
extern "C" {
#endif


void elf_runFromMemory (void *addr, int size);
int elf_runFromDisk (char *filename);
void elf_runWithDeviceTree (void *elf_addr, int elf_size, void *dt_addr, int dt_size);

#ifdef	__cplusplus
}
#endif

#endif	/* ELF_H */

