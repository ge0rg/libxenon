#ifndef __console_telnet_console_h
#define __console_telnet_console_h

#ifdef __cplusplus
extern "C" {
#endif

void telnet_console_init();
void telnet_console_tx_print(const char *buf, int bc);
void telnet_console_close();

#ifdef __cplusplus
};
#endif

#endif
