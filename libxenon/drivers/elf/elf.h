#ifndef ELF_H
#define	ELF_H

#ifdef	__cplusplus
extern "C" {
#endif


void elf_runFromMemory (void *addr, int size);
int elf_runFromDisk (char *filename);


#ifdef	__cplusplus
}
#endif

#endif	/* ELF_H */

