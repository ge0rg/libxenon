#ifndef __console_console_h
#define __console_console_h

#ifdef __cplusplus
extern "C" {
#endif

void console_set_colors(unsigned int background, unsigned int foreground); // can be called before init
void console_putch(const char c);
void console_init(void);
void console_close(void);

#ifdef __cplusplus
};
#endif

#endif
