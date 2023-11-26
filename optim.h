/*
 * various optimisations
 * 
 * Samuel DEVULDER 2018
 */
#ifndef __OPTIM_H__
#define __OPTIM_H__

/* register parameters */

#define _REG
#define REGP(reg,param)	param
#define _A0(param)	REGP(a0, param)
#define _A1(param)	REGP(a1, param)
#define _D0(param)	REGP(d0, param)
#define _D1(param)	REGP(d1, param)
#define _FP0(param)	REGP(fp0,param)
#define _FP1(param)	REGP(fp1,param)

#if REGPARM
#if defined(__GNUC__)
#undef 	REGP
#define REGP(reg, p) 	p __asm(#reg)
#elif defined(__VBCC__)
#undef 	REGP
#define REGP(reg, p) 	__reg(#reg) p
#elif defined(__SASC)
#undef  _REG
#define _REG		__asm
#undef	REGP
#define	REGP(reg, p)	register __##reg p
#endif
#endif	// REGPARM

extern _REG void Q_memset(_A0(void *dst), _D0(int fill), _D1(int count));
extern _REG void Q_memcpy(_A0(void *dst), _A1(void *src), _D0(int count));
extern _REG void LongSwap(_A0(void *val));

#include <string.h>

#undef  memset
#define memset Q_memset

#undef  memcpy
#define memcpy Q_memcpy

#endif 	// OPTIM_H
