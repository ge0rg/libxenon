// startup when being loaded from xell. It the linux ABI.
#define lpcr 318
#define hid6 1017
#define lpidr 319
#define pir 1023
#define hrmor 313
.globl _start

.align 8
_start:

b 1f

. = _start + 0x60

2:
	mfspr %r3, pir
	lwz %r4, secondary_lock@l(0)
	srw %r4, %r4, %r3
	rlwinm. %r4, %r4, 0, 31, 31
	beq 2b

	mfspr %r3, pir
	rlwinm. %r3, %r3, 0, 31, 31
//	bne after_slb

1:
		/* setup SLB identity mapping, N=0, LP=1 */
	isync
	slbia
	isync

		/* read fuses */
	mfpir	%r3
	cmplwi	%r3,0
	beql	read_fuses

		/* set page size: 16MB for both */
	mfspr %r3, hid6
	li %r4, 0xF
	sldi %r4, %r4, 44
	andc %r3, %r3, %r4
	mtspr  hid6, %r3

	li %r3, 0x10
	mtctr %r3
	li %r4,  0x100 // rs
	lis %r5, 0x0800 // rb (valid)
	lis %r6, 0x1000 // ESID increase
	ori %r6, %r6, 1 // index
	li %r7, 0x1000
1:
	slbmte %r4, %r5

	add %r5, %r5, %r6
	add %r4, %r4, %r7

	bdnz+ 1b

after_slb:

	isync
	li %r3, 0xc00
	li %r4, 0x100
	mtctr %r4
1:
	tlbiel %r3, 0
	bdnz+ 1b

	mfmsr %r5
	ori %r5, %r5, 0x2000 // FP available
	oris %r5, %r5, 0x200 // Vector avail
//	ori %r5, %r5, 0x8000 // EE
	ori %r5, %r5, 0x10 // data relocate
	ori %r5, %r5, 0x20 // instruction relocate
	rldicl %r5, %r5, 0, 1 // clear SF (set 32bit mode)
	mtsrr1 %r5

	li %r3, 0
	mtspr lpidr, %r3

	mfspr %r5, lpcr
	ori %r5, %r5, 0x400 // software tlb reload
	mtspr lpcr, %r5

	li %r3, 1  // hrmor
	sldi %r3, %r3, 63
	mtspr 313, %r3

	lis %r1, 0
	rldicr  %r1, %r1, 32,31
	oris %r1, %r1, __libc_stack_end@h
	ori %r1, %r1, __libc_stack_end@l

	mfspr %r3, pir
	addi %r3, %r3, 1
	slwi %r3, %r3, 9 // 0x200 bytes per thread
	sub %r3, %r1, %r3

	mtsprg1 %r3 // HV context base

	subi %r1, %r1, 0x1000

		/* wake up secondary thread */
	li %r4, 0
	mfspr %r3, pir
	ori %r3, %r3, 1
	li %r5, 1
	slw %r5, %r5, %r3
	li %r3, secondary_lock@l
	bl atomic_clearset

	li %r4, 0
	mfspr %r3, pir
	li %r5, 1
	slw %r5, %r5, %r3
	li %r3, secondary_alive@l
	bl atomic_clearset

		/* now jump to primary or secondary entry point */
	mfspr %r3, pir
	cmplwi %r3, 0

	lis %r3, b32@h
	ori %r3, %r3, b32@l

	beq 1f
	lis %r3, secondary@h
	ori %r3, %r3, secondary@l
1:
	mtsrr0 %r3

	rfid

b32:
	bl call_ctors
	bl main

	bl _exit
	
	b .

.globl wait
wait:
	.long 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

secondary:
1:
	or %r1, %r1, %r1
	lis %r3, wait@h
	ori %r3, %r3, wait@l
	mfspr %r5, pir
	slwi %r5, %r5, 3
	add %r3, %r3, %r5

	lwz %r4, 0(%r3)
	cmpwi %r4, 0
	beq 1b

	lwz %r1, 4(%r3)

	or %r2, %r2, %r2
	mtctr %r4

	li %r4, 0
	stw %r4, 0(%r3)
	stw %r4, 4(%r3)

	bctrl

	b 1b

.globl xenon_fuses
xenon_fuses:
	.quad 0,0,0,0,0,0,0,0,0,0,0,0

read_fuses:
	li	%r3,12
	mtctr	%r3

	li	%r4,0x200
	oris	%r4,%r4,0x8000
	rldicr	%r4,%r4,32,31
	oris 	%r4,%r4,2

	lis	%r5,0x8000
	rldicr	%r5,%r5,32,31
	ori	%r5,%r5,xenon_fuses@l

	1:
		ld	%r3,0(%r4)
		std	%r3,0(%r5)
		addi	%r4,%r4,0x200
		addi	%r5,%r5,8
		bdnz	1b

	blr

.globl return_to_xell
return_to_xell:
        	/* Init regs */
	lis	%r5,0xc810
	lis	%r6,0x9c00

		/* Copy from Flash, src = %r5, dst = %r6 */
	lis	%r4,1 /* 256k */
	mtctr   %r4

	1:	lwz     %r8,0(%r5)		//Memcopy
		stw     %r8,0(%r6)
		dcbst   %r0,%r6		//Flush cache to ram
		icbi	%r0,%r6
		sync	0
		isync
		addi    %r6,%r6,4
		addi    %r5,%r5,4
		bdnz    1b

		/* Clear IR/DR, set 64 bit mode and jump to Xell*/
	li	%r4,0x30
	mfmsr	%r3
	andc	%r3,%r3,%r4
	lis 	%r4,0x8000
	rldicr	%r4,%r4,32,31
	or	%r3,%r3,%r4
	mtsrr1	%r3

	lis	%r3,0x8000
	rldicr	%r3,%r3,32,31
	oris 	%r3,%r3,0x1c00
	ori	%r3,%r3,0x194 /* HACK: offset to a jump to main(), as start() won't work because of the stupid hardcoded hrmor in Xell... */
	mtsrr0	%r3


		/* HACK: as I call main() directly, i need to clear BSS and setup stack too... */
	li	%r3,0
	lis	%r6,0x9c08
	lis	%r4,0x8
	mtctr   %r4
	1:
		stw 	%r3,0(%r6)
		stw 	%r3,4(%r6)
		stw 	%r3,8(%r6)
		stw 	%r3,12(%r6)
		dcbf    %r0,%r6
		addi	%r6,%r6,0x10
          	bdnz	1b

	li %sp, 0
	oris %sp, %sp, 0x8000
	rldicr %sp, %sp, 32,31
	oris %sp, %sp, 0x1e00
	subi %sp, %sp, 0x80

	rfid
