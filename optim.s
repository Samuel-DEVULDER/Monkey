*	extern _Q_memcpy (REGP("a0",void *dest), REGP("a1",void *src), REGP("d0",int count));

		XDEF    _Q_memcpy
		XDEF    @Q_memcpy


		cnop    0,4
_Q_memcpy

*****   stackframe

		rsreset
		rs.l    1
.dest           rs.l    1
.src            rs.l    1
.count		rs.l	1

 ifeq REGPARM
		move.l  .dest(sp),a0
		move.l  .src(sp),a1
		move.l	.count(sp),d0
 endc
@Q_memcpy
		xref	_mc68080
		tst.b	_mc68080
		beq	.1
* AMMX optimized version
		tst.l	d0
		ble.s	.Q_memcpy0
		exg	a0,a1
.loop
		dc.w    $fe18,$0101          ; LOAD   (a0)+,d1
		dc.w    $fe19,$1024          ; STOREC d1,d0,(a1)+
		subq.l  #8,d0                ; Decrement length
		bhi.s   .loop                ; Repeat until D0 <= 0$
		rts
.1
		move.l	d0,d1
		ble.s	.Q_memcpy0
		lsr.l	#4,d1
		subq.l	#1,d1
		bmi.s	.3
		swap	d0
		lsr.w	#4,d0
.2	
	        move.l	(a1)+,(a0)+
	        move.l	(a1)+,(a0)+
	        move.l	(a1)+,(a0)+
	        move.l	(a1)+,(a0)+
	        dbf	d1,.2
		dbf	d0,.2
		swap	d0
.3		
		and.w	#15,d0
		jmp		.Q_memcpy0(d0.w*8)
.Q_memcpy0							; 8 bytes per instruction
		rts
		nop
		nop
		nop		
.Q_memcpy1
		move.b	(a1)+,(a0)+
		rts
		nop
		nop
.Q_memcpy2
		move.w	(a1)+,(a0)+
		rts
		nop
		nop
.Q_memcpy3
		move.w	(a1)+,(a0)+
		move.b	(a1)+,(a0)+
		rts
		nop
.Q_memcpy4
		move.l	(a1)+,(a0)+
		rts
		nop
		nop
.Q_memcpy5
		move.l	(a1)+,(a0)+
		move.b	(a1)+,(a0)+
		rts
		nop
.Q_memcpy6
		move.l	(a1)+,(a0)+
		move.w	(a1)+,(a0)+
		rts
		nop
.Q_memcpy7
		move.l	(a1)+,(a0)+
		move.w	(a1)+,(a0)+
		move.b	(a1)+,(a0)+
		rts
.Q_memcpy8
		move.l	(a1)+,(a0)+
		move.l	(a1)+,(a0)+
		rts
		nop
.Q_memcpy9
		move.l	(a1)+,(a0)+
		move.l	(a1)+,(a0)+
		move.b	(a1)+,(a0)+
		rts
.Q_memcpy10
		move.l	(a1)+,(a0)+
		move.l	(a1)+,(a0)+
		move.w	(a1)+,(a0)+
		rts
.Q_memcpy11
		move.l	(a1)+,(a0)+
		bra.s	.Q_memcpy7
		nop
		nop
.Q_memcpy12
		move.l	(a1)+,(a0)+
		move.l	(a1)+,(a0)+
		move.l	(a1)+,(a0)+
		rts
.Q_memcpy13
		move.l	(a1)+,(a0)+
		bra.s	.Q_memcpy9
		nop
		nop
.Q_memcpy14
		move.l	(a1)+,(a0)+
		bra.s	.Q_memcpy10
		nop
		nop
.Q_memcpy15
		move.l	(a1)+,(a0)+
		move.l	(a1)+,(a0)+
		move.l	(a1)+,(a0)+
		move.w	(a1)+,(a0)+
		move.b	(a1)+,(a0)+
		rts

* void Q_memset (REGP("a0",void *dest), REGP("d0", int fill), REGP("d1", int count))

		cnop    0,4

		XDEF    _Q_memset
		XDEF    @Q_memset

_Q_memset

*****   stackframe

		rsreset
		rs.l    1
.dest           rs.l    1
.fill           rs.l    1
.count		rs.l	1

 ifeq REGPARM
		move.l  .dest(sp),a0
		move.l  .fill(sp),d0
		move.l	.count(sp),d1
 endc
@Q_memset
		exg	d0,d1

		tst.b	_mc68080
		beq	.1
* AMMX optimized version
		tst.l	d0
		ble	.0
		move.l	a0,a1
		dc.w	$FE3F,$1101 	     ; VPERM #$77777777,D1,D1,D1
		dc.w	$7777,$7777
.loop
		dc.w    $fe19,$1024          ; STOREC d1,d0,(a1)+
		subq.l  #8,d0                ; Decrement length
		bhi.s   .loop                ; Repeat until D0 <= 0$
		rts
.1
		move.w	d1,-(sp)
		move.b	d1,(sp)
		move.w	(sp),d1
		swap	d1
		move.w	(sp)+,d1

*		and.l	#255,d1
*		mulu.l	#$01010101,d1

		lsr.l	#1,d0
		bcc.s	.2
		move.b	d1,(a0)+
.2	
		lsr.l	#1,d0
		bcc.s	.4
		move.w	d1,(a0)+
.4
		subq.l	#1,d0
		bmi.s	.0
.5		
		move.l	d1,(a0)+
		dbf		d0,.5
		clr.w	d0
		subq.l	#1,d0
		bcc.s	.5
.0
		rts
		
		cnop	0,4

		xdef	___mulsi3
___mulsi3
		move.l	(sp)+,a0
		move.l	(sp),d0
		mulu.l	4(sp),d0
		jmp	(a0)
		
		cnop	0,4

* int    LongSwap (REGP("a0",int *l))

		xdef	_LongSwap
		xdef	@LongSwap
_LongSwap
 ifeq REGPARM
		move.l  4(sp),a0
 endc
@LongSwap
		move.l	(a0),d0
		ror.w	#8,d0
		swap	d0
		ror.w	#8,d0
		move.l	d0,(a0)
		rts

* fp0 = 1/fsqrt(fp0) (\1=fp0, \2=d1 (trashed), \3=fp1 (trashed))
rsqrt	macro
		fmove.s \1,\2
		lsr.l	#1,\2
		sub.l	#$5f3759df,\2
		fmul.s	#0.5,\1
		neg.l	\2
		fmul.s	\2,\1
		fneg.s	\2,\3
		fmul.s  \2,\1
		fsub.s  #1.5,\1
		fmul.x  \3,\1
	endm

 ifne M68K

* _REG void matrixApplyPerspective(_D0(vec3* r), _A0(matrix *a), _A1(vec3 *b)) {
*	scalar w;
*	r->x = a->v[0] * b->x + a->v[1] * b->y + a->v[2]  * b->z + a->v[3];
*	r->y = a->v[4] * b->x + a->v[5] * b->y + a->v[6]  * b->z + a->v[7];
*	r->z = a->v[8] * b->x + a->v[9] * b->y + a->v[10] * b->z + a->v[11];
*	w = a->v[12] * b->x + a->v[13] * b->y + a->v[14] * b->z + a->v[15];
*	
*	if(scalarAbs(w) > 0.00001) {
*		invScale3(r, w);
*	}
* }

		xdef	_matrixApplyPerspective
		xdef	@matrixApplyPerspective
_matrixApplyPerspective
 ifeq REGPARM
		movem.l  4(sp),d0/a0/a1
 endc
@matrixApplyPerspective
		movem.l	d2-d3,-(sp)
		fmovem	fp2-fp5,-(sp)

		movem.l	(a1),d1/d2/d3
		
*	r->x = a->v[0] * b->x + a->v[1] * b->y + a->v[2]  * b->z + a->v[3];
		fmove.s	(a0)+,fp0
		fmove.s	(a0)+,fp1
		fmove.s (a0)+,fp2
		fmul.s	d1,fp0
		fmul.s	d2,fp1
		fadd.s	(a0)+,fp0
		fmul.s	d3,fp2
		
*	r->y = a->v[4] * b->x + a->v[5] * b->y + a->v[6]  * b->z + a->v[7];
		fmove.s	(a0)+,fp3
		fadd.s	fp1,fp0
		fmove.s	(a0)+,fp1
		fadd.s	fp2,fp0
		fmul.s	d1,fp3
		fmove.s (a0)+,fp2
		fmul.s	d2,fp1
		fadd.s	(a0)+,fp3
		fmul.s	d3,fp2

*	r->z = a->v[8] * b->x + a->v[9] * b->y + a->v[10] * b->z + a->v[11];
		fmove.s	(a0)+,fp4
		fadd.s	fp1,fp3
		fmove.s (a0)+,fp1
		fadd.s	fp2,fp3
		fmul.s	d1,fp4
		fmove.s	(a0)+,fp2
		fmul.s	d2,fp1
		fadd.s	(a0)+,fp4
		fmul.s	d3,fp2
		
*	w = a->v[12] * b->x + a->v[13] * b->y + a->v[14] * b->z + a->v[15];
		fmove.s	(a0)+,fp5
		fadd.s	fp1,fp4
		fmove.s (a0)+,fp1
		fadd.s	fp2,fp4
		fmul.s	d1,fp5
		fmove.s	(a0)+,fp2
		fmul.s	d2,fp1
		fadd.s	(a0)+,fp5
		fmul.s	d3,fp2
		fadd	fp1,fp5
		fmove.b	#1,fp1
		fadd	fp2,fp5
		move.l	d0,a1

*	if(scalarAbs(w) > 0.00001) {
		fabs	fp5,fp2
		fdiv	fp5,fp1
		fcmp.s	#0.00001,fp2
		fble	.1
		fmul	fp1,fp0
		fmul	fp1,fp3
		fmul	fp1,fp4
.1
		fmove.s	fp0,(a1)+
		fmove.s	fp3,(a1)+
		fmove.s	fp4,(a1)
		
		fmovem	(sp)+,fp2-fp5
		movem.l	(sp)+,d2-d3
		rts

@matrixApplyPerspective_orig
		fmovem	fp2-fp7,-(sp)

		fmove.s	(a1)+,fp0
		fmove.s	(a1)+,fp1
		fmove.s (a1),fp2
		
*	r->x = a->v[0] * b->x + a->v[1] * b->y + a->v[2]  * b->z + a->v[3];
		fmove.s	(a0)+,fp3
		fmove.s	(a0)+,fp4
		fmove.s (a0)+,fp5
		fmul	fp0,fp3
		fmul	fp1,fp4
		fmul	fp2,fp5
		fadd.s	(a0)+,fp3
		fadd	fp4,fp3

*	r->y = a->v[4] * b->x + a->v[5] * b->y + a->v[6]  * b->z + a->v[7];
		fmove.s	(a0)+,fp4
		fadd	fp5,fp3
		fmove.s (a0)+,fp5
		fmove.s	(a0)+,fp6
		fmul	fp0,fp4
		fmul	fp1,fp5
		fmul	fp2,fp6
		fadd.s	(a0)+,fp4
		fadd	fp5,fp4

*	r->z = a->v[8] * b->x + a->v[9] * b->y + a->v[10] * b->z + a->v[11];
		fmove.s	(a0)+,fp5
		fadd	fp6,fp4
		fmove.s (a0)+,fp6
		fmove.s	(a0)+,fp7
		fmul	fp0,fp5
		fmul	fp1,fp6
		fmul	fp2,fp7
		fadd.s	(a0)+,fp5
		fadd	fp6,fp5
		fadd	fp7,fp5
		
*	w = a->v[12] * b->x + a->v[13] * b->y + a->v[14] * b->z + a->v[15];
		fmul.s	(a0)+,fp0
		fmul.s	(a0)+,fp1
		fmul.s	(a0)+,fp2
		fadd.s	(a0),fp0
		fadd	fp1,fp0

		fmove.b	#1,fp6
		fadd	fp2,fp0

*	if(scalarAbs(w) > 0.00001) {
		fabs	fp0,fp1
		fdiv	fp0,fp6		; superscalar --> prepare div
		fcmp.s	#0.00001,fp1
		fble	.1
		fmul	fp6,fp3
		fmul	fp6,fp4
		fmul	fp6,fp5
.1
		move.l	d0,a1		
		fmove.s	fp3,(a1)+
		fmove.s	fp4,(a1)+
		fmove.s	fp5,(a1)
		
		fmovem	(sp)+,fp2-fp7
		rts

* _REG void normalize3(_A0(vec3 *r)) {
*	invScale3( r, length3(r) );
* }

		xdef	_normalize3
		xdef	@normalize3
_normalize3
 ifeq REGPARM
		move.l  4(sp),a0
 endc
@normalize3
		move.l	(sp)+,a1
		fmovem	fp2-fp5,-(sp)

		fmove.s	(a0)+,fp3
		fmove.s	(a0)+,fp4
		fmove.s (a0),fp5
		
		fmove	fp3,fp0
		fmove	fp4,fp1
		fmove	fp5,fp2
		
		fmul	fp0,fp0
		fmul	fp1,fp1
		fmul	fp2,fp2
		
		fadd	fp1,fp0
		fadd	fp2,fp0

* fp0=1/sqrt(fp0)
		rsqrt	fp0,d1,fp1

		fmul	fp0,fp3
		fmul	fp0,fp4
		fmul	fp0,fp5

		subq.l	#8,a0
		fmove.s	fp3,(a0)+
		fmove.s	fp4,(a0)+
		fmove.s	fp5,(a0)

		fmovem	(sp)+,fp2-fp5
		jmp	(a1)
		
* _REG void matrixApplyNormal(_D0(vec3* r), _A0(matrix *a), _A1(vec3 *b)) {
*	r->x = a->v[0] * b->x + a->v[1] * b->y + a->v[2]  * b->z;
*	r->y = a->v[4] * b->x + a->v[5] * b->y + a->v[6]  * b->z;
*	r->z = a->v[8] * b->x + a->v[9] * b->y + a->v[10] * b->z;
*	normalize3( r );
* }

		xdef	_matrixApplyNormal
		xdef	@matrixApplyNormal
_matrixApplyNormal
 ifeq REGPARM
		movem.l  4(sp),d0/a0/a1
 endc
@matrixApplyNormal
		fmovem	fp2-fp7,-(sp)

		fmove.s	(a1)+,fp0
		fmove.s	(a1)+,fp1
		fmove.s (a1),fp2
		
		move.l	d0,a1

*	r->x = a->v[0] * b->x + a->v[1] * b->y + a->v[2]  * b->z;
		fmove.s	(a0)+,fp3
		fmove.s	(a0)+,fp4
		fmove.s (a0)+,fp5
		fmul	fp0,fp3
		fmul	fp1,fp4
		fmul	fp2,fp5
		addq.l	#4,a0
		fadd	fp4,fp3
		fadd	fp5,fp3

*	r->y = a->v[4] * b->x + a->v[5] * b->y + a->v[6]  * b->z;
		fmove.s	(a0)+,fp4
		fmove.s	(a0)+,fp5
		fmove.s (a0)+,fp6
		fmul	fp0,fp4
		fmul	fp1,fp5
		fmul	fp2,fp6
		addq.l	#4,a0
		fadd	fp5,fp4
		fadd	fp6,fp4

*	r->z = a->v[8] * b->x + a->v[9] * b->y + a->v[10] * b->z;
		fmove.s	(a0)+,fp5
		fmove.s	(a0)+,fp6
		fmove.s (a0)+,fp7
		fmul	fp0,fp5
		fmul	fp1,fp6
		fmul	fp2,fp7
		fadd	fp6,fp5
		fadd	fp7,fp5
		
*	normalize3( r );
		fmul	fp0,fp0
		fmul	fp1,fp1
		fmul	fp2,fp2
		fadd	fp1,fp0
		fadd	fp2,fp0
		rsqrt	fp0,d1,fp1
		fmul	fp0,fp3
		fmul	fp0,fp4
		fmul	fp0,fp5
		
		fmove.s	fp3,(a1)+
		fmove.s	fp4,(a1)+
		fmove.s	fp5,(a1)
		
		fmovem	(sp)+,fp2-fp7
		rts
		
* _REG void matrixApply(_D0(vec3* r), _A0(matrix *a), _A1(vec3 *b)) {
*	r->x = a->v[0] * b->x + a->v[1] * b->y + a->v[2]  * b->z + a->v[3];
*	r->y = a->v[4] * b->x + a->v[5] * b->y + a->v[6]  * b->z + a->v[7];
*	r->z = a->v[8] * b->x + a->v[9] * b->y + a->v[10] * b->z + a->v[11];
* }

		xdef	_matrixApply
		xdef	@matrixApply
_matrixApply
 ifeq REGPARM
		movem.l 4(sp),d0/a0/a1
 endc
@matrixApply
		fmovem	fp2-fp7,-(sp)

		fmove.s	(a1)+,fp0
		fmove.s	(a1)+,fp1
		fmove.s (a1),fp2
		
*	r->x = a->v[0] * b->x + a->v[1] * b->y + a->v[2]  * b->z + a->v[3];
		fmove.s	(a0)+,fp3
		fmove.s	(a0)+,fp4
		fmove.s (a0)+,fp5
		fmul	fp0,fp3
		fmul	fp1,fp4
		fmul	fp2,fp5
		fadd.s	(a0)+,fp3
		fadd	fp4,fp3
		fadd	fp5,fp3

*	r->y = a->v[4] * b->x + a->v[5] * b->y + a->v[6]  * b->z + a->v[7];
		fmove.s	(a0)+,fp4
		fmove.s (a0)+,fp5
		fmove.s	(a0)+,fp6
		fmul	fp0,fp4
		fmul	fp1,fp5
		fmul	fp2,fp6
		fadd.s	(a0)+,fp4
		fadd	fp5,fp4
		fadd	fp6,fp4

*	r->z = a->v[8] * b->x + a->v[9] * b->y + a->v[10] * b->z + a->v[11];
		fmove.s	(a0)+,fp5
		fmove.s (a0)+,fp6
		fmove.s	(a0)+,fp7
		fmul	fp0,fp5
		fmul	fp1,fp6
		fmul	fp2,fp7
		fadd.s	(a0)+,fp5
		fadd	fp6,fp5
		fadd	fp7,fp5

		move.l	d0,a1		
		fmove.s	fp3,(a1)+
		fmove.s	fp4,(a1)+
		fmove.s	fp5,(a1)
		
		fmovem	(sp)+,fp2-fp7
		rts

* _REG void matrixTransposed(_A0(matrix *r)) {
* 	swap(&r->v[1],&r->v[4]);
*	swap(&r->v[2],&r->v[8]);
*	swap(&r->v[3],&r->v[12]);
*
*	swap(&r->v[6],&r->v[9]);
*	swap(&r->v[7],&r->v[13]);
*
*	swap(&r->v[11],&r->v[14]);
*}

		xdef	_matrixTransposed
		xdef	@matrixTransposed
_matrixTransposed
 ifeq REGPARM
		move.l 4(sp),a0
 endc
@matrixTransposed
		move.l	1*4(a0),d0
		move.l	2*4(a0),d1
		move.l	3*4(a0),a1
		move.l	4*4(a0),1*4(a0)
		move.l	8*4(a0),2*4(a0)
		move.l	12*4(a0),3*4(a0)
		move.l	d0,4*4(a0)
		move.l	d1,8*4(a0)
		move.l	a1,12*4(a0)
		
		move.l	6*4(a0),d0
		move.l	7*4(a0),d1
		move.l	9*4(a0),6*4(a0)
		move.l	13*4(a0),7*4(a0)
		move.l	d0,9*4(a0)
		move.l	d1,13*4(a0)

		move.l	11*4(a0),d0
		move.l	14*4(a0),11*4(a0)
		move.l	d0,11*4(a0)

		rts

		xdef	_matrixTranspose
		xdef	@matrixTranspose
_matrixTranspose
 ifeq REGPARM
		move.l	4(sp),a0
		move.l	8(sp),a1
 endc
@matrixTranspose
		move.l (a1)+,(a0)+
		move.l 12(a1),(a0)+
		move.l 28(a1),(a0)+
		move.l 44(a1),(a0)+
		move.l (a1)+,(a0)+
		move.l 12(a1),(a0)+
		move.l 28(a1),(a0)+
		move.l 44(a1),(a0)+
		move.l (a1)+,(a0)+
		move.l 12(a1),(a0)+
		move.l 28(a1),(a0)+
		move.l 44(a1),(a0)+
		move.l (a1),(a0)+
		move.l 16(a1),(a0)+
		move.l 32(a1),(a0)+
		move.l 48(a1),(a0)
		rts

		xdef	_scalarInvSqrt
		xdef	@scalarInvSqrt
_scalarInvSqrt
 ifeq REGPARM
		fmove.s	4(sp),fp0
 endc
@scalarInvSqrt
*		tst.b	_mc68080
*		bne		.1
*		fmove	fp0,fp1
*		fsqrt	fp0
*		fdiv	fp1,fp0
*		fmove.s	fp0,d0		; <-- gcc :(
*		rts
*.1
		rsqrt	fp0,d0,fp1
		fmove.s	fp0,d0		; <-- gcc :(
		rts

 endc
 
* 	struct span {
*		int x, xmax;		
*		scalar dz;
*		scalar *zb;
*		colour *out;
*		real   d[3];
*#ifndef dx
*		real   dx[3];
*#endif
*	} span;

			rsreset
TRI_sx		rs.s	3
TRI_sy		rs.s	3
TRI_nx		rs.s	3
TRI_ny		rs.s	3
TRI_c		rs.s	3
TRI_xmin	rs.l	1
TRI_xmax	rs.l	1
TRI_ymin	rs.l	1
TRI_ymax	rs.l	1
TRI_width	rs.w	1
TRI_height	rs.w	1
TRI_zbuf	rs.l	1
TRI_pbuf	rs.l	1
TRI_pxmin	rs.l	1
TRI_pxmax	rs.l	1
TRI_x		rs.l	1
TRI_dz		rs.s	1
TRI_zb		rs.l	1
TRI_out		rs.l	1
TRI_d		rs.s	3
TRI_dx		equ		TRI_nx

			rsreset
VTX_nxt		rs.l	1
VTX_point	rs.s	3
VTX_normal	rs.s	3
VTX_pj_pt	rs.s	3
VTX_pj_nor	rs.s	3
VTX_color_x	rs.s	1
VTX_color_y	rs.s	1
VTX_color_z	rs.s	1
VTX_col		rs.l	1

			rsreset
TRIANGLE_ID	rs.l	1
TRIANGLE_VT	rs.l	3

		xdef _find_span
_find_span
 ifeq REGPARM
		move.l	4(sp),a0
 endc
@find_span
		movem.l	d2-d4,-(sp)
		fmove.d	fp2,-(sp)
		
		moveq	#0,d4
		
		movem.l	TRI_d(a0),d0-d2
		fmove.s	d0,fp0
		fmove.s	d1,fp1
		or.l	d1,d0
		fmove.s	d2,fp2
		or.l	d2,d0
		bpl.s	.8
		
		movem.l	TRI_dx(a0),d0-d2

		move.l	TRI_xmax(a0),d3
		sub.l	TRI_x(a0),d3
		bra.s	.3		
.1
		fadd.s	d1,fp1
.2
		fadd.s	d2,fp2
.3
		addq.l	#1,d4 
		cmp.l	d3,d4
		bge.b	.4

		fadd.s	d0,fp0
		fblt.b	.1
		fadd.s	d1,fp1
		fblt.b	.2
		fadd.s	d2,fp2
		fblt.b	.3
		
		fmove.s	fp0,TRI_d+0*4(a0)
		fmove.s	fp1,TRI_d+1*4(a0)
		fmove.s	fp2,TRI_d+2*4(a0)
		
		add.l	d4,TRI_x(A0)
		lsl.l	#2,d4
		add.l	d4,TRI_zb(A0)
		add.l	d4,TRI_out(A0)
.8
		moveq	#1,d0
.9
		fmove.d	(sp)+,fp2
		movem.l	(sp)+,d2-d4
		rts
.4
		moveq	#0,d0
		bra.s	.9
		
		xdef _draw_span_mono
_draw_span_mono
 ifeq REGPARM
		move.l	4(sp),a0
		move.l	8(sp),d0
 endc
@draw_span_mono
		movem.l	d2-d4/a2,-(sp)
		fmovem	fp2-fp3,-(sp)
		
		movem.l	TRI_d+0*4(a0),d1-d3
		
		fmove.s	TRI_c+0*4(a0),fp0
		fmove.s	TRI_c+1*4(a0),fp2
		fmove.s	TRI_c+2*4(a0),fp3
		fmul.s	d1,fp0
		fmul.s	d2,fp2
		fmul.s	d3,fp3
		fmove.s	d1,fp1
		fadd	fp2,fp0
		fmove.s	d2,fp2
		fadd	fp3,fp0		
		fmove.s	d3,fp3
		
		movem.l	TRI_dx+0*4(a0),d1-d3
		movem.l	TRI_dz(a0),d4/a1/a2
*		move.l	TRI_zb(a0),a1
*		move.l	TRI_out(a0),a2
		
.1
		fcmp.s	(a1)+,fp0
		fble.b	.2
		fmove.s	fp0,-4(a1)
		move.l	d0,(a2)
.2
		fadd.s	d1,fp1
		fblt.b	.9
		fadd.s	d2,fp2
		fblt.b	.9
		fadd.s	d3,fp3
		fblt.b	.9
		fadd.s	d4,fp0
		addq.l	#4,a2
		bra		.1
		
.9
		fmovem	(sp)+,fp2-fp3
		movem.l	(sp)+,d2-d4/a2
		rts

		xdef _draw_span
_draw_span
 ifeq REGPARM
		move.l	4(sp),a0
		move.l	8(sp),a1
 endc
@draw_span
		movem.l	d2-d4/a2-a4,-(sp)
		fmovem	fp2-fp6,-(sp)
		
		movem.l	TRI_d+0*4(a0),d0-d2
		
		fmove.s	TRI_c+0*4(a0),fp3
		fmove.s	TRI_c+1*4(a0),fp1
		fmove.s	TRI_c+2*4(a0),fp2
		fmul.s	d0,fp3
		fmul.s	d1,fp1
		fmul.s	d2,fp2
		
		fmove.s	d0,fp0
		fadd	fp1,fp3
		fmove.s	d1,fp1
		fadd	fp2,fp3
		fmove.s	d2,fp2
		
		movem.l	TRI_dx+0*4(a0),d0-d2		
		movem.l	TRI_dz(a0),d3/a2/a3
*		move.l	TRI_zb(a0),a2
*		move.l	TRI_out(a0),a3
		

.1
		fcmp.s	(a2)+,fp3
		fble.b	.2
		fmove.s	fp3,-4(a2)
	ifne 1
		move.l	TRIANGLE_VT+0*4(A1),A4
		fmove.s	VTX_color_y(A4),fp4
		fmul	fp1,fp4
		move.l	TRIANGLE_VT+1*4(A1),A4
		fmove.s	VTX_color_y(A4),fp5
		fmul	fp2,fp5
		move.l	TRIANGLE_VT+2*4(A1),A4
		fmove.s	VTX_color_y(A4),fp6
		fmul	fp0,fp6
		fadd	fp4,fp5
		
		move.l	TRIANGLE_VT+0*4(A1),A4
		fmove.s	VTX_color_x(A4),fp4
		fmul	fp1,fp4
		fadd	fp5,fp6
		move.l	TRIANGLE_VT+1*4(A1),A4
		fmove.s	VTX_color_x(A4),fp5
		fmul.l	#$FF00,fp6
		fmul	fp2,fp5
		move.l	TRIANGLE_VT+2*4(A1),A4
		fmove.l	fp6,(a3)
		fmove.s	VTX_color_x(A4),fp6
		fmul	fp0,fp6
		fadd	fp4,fp5
		
		move.l	TRIANGLE_VT+0*4(A1),A4
		fmove.s	VTX_color_z(A4),fp4
		fmul	fp1,fp4
		fadd	fp5,fp6
		move.l	TRIANGLE_VT+1*4(A1),A4
		fmove.s	VTX_color_z(A4),fp5
		fmul	fp2,fp5
		fmul.w	#$00ff,fp6
		move.l	TRIANGLE_VT+2*4(A1),A4
		fmove.w	fp6,(a3)
		fmove.s	VTX_color_z(A4),fp6
		fmul	fp0,fp6
		fadd	fp4,fp5
		fadd	fp5,fp6
		fmul.w	#255,fp6
		fmove.w	fp6,d4
		move.b	d4,3(a3)
	else
		moveq	#-1,d4
		move.l	d4,(a3)
	endc
.2
		fadd.s	d0,fp0
		fblt.b	.9
		fadd.s	d1,fp1
		fblt.b	.9
		fadd.s	d2,fp2
		fblt.b	.9
		fadd.s	d3,fp3
		addq.l	#4,a3
		bra		.1
		
.9
		fmovem	(sp)+,fp2-fp6
		movem.l	(sp)+,d2-d4/a2-a4
		rts
