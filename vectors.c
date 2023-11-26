/**
 * Vector maths.
 * (c) L. Diener 2010
 */

#include "vectors.h"
#include <math.h>
#include <stdio.h>

/*
// Generate vectors
_REG vec2 *makeVec2(vec2 *v, scalar x, scalar y) {
	v->x = x;
	v->y = y;
	return v;
}

_REG vec3 *makeVec3(vec3 *v, scalar x, scalar y, scalar z) {
	v->x = x;
	v->y = y;
	v->z = z;
	return v;
}
*/

// Vector-scalar multiplication
_REG void mult2(_A0(vec2 *r), _A1(vec2 *a), _FP0(scalar s)) {
	r->x = a->x * s;
	r->y = a->y * s;
}

_REG void mult3(_A0(vec3 *r), _A1(vec3* a), _FP0(scalar s)) {
	r->x = a->x * s;
	r->y = a->y * s;
	r->z = a->z * s;
}

_REG void scale2(_A0(vec2 *r), _FP0(scalar s)) {
	r->x *= s;
	r->y *= s;
}

_REG void scale3(_A0(vec3 *r), _FP0(scalar s)) {
	r->x *= s;
	r->y *= s;
	r->z *= s;
}

// Vector-scalar division
_REG void div2(_A0(vec2 *r), _A1(vec2 *a), _FP0(scalar s)) {
	mult2(r,a,scalarInv(s));
}

_REG void div3(_A0(vec3 *r), _A1(vec3 *a), _FP0(scalar s)) {
	mult3(r,a,scalarInv(s));
}

_REG void invScale2(_A0(vec2 *r), _FP0(scalar s)) {
	scale2(r, scalarInv(s));
}

_REG void invScale3(_A0(vec3 *r), _FP0(scalar s)) {
	scale3(r, scalarInv(s));
}

// Inverting a vector
_REG void inverted2(_A0(vec2 *r), _A1(vec2 *a)) {
	r->x = -a->x;
	r->y = -a->y;
}

_REG void inverted3(_A0(vec3 *r), _A1(vec3 *a)) {
	r->x = -a->x;
	r->y = -a->y;
	r->z = -a->z;
}

_REG void invert2(_A0(vec2 *r)) {
	r->x = -r->x;
	r->y = -r->y;
}

_REG void invert3(_A0(vec3 *r)) {
	r->x = -r->x;
	r->y = -r->y;
	r->z = -r->z;
}

// Vector-vector addition
_REG void add2(_D0(vec2 *r), _A0(vec2 *a), _A1(vec2 *b)) {
	r->x = a->x + b->x;
	r->y = a->y + b->y;
}

_REG void add3(_D0(vec3 *r), _A0(vec3 *a), _A1(vec3 *b)) {
	r->x = a->x + b->x;
	r->y = a->y + b->y;
	r->z = a->z + b->z;
}

_REG void addTo2(_A0(vec2 *r), _A1(vec2 *a)) {
	r->x += a->x;
	r->y += a->y;
}

_REG void addTo3(_A0(vec3 *r), _A1(vec3 *a)) {
	r->x += a->x;
	r->y += a->y;
	r->z += a->z;
}

// Vector-vector subtraction
_REG void sub2(_D0(vec2 *r), _A0(vec2 *a), _A1(vec2 *b)) {
	r->x = a->x - b->x;
	r->y = a->y - b->y;
}

_REG void sub3(_D0(vec3 *r), _A0(vec3 *a), _A1(vec3 *b)) {
	r->x = a->x - b->x;
	r->y = a->y - b->y;
	r->z = a->z - b->z;
}

_REG void subFrom2(_A0(vec2 *r), _A1(vec2 *a)) {
	r->x -= a->x;
	r->y -= a->y;
}

_REG void subFrom3(_A0(vec3 *r), _A1(vec3 *a)) {
	r->x -= a->x;
	r->y -= a->y;
	r->z -= a->z;
}

// Vector-vector multiplication
_REG scalar dot2(_A0(vec2 *a), _A1(vec2 *b)) {
	return ( a->x*b->x + a->y*b->y );
}

_REG scalar dot3(_A0(vec3 *a), _A1(vec3 *b)) {
	return( a->x*b->x + a->y*b->y + a->z*b->z );
}

_REG void cross(_D0(vec3 *r), _A0(vec3 *a), _A1(vec3 *b)) {
	r->x = a->y * b->z - a->z * b->y;
	r->y = a->x * b->z - a->z * b->x;
	r->z = a->x * b->y - a->y * b->x;
}

// Vector sizes
_REG scalar length2(_A0(vec2 *v)) {
	return( scalarSqrt( dot2(v,v) ) );
}

_REG scalar length3(_A0(vec3 *v)) {
	return( scalarSqrt( dot3(v,v) ) );
}

_REG scalar dist2(_A0(vec2 *a), _A1(vec2 *b)) {
	vec2 t;
	return( length2(makeVec2(&t, a->x - b->x, a->y - b->y)));
}

_REG scalar dist3(_A0(vec3 *a), _A1(vec3 *b)) {
	vec3 t;
	return( length3(makeVec3(&t, a->x - b->x, a->y - b->y, a->z - b->z)));
}

// Vector normalization
_REG void normalized2(_A0(vec2 *r), _A1(vec2 *v)) {
	div2( r, v, length2(v) );
}

_REG void normalized3(_A0(vec3 *r), _A1(vec3 *v)) {
	div3( r, v, length3(v) );
}

_REG void normalize2(_A0(vec2 *r)) {
	invScale2( r, length2(r) );
}

#if M68K==0
_REG void normalize3(_A0(vec3 *r)) {
	invScale3( r, length3(r) );
}
#endif

// Projection functions
_REG void projectedOnto2(_D0(vec2 *r), _A0(vec2 *a), _A1(vec2 *b)) {
	mult2( r, a, dot2(a, b) );
	invScale2( r, length2(a) * length2(b) );
}

_REG void projectedOnto3(_D0(vec3 *r), _A0(vec3 *a), _A1(vec3 *b)) {
	mult3( r, a, dot3(a, b) );
	invScale3( r, length3(a) * length3(b) );
}

_REG void projectOnto2(_A0(vec2 *a), _A1(vec2 *r)) {
	mult2( r, a, dot2(a, r) );
	invScale2( r, length2(a) * length2(r) );
}

_REG void projectOnto3(_A0(vec3 *a), _A1(vec3 *r)) {
	mult3( r, a, dot3(a, r) );
	invScale3( r, length3(a) * length3(r) );
}

// Reflection functions
_REG void reflected2(_D0(vec2 *r), _A0(vec2 *i), _A1(vec2 *n)) {
	mult2( r, n, dot2( n, i ) * ((scalar)2) );
	subFrom2( r, i );
}

_REG void reflected3(_D0(vec3 *r), _A0(vec3 *i), _A1(vec3 *n)) {
	mult3( r, n, dot3( n, i ) * ((scalar)2) );
	subFrom3( r, i );
}

_REG void reflect2(_A0(vec2 *r), _A1(vec2 *n)) {
	vec2 i = *r;
	mult2( r, n, dot2( n, &i ) * ((scalar)2) );
	subFrom2( r, &i );
}

_REG void reflect3(_A0(vec3 *r), _A1(vec3 *n)) {
	vec3 i = *r;	
	mult3( r, n, dot3( n, &i ) * ((scalar)2) );
	subFrom3( r, &i );
}

// Polar coordinates
_REG scalar vecAtan(_A0(vec2 *c)) {
	return scalarAtan2( c->x, c->y );
}

// Rounding error avoidance
_REG void extendTo(_A0(vec3 *r), _A1(vec3 *d)) {
// 	if( d->x > 0 ) {
// 		r->x = nextScalar( r->x );
// 	}
// 	else {
// 		r->x = prevScalar( r->x );
// 	}
// 	if( d->y > 0 ) {
// 		r->y = nextScalar( r->y );
// 	}
// 	else {
// 		r->y = prevScalar( r->y );
// 	}
// 	if( d->z > 0 ) {
// 		r->z = nextScalar( r->z );
// 	}
// 	else {
// 		r->z = prevScalar( r->z );
// 	}
	vec3 ext;
	mult3( &ext, d, 0.1 );
	addTo3( r, &ext );
}


// Debugging
_REG void vecPrintf3(_A0(vec3 *v)) {
	printf( "%f, %f, %f\n", v->x, v->y, v->z );
}

_REG int vec3Cmp(_A0(const vec3 *a), _A1(const vec3 *b)) {
	int d;
	d = scalarCmp(a->x, b->x); if(d) return d;
	d = scalarCmp(a->y, b->y); if(d) return d;
	d = scalarCmp(a->z, b->z);       return d;
}
