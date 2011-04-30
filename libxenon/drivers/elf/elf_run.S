.section ".elfldr"

#include <ppc/xenonsprs.h>

.globl elf_hold_thread
elf_hold_thread:
	lis %r4,0x87ff
	lis %r3, elf_secondary_hold_addr@h
	ori %r3, %r3, elf_secondary_hold_addr@l
	add %r4,%r4,%r3
	lis %r3, elfldr_start@h
	ori %r3, %r3, elfldr_start@l
	sub %r4,%r4,%r3

1:
	or %r1, %r1, %r1       /* low priority */
	lwz %r3, 0(%r4)
	cmpwi %r3, 0
	beq 1b

.globl elf_run
elf_run:
	/* compute real mode entry point addr */
	lis %r4, 0x8000
	rldicr %r4, %r4, 32, 31
	or %r3, %r3, %r4
	mtsrr0 %r3

	/* Clear IR/DR and set 64bits mode */
	mfmsr	%r3
	li	%r4, 0x30
	andc	%r3, %r3, %r4
	lis	%r4, 0x8000
	rldicr	%r4, %r4, 32, 31
	or	%r3, %r3, %r4
	mtsrr1	%r3

	/* gogogo :) */
	rfid

.globl elf_secondary_hold_addr
elf_secondary_hold_addr:
	.long 0