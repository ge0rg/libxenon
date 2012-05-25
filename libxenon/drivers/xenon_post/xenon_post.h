#ifndef __xenon_post_h
#define __xenon_post_h

#ifdef __cplusplus
extern "C" {
#endif

#define POST(c) xenon_post(c)

void xenon_post(char post_code);

#ifdef __cplusplus
};
#endif

#endif
