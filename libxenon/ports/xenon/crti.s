.section	".init"
.align 2

	.global	_init
	.type	_init, @function
_init:
mflr    %r0 
std     %r0,16(%r1)
stdu    %r1,-128(%r1)
	
		
	.section	".fini"
	.align	2
	.global	_fini
	.type	_fini, @function
_fini:
mflr    %r0 
std     %r0,16(%r1)
stdu    %r1,-128(%r1)
