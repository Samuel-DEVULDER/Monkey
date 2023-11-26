/**
 * Matrices, 4x4 only.
 * Matrix inverse lifted from MESA and adapted. Holy christ.
 * (c) 2010 L. Diener
 */

/**
 * Matrices look like so:
 * 0  1  2  3
 * 4  5  6  7
 * 8  9  10 11
 * 12 13 14 15
 */

#include "matrices.h"
#include "vectors.h"

_REG void matrixMult(_D0(matrix* r), _A0(matrix *a), _A1(matrix *b)) {
	int x,y;
	for( x = 0; x < 4; x++ ) {
		for( y = 0; y < 4; y++ ) {
			r->v[4*y+x] =
				a->v[0+4*y] * b->v[0+x] +
				a->v[1+4*y] * b->v[4+x] +
				a->v[2+4*y] * b->v[8+x] +
				a->v[3+4*y] * b->v[12+x];
		}
	}
}

inline _REG int matrixInverse(_A0(matrix* r), _A1(matrix *in)) {
	scalar det;
	int i;

	scalar* m = in->v;
	
	r->v[0] =
		m[5]*m[10]*m[15] -
		m[5]*m[11]*m[14] -
		m[9]*m[6]*m[15] +
		m[9]*m[7]*m[14] +
		m[13]*m[6]*m[11] -
		m[13]*m[7]*m[10];
	r->v[4] =
		-m[4]*m[10]*m[15] +
		m[4]*m[11]*m[14] +
		m[8]*m[6]*m[15] -
		m[8]*m[7]*m[14] -
		m[12]*m[6]*m[11] +
		m[12]*m[7]*m[10];
	r->v[8] =
		m[4]*m[9]*m[15] -
		m[4]*m[11]*m[13] -
		m[8]*m[5]*m[15] +
		m[8]*m[7]*m[13] +
		m[12]*m[5]*m[11] -
		m[12]*m[7]*m[9];
	r->v[12] =
		-m[4]*m[9]*m[14] +
		m[4]*m[10]*m[13] +
		m[8]*m[5]*m[14] -
		m[8]*m[6]*m[13] -
		m[12]*m[5]*m[10] +
		m[12]*m[6]*m[9];
	r->v[1] =
		-m[1]*m[10]*m[15] +
		m[1]*m[11]*m[14] +
		m[9]*m[2]*m[15] -
		m[9]*m[3]*m[14] -
		m[13]*m[2]*m[11] +
		m[13]*m[3]*m[10];
	r->v[5] =
		m[0]*m[10]*m[15] -
		m[0]*m[11]*m[14] -
		m[8]*m[2]*m[15] +
		m[8]*m[3]*m[14] +
		m[12]*m[2]*m[11] -
		m[12]*m[3]*m[10];
	r->v[9] =
		-m[0]*m[9]*m[15] +
		m[0]*m[11]*m[13] +
		m[8]*m[1]*m[15] -
		m[8]*m[3]*m[13] -
		m[12]*m[1]*m[11] +
		m[12]*m[3]*m[9];
	r->v[13] =
		m[0]*m[9]*m[14] -
		m[0]*m[10]*m[13] -
		m[8]*m[1]*m[14] +
		m[8]*m[2]*m[13] +
		m[12]*m[1]*m[10] -
		m[12]*m[2]*m[9];
	r->v[2] =
		m[1]*m[6]*m[15] -
		m[1]*m[7]*m[14] -
		m[5]*m[2]*m[15] +
		m[5]*m[3]*m[14] +
		m[13]*m[2]*m[7] -
		m[13]*m[3]*m[6];
	r->v[6] =
		-m[0]*m[6]*m[15] +
		m[0]*m[7]*m[14] +
		m[4]*m[2]*m[15] -
		m[4]*m[3]*m[14] -
		m[12]*m[2]*m[7] +
		m[12]*m[3]*m[6];
	r->v[10] =
		m[0]*m[5]*m[15] -
		m[0]*m[7]*m[13] -
		m[4]*m[1]*m[15] +
		m[4]*m[3]*m[13] +
		m[12]*m[1]*m[7] -
		m[12]*m[3]*m[5];
	r->v[14] =
		-m[0]*m[5]*m[14] +
		m[0]*m[6]*m[13] +
		m[4]*m[1]*m[14] -
		m[4]*m[2]*m[13] -
		m[12]*m[1]*m[6] +
		m[12]*m[2]*m[5];
	r->v[3] =
		-m[1]*m[6]*m[11] +
		m[1]*m[7]*m[10] +
		m[5]*m[2]*m[11] -
		m[5]*m[3]*m[10] -
		m[9]*m[2]*m[7] +
		m[9]*m[3]*m[6];
	r->v[7] =
		m[0]*m[6]*m[11] -
		m[0]*m[7]*m[10] -
		m[4]*m[2]*m[11] +
		m[4]*m[3]*m[10] +
		m[8]*m[2]*m[7] -
		m[8]*m[3]*m[6];
	r->v[11] =
		-m[0]*m[5]*m[11] +
		m[0]*m[7]*m[9] +
		m[4]*m[1]*m[11] -
		m[4]*m[3]*m[9] -
		m[8]*m[1]*m[7] +
		m[8]*m[3]*m[5];
	r->v[15] =
		m[0]*m[5]*m[10] -
		m[0]*m[6]*m[9] -
		m[4]*m[1]*m[10] +
		m[4]*m[2]*m[9] +
		m[8]*m[1]*m[6] -
		m[8]*m[2]*m[5];

	det =
		m[0]*r->v[0] +
		m[1]*r->v[4] +
		m[2]*r->v[8] +
		m[3]*r->v[12];
		
	if (det == 0) {
		return 0;
	}

	det = scalarInv(det);

	for( i = 0; i < 16; i++) {
		r->v[i] = r->v[i] * det;
	}

	return 1;
}

#if M68K==0
static _REG void swap(_A0(float *src),_A1(float *dst)) {
	float t = *src;
	*src = *dst; *dst = t;
}

_REG void matrixTransposed(_A0(matrix *r)) {
	swap(&r->v[1],&r->v[4]);
	swap(&r->v[2],&r->v[8]);
	swap(&r->v[3],&r->v[12]);

	swap(&r->v[6],&r->v[9]);
	swap(&r->v[7],&r->v[13]);

	swap(&r->v[11],&r->v[14]);
}

_REG void matrixTranspose(_A0(matrix* r), _A1(matrix *m)) {
	r->v[0] = m->v[0];
	r->v[1] = m->v[4];
	r->v[2] = m->v[8];
	r->v[3] = m->v[12];
	r->v[4] = m->v[1];
	r->v[5] = m->v[5];
	r->v[6] = m->v[9];
	r->v[7] = m->v[13];
	r->v[8] = m->v[2];
	r->v[9] = m->v[6];
	r->v[10] = m->v[10];
	r->v[11] = m->v[14];
	r->v[12] = m->v[3];
	r->v[13] = m->v[7];
	r->v[14] = m->v[11];
	r->v[15] = m->v[15];
}
#endif

_REG void matrixNormalTransform(_A0(matrix* r), _A1(matrix *m)) {
	m->v[3] = 0;
	m->v[7] = 0;
	m->v[11] = 0;
	m->v[15] = 1;
	matrixInverse( r, m );
	matrixTransposed( r);
}

#if M68K==0
_REG void matrixApply(_D0(vec3* r), _A0(matrix *a), _A1(vec3 *b)) {
	r->x = a->v[0] * b->x + a->v[1] * b->y + a->v[2]  * b->z + a->v[3];
	r->y = a->v[4] * b->x + a->v[5] * b->y + a->v[6]  * b->z + a->v[7];
	r->z = a->v[8] * b->x + a->v[9] * b->y + a->v[10] * b->z + a->v[11];
}
#endif

#if M68K==0
_REG void matrixApplyNormal(_D0(vec3* r), _A0(matrix *a), _A1(vec3 *b)) {
	r->x = a->v[0] * b->x + a->v[1] * b->y + a->v[2]  * b->z;
	r->y = a->v[4] * b->x + a->v[5] * b->y + a->v[6]  * b->z;
	r->z = a->v[8] * b->x + a->v[9] * b->y + a->v[10] * b->z;
	normalize3( r );
}
#endif

#if M68K==0
_REG void matrixApplyPerspective(_D0(vec3* r), _A0(matrix *a), _A1(vec3 *b)) {
	scalar w;
	r->x = a->v[0] * b->x + a->v[1] * b->y + a->v[2]  * b->z + a->v[3];
	r->y = a->v[4] * b->x + a->v[5] * b->y + a->v[6]  * b->z + a->v[7];
	r->z = a->v[8] * b->x + a->v[9] * b->y + a->v[10] * b->z + a->v[11];
	w = a->v[12] * b->x + a->v[13] * b->y + a->v[14] * b->z + a->v[15];
	
	if(scalarAbs(w) > 0.00001) {
		invScale3(r, w);
	}
}
#endif

_REG void matrixId(_A0(matrix* m)) {
	m->v[0]  = 1; m->v[1]  = 0; m->v[2]  = 0; m->v[3]  = 0;
	m->v[4]  = 0; m->v[5]  = 1; m->v[6]  = 0; m->v[7]  = 0;
	m->v[8]  = 0; m->v[9]  = 0; m->v[10] = 1; m->v[11] = 0;
	m->v[12] = 0; m->v[13] = 0; m->v[14] = 0; m->v[15] = 1;
}

_REG void matrixRotY(_A0(matrix* m), _FP0(scalar a)) {
	scalar ca = scalarCos(a), sa = scalarSin(a);
	m->v[0]  = ca; m->v[1]  = 0; m->v[2]  = -sa; m->v[3]  = 0;
	m->v[4]  = 0;  m->v[5]  = 1; m->v[6]  = 0;   m->v[7]  = 0;
	m->v[8]  = sa; m->v[9]  = 0; m->v[10] = ca;  m->v[11] = 0;
	m->v[12] = 0;  m->v[13] = 0; m->v[14] = 0;   m->v[15] = 1;
}
