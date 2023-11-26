/**
 * Vector maths.
 * (c) L. Diener 2010
 */

#ifndef __VECTORS_H__
#define __VECTORS_H__

#include "scalars.h"

typedef struct vec2 {
	scalar x;
	scalar y;
} vec2;

typedef struct vec3 {
	scalar x;
	scalar y;
	scalar z;
} vec3;

//vec2 *makeVec2(vec2 *r, scalar x, scalar y);
//vec3 *makeVec3(vec3 *r, scalar x, scalar y, scalar z);
#define makeVec2(r,a,b)   ((r)->x = (a), (r)->y = (b), (r))
#define makeVec3(r,a,b,c) ((r)->x = (a), (r)->y = (b), (r)->z = (c), (r))

_REG void mult2(_A0(vec2 *r), _A1(vec2 *a), _FP0(scalar s));
_REG void mult3(_A0(vec3 *r), _A1(vec3 *a), _FP0(scalar s));
_REG void scale2(_A0(vec2 * r), _FP0(scalar s));
_REG void scale3(_A0(vec3 *r), _FP0(scalar s));

_REG void div2(_A0(vec2 * r), _A1(vec2 *a), _FP0(scalar s));
_REG void div3(_A0(vec3 *r), _A1(vec3 *a), _FP0(scalar s));
_REG void invScale2(_A0(vec2 *r), _FP0(scalar s));
_REG void invScale3(_A0(vec3 *r), _FP0(scalar s));

_REG void inverted2(_A0(vec2 * r), _A1(vec2 *a));
_REG void inverted3(_A0(vec3 *r), _A1(vec3 *a));
_REG void invert2(_A0(vec2 * r));
_REG void invert3(_A0(vec3 *r));

_REG void add2(_D0(vec2 *r), _A0(vec2 *a), _A1(vec2 *b));
_REG void add3(_D0(vec3 *r), _A0(vec3 *a), _A1(vec3 *b));
_REG void addTo2(_A0(vec2 *r), _A1(vec2 *a));
_REG void addTo3(_A0(vec3 *r), _A1(vec3 *a));

_REG void sub2(_D0(vec2 *r), _A0(vec2 *a), _A1(vec2 *b));
_REG void sub3(_D0(vec3 *r), _A0(vec3 *a), _A1(vec3 *b));
_REG void subFrom2(_A0(vec2 *r), _A1(vec2 *a));
_REG void subFrom3(_A0(vec3 *r), _A1(vec3 *a));

_REG scalar dot2(_A0(vec2 *a), _A1(vec2 *b));
_REG scalar dot3(_A0(vec3 *a), _A1(vec3 *b));

_REG void cross(_D0(vec3 *r), _A0(vec3 *a), _A1(vec3 *b));

_REG scalar length2(_A0(vec2 *v));
_REG scalar length3(_A0(vec3 *v));
_REG scalar dist2(_A0(vec2 *a), _A1(vec2 *b));
_REG scalar dist3(_A0(vec3 *a), _A1(vec3 *b));

_REG void normalized2(_A0(vec2 * r), _A1(vec2 *v));
_REG void normalized3(_A0(vec3 *r), _A1(vec3 *v));
_REG void normalize2(_A0(vec2 * r));
_REG void normalize3(_A0(vec3 *r));

_REG void reflected2(_D0(vec2 * r), _A0(vec2 *i), _A1(vec2 *n));
_REG void reflected3(_D0(vec3 *r), _A0(vec3 *i), _A1(vec3 *n));
_REG void reflect2(_A0(vec2 * r), _A1(vec2 *n));
_REG void reflect3(_A0(vec3 *r), _A1(vec3 *n));

_REG scalar vecAtan(_A0(vec2 *c));

_REG void extendTo(_A0(vec3 *r), _A1(vec3 *d));

_REG void vecPrintf3(_A0(vec3 *v));

_REG int vec3Cmp(_A0(const vec3 *a), _A1(const vec3 *b));

#endif
