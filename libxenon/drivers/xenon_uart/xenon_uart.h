#ifndef __xenon_uart_h
#define __xenon_uart_h

#ifdef __cplusplus
extern "C" {
#endif

void putch(unsigned char c);
int kbhit(void);
int getch(void);

#ifdef __cplusplus
};
#endif

#endif
