/*  *********************************************************************
    *  Broadcom Common Firmware Environment (CFE)
    *  
    *  Physical memory peek/poke routines	File: lib_physio.h
    *  
    *  Little stub routines to allow access to arbitrary physical
    *  addresses.  In most cases this should not be needed, as
    *  many physical addresses are within kseg1, but this handles
    *  the cases that are not automagically, so we don't need
    *  to mess up the code with icky macros and such.
    *  
    *  Author:  Mitch Lichtenberg (mpl@broadcom.com)
    *  
    *********************************************************************  
    *
    *  Copyright 2000,2001
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
    *     and retain this copyright notice and list of conditions as 
    *     they appear in the source file.
    *  
    *  2) No right is granted to use any trade name, trademark, or 
    *     logo of Broadcom Corporation. Neither the "Broadcom 
    *     Corporation" name nor any trademark or logo of Broadcom 
    *     Corporation may be used to endorse or promote products 
    *     derived from this software without the prior written 
    *     permission of Broadcom Corporation.
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


#ifndef _LIB_PHYSIO_H
#define _LIB_PHYSIO_H

typedef unsigned int physaddr_t;

#define __UNCADDRX(x) (((unsigned int)(x))|0x00000000UL)

static inline uint32_t __pswap32(uint32_t x)
{
    uint32_t y;

    y = ((x & 0xFF) << 24) |
	((x & 0xFF00) << 8) |
	((x & 0xFF0000) >> 8) |
	((x & 0xFF000000) >> 24);

    return y;
}


//#define phys_write8(a,b) *((volatile uint8_t *) __UNCADDRX(a)) = (b)
//#define phys_write16(a,b) *((volatile uint16_t *) __UNCADDRX(a)) = (b)
#define phys_write32(a,b) *((volatile uint32_t *) __UNCADDRX(a)) = __pswap32(b)
//#define phys_write64(a,b) *((volatile uint32_t *) __UNCADDRX(a)) = (b)
//#define phys_read8(a) *((volatile uint8_t *) __UNCADDRX(a))
//#define phys_read16(a) *((volatile uint16_t *) __UNCADDRX(a))
#define _phys_read32(a) *((volatile uint32_t *) __UNCADDRX(a))
static inline uint32_t phys_read32(physaddr_t a) { uint32_t res = _phys_read32(a); return __pswap32(res); }
//#define phys_read64(a) *((volatile uint64_t *) __UNCADDRX(a))


#endif
