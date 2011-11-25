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
//	or %r1, %r1, %r1       /* low priority */
	lwz %r3, 0(%r4)
	cmpwi %r3, 0
	beq 1b

//	or %r2, %r2, %r2
	li %r4,0

.globl elf_run
elf_run:
	mr	%r31,%r3
	mr	%r30,%r4

	lis %r4,0x07ff
	lis %r3, b64@h
	ori %r3, %r3, b64@l
	add %r4,%r4,%r3
	lis %r3, elfldr_start@h
	ori %r3, %r3, elfldr_start@l
	sub %r4,%r4,%r3
	mtsrr0 %r4

		/* Clear IR/DR and set 64bits mode */
	mfmsr	%r3
	li	%r4, 0x30
	andc	%r3, %r3, %r4
	lis	%r4, 0x8000
	rldicr	%r4, %r4, 32, 31
	or	%r3, %r3, %r4
	mtsrr1	%r3

	rfid

b64:
		/* invalidate all SLBs */
	isync
	slbia
	isync

		/* reset page sizes */
	mfspr %r3, hid6
	li %r4, 0xF
	sldi %r4, %r4, 44
	andc %r3, %r3, %r4
	li %r4, 0x8
	sldi %r4, %r4, 44
	or %r3, %r3, %r4
	mtspr hid6, %r3
	sync
	isync

		/* reset TLB reload */
	mfspr %r3, lpcr
	li %r4, 0x400
	andc %r3,%r3,%r4
	mtspr lpcr, %r3
	sync
	isync

		/* gogogo :) */
	mfspr	%r3, pir /* linux wants thread # in r3 for secondary threads */
	cmplwi	%r3,0
	bne 1f
	mr	%r3,%r30
1:
	mr	%r4,%r31
	li	%r5,0

	

	mtctr %r31
	bctr

.globl elf_secondary_hold_addr
elf_secondary_hold_addr:
	.long 0