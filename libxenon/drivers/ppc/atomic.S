
	// Taken from: http://www.ibm.com/developerworks/library/pa-atom/ 
.globl atomic_inc
atomic_inc:
1:
	lwarx  %r4, 0, %r3
	addi   %r4, %r4, 1
	stwcx. %r4, 0, %r3
	bne-   1b
	blr

.globl atomic_dec
atomic_dec:
1:
	lwarx  %r4, 0, %r3
	subi   %r4, %r4, 1
	stwcx. %r4, 0, %r3
	bne-   1b
	blr

.globl atomic_clearset
atomic_clearset:
1:
	lwarx  %r6, 0, %r3
	andc   %r6, %r6, %r4
	or     %r6, %r6, %r5
	stwcx. %r6, 0, %r3
	bne-   1b
	blr

.globl lock
lock:
1:
	lwarx  %r4, 0, %r3
	cmpwi  %r4, 0
	li     %r4, 1
	bne 1f
	stwcx. %r4, 0, %r3
	bne- 1b

1:
	blr

.globl unlock
unlock:
	li %r3, 0
	stw %r4, 0(%r3)
	blr
