/*  *********************************************************************
	*  Broadcom Common Firmware Environment (CFE)
	*  
	*  USB Human Interface Driver				File: usb.c
	*  
	*  Xbox 360 Controller support
	*  
	*  Author:  Felix Domke (based on usbhid.c)
	*  
	*********************************************************************  
	*
	*  Copyright 2000,2001,2002,2003,2005
	*  Broadcom Corporation. All rights reserved.
	*  
	*  This software is furnished under license and may be used and 
	*  copied only in accordance with the following terms and 
	*  conditions.  Subject to these conditions, you may download, 
	*  copy, install, use, modify and distribute modified or unmodified 
	*  copies of this software in source and/or binary form.  No title 
	*  or ownership is transferred hereby.
	*  
	*  1) Any source code used, modified or distributed must reproduce 
	*	 and retain this copyright notice and list of conditions 
	*	 as they appear in the source file.
	*  
	*  2) No right is granted to use any trade name, trademark, or 
	*	 logo of Broadcom Corporation.  The "Broadcom Corporation" 
	*	 name may not be used to endorse or promote products derived 
	*	 from this software without the prior written permission of 
	*	 Broadcom Corporation.
	*  
	*  3) THIS SOFTWARE IS PROVIDED "AS-IS" AND ANY EXPRESS OR
	*	 IMPLIED WARRANTIES, INCLUDING BUT NOT LIMITED TO, ANY IMPLIED
	*	 WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
	*	 PURPOSE, OR NON-INFRINGEMENT ARE DISCLAIMED. IN NO EVENT 
	*	 SHALL BROADCOM BE LIABLE FOR ANY DAMAGES WHATSOEVER, AND IN 
	*	 PARTICULAR, BROADCOM SHALL NOT BE LIABLE FOR DIRECT, INDIRECT,
	*	 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
	*	 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
	*	 GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
	*	 BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
	*	 OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR 
	*	 TORT (INCLUDING NEGLIGENCE OR OTHERWISE), EVEN IF ADVISED OF 
	*	 THE POSSIBILITY OF SUCH DAMAGE.
	********************************************************************* */

#include <input/input.h>
#include "cfe.h"

#include "usbchap9.h"
#include "usbd.h"

/* For systems with non-coherent DMA, allocate all buffers to be
   cache-aligned and multiples of a cache line in size, so that they
   can be safely flushed or invalidated. */

#define CACHE_ALIGN	32	   /* XXX place holder, big enough to now. */
#define BUFF_ALIGN	 16
#define ALIGN(n,align) (((n)+((align)-1)) & ~((align)-1))

#define usb_dma_alloc(n) (KMALLOC(ALIGN((n),CACHE_ALIGN),BUFF_ALIGN))
#define usb_dma_free(p)  (KFREE(p))


static int usbctrl_attach(usbdev_t *dev,usb_driver_t *drv);
static int usbctrl_detach(usbdev_t *dev);

#define UBR_KBD_MAX 20

static int controller_mask = 0;

typedef struct usbctrl_softc_s {
	int uhid_ipipe;
	int uhid_ipipemps;
	int uhid_devtype;
	int is_wireless, index;
	uint8_t *uhid_imsg;
	uint8_t uhid_lastmsg[UBR_KBD_MAX];
	uint32_t uhid_shiftflags;
} usbctrl_softc_t;

usb_driver_t usbctrl_driver = {
	"Xbox 360 Controller",
	usbctrl_attach,
	usbctrl_detach
};


/*  *********************************************************************
	*  usbctrl_ireq_callback(ur)
	*  
	*  This routine is called when our interrupt transfer completes
	*  and there is report data to be processed.
	*  
	*  Input parameters: 
	*			 ur - usb request
	*			 
	*  Return value:
	*			 0
	********************************************************************* */


// from wireless:
//08 80 82 00 40 01 02 20 00 07 05 82 03 20 00 02 07 05 02 03 
//00 0f 00 f0 f0 cc 56 57 9e c0 53 7f c9 00 00 05 13 e7 20 1d 
//00 00 00 f0 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
//00 f8 02 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
//00 00 00 13 e2 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
//00 00 00 f0 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
//00 01 00 f0 00 13 00 00 00 00 4f fc 1e 06 88 07 f6 f7 00 00 
//00 00 00 f0 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
//00 01 00 f0 00 13 00 00 00 00 9d fd 1e 06 88 07 f6 f7 00 00 
//00 00 00 f0 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
//00 01 00 f0 00 13 00 00 00 00 9d fd ce 04 88 07 f6 f7 00 00 

static int usbctrl_ireq_callback(usbreq_t *ur)
{
	usbctrl_softc_t *uhid = (ur->ur_dev->ud_private);
	
/*	int i;
	for (i = 0; i < uhid->uhid_ipipemps; ++i)
		printf("%02x ", ur->ur_buffer[i]);
	printf("\n");  */
	
	/*
	 * If the driver is unloaded, the request will be cancelled.
	 */

	if (ur->ur_status == 0xFF) {
		usb_free_request(ur);
		return 0;
	}

	struct controller_data_s c;
	unsigned char *b = ur->ur_buffer;
	
	if (uhid->is_wireless)
	{
		if (b[5] == 0x13)
		{
			if (b[6] == 2) /* CHECKME: is this required for the Xbox racing wheel? */
				b++;
			b += 4;
		} else
			goto ignore;
	}

	c.s1_x = (b[7] << 8) | b[6];
	c.s1_y = (b[9] << 8) | b[8];
	c.s2_x = (b[11] << 8) | b[10];
	c.s2_y = (b[13] << 8) | b[12];
	c.s1_z = !!(b[2] & 0x40);
	c.s2_z = !!(b[2] & 0x80);
	c.lt = b[4];
	c.rt = b[5];
	c.lb = !!(b[3] & 1);
	c.rb = !!(b[3] & 2);
	
	c.a = !!(b[3] & 0x10);
	c.b = !!(b[3] & 0x20);
	c.x = !!(b[3] & 0x40);
	c.y = !!(b[3] & 0x80);
	
	c.start = !!(b[2] & 0x10);
	c.select  = !!(b[2] & 0x20);
	
	c.up = !!(b[2] & 1);
	c.down = !!(b[2] & 2);
	c.left = !!(b[2] & 4);
	c.right = !!(b[2] & 8);
	
	c.logo = !!(b[3] & 0x4);
	
	set_controller_data(uhid->index, &c);

ignore:
	usb_queue_request(ur);

	return 0;
}


/*  *********************************************************************
	*  usbctrl_queue_intreq(dev,softc)
	*  
	*  Queue an interrupt request for this usb device.  The
	*  driver will place this request on the queue that corresponds
	*  to the endpoint, and will call the callback routine when
	*  something happens.
	*  
	*  Input parameters: 
	*			 dev - usb device
	*			 softc - the usb hid softc
	*			 
	*  Return value:
	*			 nothing
	********************************************************************* */

static void usbctrl_queue_intreq(usbdev_t *dev,usbctrl_softc_t *softc)
{
	usbreq_t *ur;

	ur = usb_make_request(dev,
						  softc->uhid_ipipe,
						  softc->uhid_imsg, softc->uhid_ipipemps,
						  UR_FLAG_IN);

	ur->ur_callback = usbctrl_ireq_callback;

	usb_queue_request(ur);
}


/*  *********************************************************************
	*  usbctrl_attach(dev,drv)
	*  
	*  This routine is called when the bus scan stuff finds a HID
	*  device.  We finish up the initialization by configuring the
	*  device and allocating our softc here.
	*  
	*  Input parameters: 
	*			 dev - usb device, in the "addressed" state.
	*			 drv - the driver table entry that matched
	*			 
	*  Return value:
	*			 0
	********************************************************************* */

static int usbctrl_attach(usbdev_t *dev,usb_driver_t *drv)
{
	usb_config_descr_t *cfgdscr = dev->ud_cfgdescr;
	usb_endpoint_descr_t *epdscr;
	usb_interface_descr_t *ifdscr;
	usbctrl_softc_t *softc;

	dev->ud_drv = drv;

	softc = KMALLOC(sizeof(usbctrl_softc_t),0);
	memset(softc,0,sizeof(usbctrl_softc_t));
	dev->ud_private = softc;

	epdscr = usb_find_cfg_descr(dev,USB_ENDPOINT_DESCRIPTOR_TYPE,0);
	ifdscr = usb_find_cfg_descr(dev,USB_INTERFACE_DESCRIPTOR_TYPE,0);

	if (!epdscr || !ifdscr) {
		printf("couldn't find descriptor!\n");
		return 0;
	}
	
	softc->is_wireless = GETUSBFIELD(&dev->ud_devdescr, idProduct) == 0x291;
	
	int i;
	for (i = 0; controller_mask & (1<<i); ++i);
	printf("attched controller %d\n", i);
	softc->index = i;
	controller_mask |= 1<<i;

	/*
	 * Allocate a DMA buffer
	 */

	softc->uhid_imsg = usb_dma_alloc(UBR_KBD_MAX);
	if (softc->uhid_imsg == NULL) {
		printf("couldn't alloc buffer\n");
		/* Could not allocate a buffer, fail. */
		return -1;
	}
	
	/*
	 * Choose the standard configuration.
	 */

	usb_set_configuration(dev,cfgdscr->bConfigurationValue);

	/*
	 * Open the interrupt pipe.
	 */

	softc->uhid_ipipe = usb_open_pipe(dev,epdscr);
	softc->uhid_ipipemps = GETUSBFIELD(epdscr,wMaxPacketSize);

	/*
	 * Queue a transfer on the interrupt endpoint to catch
	 * our first characters.
	 */

	usbctrl_queue_intreq(dev,softc);

	return 0;
}

/*  *********************************************************************
	*  usbctrl_detach(dev)
	*  
	*  This routine is called when the bus scanner notices that
	*  this device has been removed from the system.  We should
	*  do any cleanup that is required.  The pending requests
	*  will be cancelled automagically.
	*  
	*  Input parameters: 
	*			 dev - usb device
	*			 
	*  Return value:
	*			 0
	********************************************************************* */

static int usbctrl_detach(usbdev_t *dev)
{
	usbctrl_softc_t *uhid = dev->ud_private;
	
	printf("detached controller %d\n", uhid->index);

	controller_mask &= ~(1<<uhid->index);
	return 0;
}
