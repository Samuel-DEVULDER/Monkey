/**
 * Scalar values.
 * (c) 2010 L. Diener
 * adapted to amiga (m68k) by S.Devulder 2018.
 */

#include "scalars.h"
#include <math.h>

float costab[] = {
#include "costab.h"	
};

// Scalar value functions
// Assume we're using IEEE floats or doubles.
scalar nextScalar(scalar s) {
	return( s + 0.0001 );
// 	return nextafter( s, s + ((scalar)1) );
}

scalar prevScalar(scalar s) {
	return( s - 0.0001 );
// 	return nextafter( s, s - ((scalar)1) );
}

scalar scalarRand() {
	return (((float)rand()) / ((float)RAND_MAX));
}

#if M68K==0
_REG scalar scalarInvSqrt(_FP0(scalar s)) {
	return scalarInv(scalarSqrt(s));
}
#endif

_REG int scalarCmp(_FP0(scalar a), _FP1(scalar b)) {
	scalar _a = scalarAbs(a), _b = scalarAbs(b);
	scalar d = a-b;
	scalar thr = 0.00001;

	if(_a<_b) _a = _b;
	if(_a<thr || scalarAbs(d) <= _a*thr) return 0;
	return d>0 ? 1 : -1;
}
