/*
 *  $Id: newlib-1.16.0.patch,v 1.8 2008-11-29 12:08:00 wntrmute Exp $
 */

#ifndef _MACHINE__TYPES_H
#define _MACHINE__TYPES_H
#include <machine/_default_types.h>

#define __fpos_t_defined
typedef long long _fpos_t;

#define __off_t_defined
typedef long long _off_t;

#endif // _MACHINE__TYPES_H
