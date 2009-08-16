// startup when being loaded from xell. It the linux ABI.
#define lpcr 318
#define hid6 1017
#define lpidr 319
.globl _start

.align 8
_start:

b 1f


. = _start + 0x60
# secondary threads. Do nothing so far.
b .

		/* setup SLB identity mapping, N=0, LP=1 */
1:

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

	li %r3, 0  // hrmor
	mtspr 313, %r3

	lis %r1, 0
	rldicr  %r1, %r1, 32,31
	oris %r1, %r1, __libc_stack_end@h
	ori %r1, %r1, __libc_stack_end@l
	subi %r1, %r1, 0x800
	mtsprg1 %r1 // HV context base
	subi %r1, %r1, 0x800

	lis %r3, b32@h
	ori %r3, %r3, b32@l
	mtsrr0 %r3

	rfid

b32:
	bl call_ctors
	bl main
	b .
