#include <ppc/xenonsprs.h>

	/* bring a core to sleep. */
.globl cpusleep
cpusleep:
		/* save those */
	mfsrr0  %r7
	mfsrr1  %r8
	isync

		/* skip HID1 if secondary */
	mfspr   %r0, pir
	clrlwi. %r0, %r0, 31
	bne     1f

	mfspr   %r9, hid1
	rldimi  %r9, %r0, 58,4
	sync
	mtspr   hid1, %r9
	sync
	isync

1:
	li %r0, 0
	mtspr ctrlwr, %r0
	b 1b
