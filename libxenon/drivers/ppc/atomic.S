
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

	// thanks cOz for the following :)
.globl lock
lock:
        lwarx   %r4, 0, %r3
        cmplwi  %r4, 0
        bc	6, eq, spinner
        stwcx.  %r3, 0, %r3
        bc      6, eq, spinner
        sync    1  // lwsync
        .long   0x7FFFFB78 // db16cyc - AIX for 16 cycle delay
        blr
spinner:
        .long   0x7FFFFB78 // db16cyc - AIX for 16 cycle delay
        lwz     %r4, 0(%r3)
        cmplwi  %r4, 0
        bc      15, eq, lock
        b       spinner
 
.globl unlock
unlock:
        li      %r4, 0
        sync    1  // lwsync
        stw     %r4, 0(%r3)
        blr