/**
 * Matrices, 4x4 only.
 * (c) 2010 L. Diener
 */

/**
 * Matrices look like so:
 * 0  1  2  3
 * 4  5  6  7
 * 8  9  10 11
 * 12 13 14 15
 */

#ifndef __MATRICES_H__
#define __MATRICES_H__

#include "scalars.h"
#include "vectors.h"

typedef struct matrix {
	scalar v[16];
} matrix;

_REG void matrixMult(_D0(matrix* r), _A0(matrix *a), _A1(matrix *b));
_REG void matrixTranspose(_A0(matrix* r), _A1(matrix *m));
_REG void matrixTransposed(_A0(matrix* r));
_REG int  matrixInverse(_A0(matrix* r), _A1(matrix *in));
_REG void matrixNormalTransform(_A0(matrix* r), _A1(matrix *m));
_REG void matrixApply(_D0(vec3* r), _A0(matrix *a), _A1(vec3 *b));
_REG void matrixApplyNormal(_D0(vec3* r), _A0(matrix *a), _A1(vec3 *b));
_REG void matrixId(_A0(matrix* m));
_REG void matrixRotY(_A0(matrix* m), _FP0(scalar a));
_REG void matrixApplyPerspective(_D0(vec3* r), _A0(matrix *a), _A1(vec3 *b));

#define matrixScale(m,x,y,z) do { \
	(m)->v[0]  = ((scalar)1)/(x); (m)->v[1]  = 0; (m)->v[2]  = 0; (m)->v[3]  = 0;\
	(m)->v[4]  = 0; (m)->v[5]  = ((scalar)1)/(y); (m)->v[6]  = 0; (m)->v[7]  = 0;\
	(m)->v[8]  = 0; (m)->v[9]  = 0; (m)->v[10] = ((scalar)1)/(z); (m)->v[11] = 0;\
	(m)->v[12] = 0; (m)->v[13] = 0; (m)->v[14] = 0; (m)->v[15] = 1;\
} while(0);


#define matrixTranslate(m,x,y,z) do {\
	(m)->v[0]  = 1; (m)->v[1]  = 0; (m)->v[2]  = 0; (m)->v[3]  = -x;\
	(m)->v[4]  = 0; (m)->v[5]  = 1; (m)->v[6]  = 0; (m)->v[7]  = -y;\
	(m)->v[8]  = 0; (m)->v[9]  = 0; (m)->v[10] = 1; (m)->v[11] = -z;\
	(m)->v[12] = 0; (m)->v[13] = 0; (m)->v[14] = 0; (m)->v[15] = 1;\
} while(0)

#define matrixPerspective(m, degrees, aspect, near, far) do { \
        scalar f = ((scalar)1) / scalarTan((degrees)*scalarPI/((scalar)180)/((scalar)2)); \
	(m)->v[0]  = f/(aspect); (m)->v[1] = 0;  (m)->v[2]  = 0;                     (m)->v[3]  = 0; \
	(m)->v[4]  = 0;        (m)->v[5] = f;  (m)->v[6]  = 0;                     (m)->v[7]  = 0; \
	(m)->v[8]  = 0;        (m)->v[9] = 0;  (m)->v[10] = ((far)+(near))/((near)-(far)); (m)->v[11] = ((scalar)2)*(far)*(near)/((near)-(far)); \
	(m)->v[12] = 0;        (m)->v[13] = 0; (m)->v[14] = ((scalar)-1);          (m)->v[15] = 0; \
} while(0)

#endif
