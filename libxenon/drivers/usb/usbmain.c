/*  *********************************************************************
    *  Broadcom Common Firmware Environment (CFE)
    *  
    *  Main Module				File: usbmain.c
    *  
    *  Main module that invokes the top of the USB stack from CFE.
    *  
    *  Author:  Mitch Lichtenberg
    *  
    *********************************************************************  
    *
    *  Copyright 2000,2001,2002,2003
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
    *     and retain this copyright notice and list of conditions 
    *     as they appear in the source file.
    *  
    *  2) No right is granted to use any trade name, trademark, or 
    *     logo of Broadcom Corporation.  The "Broadcom Corporation" 
    *     name may not be used to endorse or promote products derived 
    *     from this software without the prior written permission of 
    *     Broadcom Corporation.
    *  
    *  3) THIS SOFTWARE IS PROVIDED "AS-IS" AND ANY EXPRESS OR
    *     IMPLIED WARRANTIES, INCLUDING BUT NOT LIMITED TO, ANY IMPLIED
    *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
    *     PURPOSE, OR NON-INFRINGEMENT ARE DISCLAIMED. IN NO EVENT 
    *     SHALL BROADCOM BE LIABLE FOR ANY DAMAGES WHATSOEVER, AND IN 
    *     PARTICULAR, BROADCOM SHALL NOT BE LIABLE FOR DIRECT, INDIRECT,
    *     INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
    *     (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
    *     GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
    *     BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
    *     OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR 
    *     TORT (INCLUDING NEGLIGENCE OR OTHERWISE), EVEN IF ADVISED OF 
    *     THE POSSIBILITY OF SUCH DAMAGE.
    ********************************************************************* */


#include "cfe.h"
#include "lib_physio.h"

#if CFG_PCI
#include "pcireg.h"
#include "pcivar.h"
#endif

#include "usbchap9.h"
#include "usbd.h"


/*  *********************************************************************
    *  Externs
    ********************************************************************* */

extern usb_hcdrv_t ohci_driver;			/* OHCI Driver dispatch */

extern int ohcidebug;				/* OHCI debug control */
extern int usb_noisy;				/* USBD debug control */

int ui_init_usbcmds(void);			/* forward */

/*  *********************************************************************
    *  Globals
    ********************************************************************* */

/*
 * We keep track of the pointers to USB buses in globals.
 * One entry in this array per USB bus (the Opti controller
 * on the SWARM has two functions, so it's two buses)
 */

#define USB_MAX_BUS 4
int usb_buscnt = 0;
usbbus_t *usb_buses[USB_MAX_BUS];


/*  *********************************************************************
    *  usb_cfe_timer(arg)
    *  
    *  This routine is called periodically by CFE's timer routines
    *  to give the USB subsystem some time.  Basically we scan
    *  for work to do to manage configuration updates, and handle
    *  interrupts from the USB controllers.
    *  
    *  Input parameters: 
    *  	   arg - value we passed when the timer was initialized
    *  	          (not used)
    *  	   
    *  Return value:
    *  	   nothing
    ********************************************************************* */

static void usb_cfe_timer(void *arg)
{
    int idx;
    static int in_poll = 0;

    /*
     * We sometimes call the timer routines in here, which calls
     * the polling loop.  This code is not reentrant, so
     * prevent us from running the interrupt routine or
     * bus daemon while we are already in there.
     */

    if (in_poll) return;

    /*
     * Do not allow nested "interrupts."
     */

    in_poll = 1;

    for (idx = 0; idx < usb_buscnt; idx++) {
	if (usb_buses[idx]) {
	    usb_poll(usb_buses[idx]);
	    usb_daemon(usb_buses[idx]);
	    }
	}

    /*
     * Okay to call polling again.
     */

    in_poll = 0;
}


/*  *********************************************************************
    *  usb_init_one_ohci(addr)
    *  
    *  Initialize one USB controller.
    *  
    *  Input parameters: 
    *  	   addr - physical address of OHCI registers
    *  	   
    *  Return value:
    *  	   0 if ok
    *  	   else error
    ********************************************************************* */
static int usb_init_one_ohci(uint32_t addr)
{
    usbbus_t *bus;
    int res;

    bus = UBCREATE(&ohci_driver, addr);

    if (bus == NULL) {
	printf("USB: Could not create OHCI driver structure for controller at 0x%08X\n",addr);
	return -1;
	}

    bus->ub_num = usb_buscnt;

    res = UBSTART(bus);

    if (res != 0) {
	printf("USB: Could not init OHCI controller at 0x%08X\n",addr);
	UBSTOP(bus);
	return -1;
	}
    else {
	usb_buses[usb_buscnt++] = bus;
	usb_initroot(bus);
	}

    return 0;
}

#if CFG_PCI
/*  *********************************************************************
    *  usb_init_pci_ohci()
    *  
    *  Initialize all PCI-based OHCI controllers
    *  
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   0 if ok
    *  	   else error
    ********************************************************************* */
static int usb_init_pci_ohci(void)
{
    int res;
    pcitag_t tag;
    uint32_t pciclass;
    phys_addr_t bar;
    int idx;

    idx = 0;

    while (pci_find_class(PCI_CLASS_SERIALBUS,idx,&tag) == 0) {
	pciclass = pci_conf_read(tag,PCI_CLASS_REG);
	if ((PCI_SUBCLASS(pciclass) == PCI_SUBCLASS_SERIALBUS_USB) && 
	    (PCI_INTERFACE(pciclass) == 0x10)) {
	    /* On the BCM1250, this sets the address to "match bits" mode, 
	       which eliminates the need for byte swaps of data to/from the registers. */
	    if (pci_map_mem(tag,PCI_MAPREG_START,PCI_MATCH_BITS,&bar) == 0) {
		pci_tagprintf(tag,"OHCI USB controller found at %08X\n",(uint32_t) bar);
		/* XXX hack: assumes physaddrs are 32 bits, bad bad! */
		res = usb_init_one_ohci((uint32_t)bar);
		if (res < 0) break;
		}
	    else {
		pci_tagprintf(tag,"Could not map OHCI base address\n");
		}
	    }
	idx++;
	}

    return 0;
}
#endif


int usb_initialized = 0;

int usb_init(void)
{
    static int initdone = 0;

    if (initdone) {
	printf("USB has already been initialized.\n");
	return -1;
	}

	initdone = 1;

	usb_buscnt = 0;

#if CFG_PCI
	usb_init_pci_ohci();
#endif

#if 0
	usb_noisy = 1;
	ohcidebug = 1;
#endif

	usb_init_one_ohci(0xea002000);
	usb_init_one_ohci(0xea004000);

//    cfe_bg_add(usb_cfe_timer,NULL);
	usb_initialized = 1;

	return 0;
}

void usb_do_poll(void)
{
	if (!usb_initialized)
		return;
	usb_cfe_timer(0);
}
