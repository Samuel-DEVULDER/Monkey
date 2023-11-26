/**
 * Scalar values.
 * (c) 2010 L. Diener
 */

#ifndef __SCALARS_H__
#define __SCALARS_H__

#include <math.h>
#include <stdlib.h>


#include "optim.h"

#define scalar float

#define scalarSqrt(x) ((float)sqrt((x)))
#define scalarPow(x,y) ((float)pow((x),(y)))
#define scalarMin(x,y) ((x)<(y)?(x):(y))
#define scalarMax(x,y) ((x)>(y)?(x):(y))
#define scalarAtan2(x,y) (atan2((x),(y)))
#define scalarTan(x) (tan((x)))
#define scalarCos(x) (cos((x)))
#define scalarSin(x) (sin((x)))
#define scalarACos(x) (acos(x))
#define scalarASin(x) (asin(x))
#define scalarPI 3.14159265
#define scalarE 2.71828183
#define scalarInf INFINITY
#define scalarAbs(x) (((x)<0)?(-x):(x))
#define scalarLog(x) (log((x)))
#define scalarLog2(x) (log2((x)))
#define scalarMod(x,y) (fmod((x),(y)))
#define scalarAdd(x,y) ((x)+(y))
#define scalarSub(x,y) ((x)-(y))
#define scalarNeg(x)   (-(x))
#define scalarMul(x,y) ((x)*(y))
#define scalarDiv(x,y) ((scalar)((x)/(y)))
#define scalarInv(x) scalarDiv(1.0f, x)

scalar nextScalar(scalar s);
scalar prevScalar(scalar s);
scalar scalarRand();

#if M68K && defined(__VBCC__)
scalar scalarInvSqrt (_FP0(scalar s)) =
        "\tinline\n"
        "\tfmove.s\tfp0,d0\n"
        "\tlsr.l\t#1,d0\n"
		"\tsub.l\t#$5f3759df,d0\n"
		"\tfmul.s\t#0.5,fp0\n"
		"\tneg.l\td0\n"
		"\tfmul.s\td0,fp0\n"
		"\tfneg.s\td0,fp1\n"
		"\tfmul.s\td0,fp0\n"
		"\tfsub.s\t#1.5,fp0\n"
		"\tfmul.x\tfp1,fp0\n"
        "\teinline";
#elif M68K && defined(__GNUC__)
__inline static scalar scalarInvSqrt(scalar s)
{
    register scalar fp0 = s, fp1;
	register int d0;
    __asm("fmove%.s %1,%0"      : "=md"  (d0) : "f" (fp0));
	__asm("lsr.l #1,%0"         : "=&md" (d0) : "0" (d0));
	__asm("sub.l %1,%0"         : "=&md" (d0) : "i" (0x5f3759df), "0" (d0));
	__asm("fmul.s %1,%0"        : "=&f" (fp0) : "i" (0.5f), "0" (fp0));
	__asm("neg.l %0"            : "=&md" (d0) : "0" (d0));
	__asm("fmul.s %1,%0"        : "=&f" (fp0) : "md" (d0), "0" (fp0));
	__asm("fneg.s %1,%0"        : "=f"  (fp1) : "md" (d0));
	__asm("fmul.s %1,%0"        : "=&f" (fp0) : "md" (d0), "0" (fp0));
	__asm("fsub.s %1,%0"        : "=&f" (fp0) : "i" (1.5f), "0" (fp0));
	__asm("fmul.x %1,%0"        : "=&f" (fp0) : "f" (fp1), "0" (fp0));
	return fp0;
}
#else
_REG scalar scalarInvSqrt(_FP0(scalar s));
//#define scalarInvSqrt(s) scalarInv(scalarSqrt(s))
#endif

_REG int scalarCmp(_FP0(scalar a), _FP1(scalar b));

#endif
