#ifndef __xenon_post_h
#define __xenon_post_h
#ifdef __ASSEMBLER__
#define POST(c) \
	li		%r3, c; \
	sldi	%r3, %r3, 56; \
	lis		%r4, 0x2006; \
	ori		%r4, %r4, 0x1010; \
	std		%r3, 0(%r4)
#else

#ifdef __cplusplus
extern "C" {
#endif

#define POST(c) xenon_post(c)

void xenon_post(char post_code);

#ifdef __cplusplus
};
#endif

#endif

#endif
