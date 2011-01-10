#ifndef __xenos_xenos_h
#define __xenos_xenos_h

#ifdef __cplusplus
extern "C" {
#endif

#define VIDEO_MODE_AUTO		    -1
#define VIDEO_MODE_VGA_640x480   0
#define VIDEO_MODE_VGA_1024x768  1
#define VIDEO_MODE_PAL60         2
#define VIDEO_MODE_YUV_480P      3
#define VIDEO_MODE_PAL50         4
#define VIDEO_MODE_VGA_1280x768  5
#define VIDEO_MODE_VGA_1360x768  6
#define VIDEO_MODE_VGA_1280x720  7
#define VIDEO_MODE_VGA_1440x900  8
#define VIDEO_MODE_VGA_1280x1024 9
#define VIDEO_MODE_HDMI_720P     10
#define VIDEO_MODE_YUV_720P      11

void xenos_init(int videoMode);

#ifdef __cplusplus
};
#endif

#endif      
