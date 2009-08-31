#ifndef __xenon_sound_h
#define __xenon_sound_h

#ifdef __cplusplus
extern "C" {
#endif

	/* you init, you get the nr. of LE, 16bit x 2 channels, signed audio bytes to write, and submit that as max. */

void xenon_sound_init(void);
void xenon_sound_submit(void *data, int len);
int xenon_sound_get_free(void);
int xenon_sound_get_unplayed(void);

#ifdef __cplusplus
};
#endif

#endif
