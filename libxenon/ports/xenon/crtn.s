.section	".init"
addi    %r1,%r31,128
ld      %r0,16(%r1)
mtlr    %r0
blr	
		
.section	".fini"
addi    %r1,%r31,128
ld      %r0,16(%r1)
mtlr    %r0
blr
