#ifndef USBMAIN_H
#define	USBMAIN_H

#ifdef	__cplusplus
extern "C" {
#endif

int usb_init(void);
void usb_shutdown(void);
void usb_do_poll(void);

#ifdef	__cplusplus
}
#endif

#endif	/* USBMAIN_H */

