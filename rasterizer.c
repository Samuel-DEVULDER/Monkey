/**
 * A very simple software rasterizer.
 * (c) L. Diener 2011
 * adapted to amigaOS by S.Devulder 2018
 */

#include "rasterizer.h"

#define SCREEN_X(p) (((p)+1)*((scalar)(width/2)))
#define SCREEN_Y(p) (((p)+1)*((scalar)(height/2)))

#if 1
#define real scalar
#define real2scalar(x) (x)
#define scalar2real(x) (x)
#define some_neg(x,y,z) ((x)<0 || (y)<0 || (z)<0)
#define dx nx
#else
#define base (1<<16)
#define real int
#define real2scalar(x) ((scalar)((x)*(1.0/base)))
#define scalar2real(x) ((real)((x)*base))
#define some_neg(x,y,z) (((x)|(y)|(z))<0)
#endif

typedef struct {
	scalar sx[3];
	scalar sy[3];
	scalar nx[3];
	scalar ny[3];
//	scalar b[3];
	scalar c[3];
	int xmin;
	int xmax;
	int ymin;
	int ymax;
//
	struct {
		unsigned short width;
		unsigned short height;
		scalar  *zbuf;
		colour *pbuf;
		
		int *xmin;
		int *xmax;
	} extra;
//	
	scalar dz;
	scalar *zb;
	colour *out;
	real   d[3];
#ifndef dx
	real   dx[3];
#endif
	int x;
} tri;

void project_onto_screen(tri *t, triangle* modelTri) {
	int width  = t->extra.width-1;
	int height = t->extra.height-1;
	int i;
	
	// Lines of the form: d = nx * ( x - sx ) + ny * ( y - sy )
	for( i = 0; i < 3; i++ ) {
		t->sx[i] = SCREEN_X( modelTri->vertices[i]->proj_point.x );
		t->sy[i] = SCREEN_Y( modelTri->vertices[i]->proj_point.y );
	}

	// Bounding rectangles.
	t->xmin = floor( scalarMin( scalarMin( t->sx[0], t->sx[1] ), t->sx[2] ) );
	t->xmax =  ceil( scalarMax( scalarMax( t->sx[0], t->sx[1] ), t->sx[2] ) );
	t->ymin = floor( scalarMin( scalarMin( t->sy[0], t->sy[1] ), t->sy[2] ) );
	t->ymax  = ceil( scalarMax( scalarMax( t->sy[0], t->sy[1] ), t->sy[2] ) );

	// Clip.
	t->xmin = scalarMax(0, t->xmin);
	t->ymin = scalarMax(0, t->ymin);
	t->xmax = scalarMin(width, t->xmax);
	t->ymax = scalarMin(height, t->ymax);

	// normal weights (reciprocals)
	t->c[0] = 1.0f / modelTri->vertices[2]->proj_point.z;
	t->c[1] = 1.0f / modelTri->vertices[0]->proj_point.z;
	t->c[2] = 1.0f / modelTri->vertices[1]->proj_point.z;
}

static inline void compute_normals(tri *t) {
	int i;

	// Normals
	for( i = 0; i < 3; i++ ) {
		scalar x = -(t->sy[(i+1)%3] - t->sy[i]);
		scalar y =  (t->sx[(i+1)%3] - t->sx[i]);
		scalar l = scalarInvSqrt( x*x + y*y );
		t->nx[i] = x*l;
		t->ny[i] = y*l;
	}
}

void compute_inverse_barycentric_coordinates(tri *t) {
	scalar b0, b1, b2;
	
	// For barycentric coordinates
	b0 = t->nx[0] * ( t->sx[2] - t->sx[0] ) + t->ny[0] * ( t->sy[2] - t->sy[0] );
	b1 = t->nx[1] * ( t->sx[0] - t->sx[1] ) + t->ny[1] * ( t->sy[0] - t->sy[1] );
	b2 = t->nx[2] * ( t->sx[1] - t->sx[2] ) + t->ny[2] * ( t->sy[1] - t->sy[2] );
	
	//b0 = scalarAbs(b0);
	//b1 = scalarAbs(b1);
	//b2 = scalarAbs(b2);
	
	b0 = 1.0f/b0;
	b1 = 1.0f/b1;
	b2 = 1.0f/b2;

	// prescale
	t->nx[0] *= b0; t->ny[0] *= b0;
	t->nx[1] *= b1; t->ny[1] *= b1;
	t->nx[2] *= b2; t->ny[2] *= b2;
#ifndef	dx
	t->dx[0] = scalar2real(t->nx[0]);
	t->dx[1] = scalar2real(t->nx[1]);
	t->dx[2] = scalar2real(t->nx[2]);
#endif
}

void draw_triangle(tri *t, triangle *modelTri) {
	// update ptrs
	int idx      = t->xmin + (unsigned short)(t->extra.height - t->ymin) * t->extra.width;
	scalar  *zb_  = &t->extra.zbuf[idx];
	colour *out_ = &t->extra.pbuf[idx];

	// values at the top left corner of the bounding box
	scalar d1_ = t->nx[0] * ( t->xmin - t->sx[0] ) + t->ny[0] * ( t->ymin - t->sy[0] ); 
	scalar d2_ = t->nx[1] * ( t->xmin - t->sx[1] ) + t->ny[1] * ( t->ymin - t->sy[1] );
	scalar d3_ = t->nx[2] * ( t->xmin - t->sx[2] ) + t->ny[2] * ( t->ymin - t->sy[2] );

	// Draw pixels inside, if need be
	short j = t->ymax - t->ymin;
	while(1 + --j) {
		short i = t->xmax - t->xmin;
		colour *out = out_;
		scalar *zb = zb_, d1 = d1_, d2 = d2_, d3 = d3_;
		scalar z, dz;

		// prepare next row
		d1_  += t->ny[0];   
		d2_  += t->ny[1]; 
        d3_  += t->ny[2]; 
		zb_  -= t->extra.width; 
		out_ -= t->extra.width;			

		// advance until inside triangle
#if 0
		if(!(d1>=0 && d2>=0 && d3>=0)) {
			unsigned int j = 0, k;
			k = -d1 / t->nx[0];
			if(d1<0 && k>j) j = k;
			k = -d2 * t->nx[1];
			if(d2<0 && k>j) j = k;
			k = -d3 * t->nx[2];
			if(d3<0 && k>j) j = k;
			d1 += j * t->nx[0];
			d2 += j * t->nx[1];
			d3 += j * t->nx[2];
			i -= j;
			if(i<=0) continue;
			zb += j;
			out += j;
		}
#endif
		while(!(d1>=0 && d2>=0 && d3>=0) && (1 + --i)) {
			d1 += t->nx[0]; ++zb;
			d2 += t->nx[1]; ++out;
			d3 += t->nx[2]; 
		}
		
		// no real intersection
		if(i<=0) continue;

		// distance of the pixel to the camera
		z  = t->c[1] * d2 + t->c[2] * d3 + t->c[0] * d1;
			
		// increment in the dx direction
		dz = t->c[1]*t->nx[1] + t->c[2]*t->nx[2] + t->c[0]*t->nx[0];

		while(1 + --i) {
			// Z test
			if( z > *zb++ ) {
				scalar r,g,b;
				zb[-1] = z;

				r =
					modelTri->vertices[0]->color.x * d2 +
					modelTri->vertices[1]->color.x * d3 +
					modelTri->vertices[2]->color.x * d1;
				g =
					modelTri->vertices[0]->color.y * d2  +
					modelTri->vertices[1]->color.y * d3  +
					modelTri->vertices[2]->color.y * d1;
				b =
					modelTri->vertices[0]->color.z * d2 +
					modelTri->vertices[1]->color.z * d3 +
					modelTri->vertices[2]->color.z * d1;

				*out = makeColour(r,g,b);
			}
			++out; z += dz;
	
			// update with delta in dx direction
			d1 += t->nx[0]; if(d1<0) break;
			d2 += t->nx[1]; if(d2<0) break;
			d3 += t->nx[2]; if(d3<0) break;
		}
	}
}

#if 0
static _REG void plot(_A0(tri *t), _D0(int x), _D1(int y)) {
	if((unsigned)x>=(unsigned)t->extra.width) return;
	--y;
	if((unsigned)y<(unsigned)t->extra.height) {
		if(x<t->extra.xmin[y]) t->extra.xmin[y]=x-1;
		if(x>t->extra.xmax[y]) t->extra.xmax[y]=x+1;	
		++y;
		if((unsigned)y<(unsigned)t->extra.height) {
			if(x<t->extra.xmin[y]) t->extra.xmin[y]=x-1;
			if(x>t->extra.xmax[y]) t->extra.xmax[y]=x+1;	
			++y;
			if((unsigned)y<(unsigned)t->extra.height) {
				if(x<t->extra.xmin[y]) t->extra.xmin[y]=x-1;
				if(x>t->extra.xmax[y]) t->extra.xmax[y]=x+1;	
			}
		}
	}
	//t->extra.pbuf[x + t->extra.width*(t->extra.height-y)] = -1;
}
#else
static inline _REG void plot(_A0(tri *t), _D0(int x), _D1(int y)) {
	//if((unsigned)x>=(unsigned)t->extra.width)  return;
	if((unsigned)y>=(unsigned)t->extra.height) return;
	if(x<t->extra.xmin[y]) t->extra.xmin[y]=x;
	//if(x>t->extra.xmax[y]) t->extra.xmax[y]=x+1;	
}
#endif

static inline void plotLineLow(tri *t, int x0, int y0, int x1, int y1) {
	int dx = x1 - x0, dy = y1 - y0, yi = 1, d, x, y;
	if(dy<0) {dy = -dy; yi = -1;}
	dy += dy; d = dy - dx; y = y0; dx += dx; 
	for(x=x0; x<=x1; ++x) {
		plot(t, x, y);
		if(d>0) {
			y += yi;
			d -= dx;
		}
		d += dy;
	}
}

static inline void plotLineHigh(tri *t, int x0, int y0, int x1, int y1) {
	int dx = x1 - x0, dy = y1 - y0, xi = 1, d, x, y;
	if(dx<0) {dx = -dx; xi = -1;}
	dx += dx; d = dx - dy; x = x0; dy += dy; 
	for(y=y0; y<=y1; ++y) {
		plot(t, x, y);
		if(d>0) {
			x += xi;
			d -= dy;
		}
		d += dx;
	}
}

static void plotLine(tri *t, int x0, int y0, int x1, int y1) {
	int dx = x1 - x0, dy = y1 - y0;
	if(dx<0) dx = -dx; if(dy<0) dy = -dy;
	if(dy<dx) {
		if(x0>x1) 	plotLineLow(t, x1, y1, x0, y0);
		else		plotLineLow(t, x0, y0, x1, y1);
	} else {
		if(y0>y1) 	plotLineHigh(t, x1, y1, x0, y0);
		else		plotLineHigh(t, x0, y0, x1, y1);		
	}
}

_REG void plotTriangle(_A0(tri *t))  {
	const scalar r = 0.5;
	plotLine(t, t->sx[0]+r, t->sy[0]+r, t->sx[1]+r, t->sy[1]+r);
	plotLine(t, t->sx[0]+r, t->sy[0]+r, t->sx[2]+r, t->sy[2]+r);
	plotLine(t, t->sx[2]+r, t->sy[2]+r, t->sx[1]+r, t->sy[1]+r);
}

static _REG void draw_triangle_2(_A0(tri *t), _A1(triangle *modelTri)) {
	int y = t->ymin;
	int idx = (unsigned short)(t->extra.height-1 - y) * t->extra.width;
	scalar *zb_  = &t->extra.zbuf[idx];
	colour *out_ = &t->extra.pbuf[idx];

	plotTriangle(t);
		
	for(; y<=t->ymax; ++y, zb_-=t->extra.width, out_-=t->extra.width) {
		int xmin = t->extra.xmin[y];
		int xmax = t->extra.xmax[y];
		int x;
		
		t->extra.xmin[y] = t->extra.width;
		t->extra.xmax[y] = 0;
		
		if(xmax>t->xmax) xmax = t->xmax;
		if(xmin<t->xmin) xmin = t->xmin;
		if(xmax<xmin) continue;
		
		for(x=xmin; x<=xmax; ++x) {
			scalar d1 =  t->nx[0] * ( x - t->sx[0] ) + t->ny[0] * ( y - t->sy[0] );
			scalar d2 =  t->nx[1] * ( x - t->sx[1] ) + t->ny[1] * ( y - t->sy[1] );
			scalar d3 =  t->nx[2] * ( x - t->sx[2] ) + t->ny[2] * ( y - t->sy[2] );
			
			if(d1>=0 && d2>=0 && d3>=0) {
				scalar  z = t->c[1] * d2 + t->c[2] * d3 + t->c[0] * d1, r,g,b;
				if(z> zb_[x]) {
					zb_[x] = z;
					r =
					modelTri->vertices[0]->color.x * d2 +
					modelTri->vertices[1]->color.x * d3 +
					modelTri->vertices[2]->color.x * d1;
					g =
					modelTri->vertices[0]->color.y * d2  +
					modelTri->vertices[1]->color.y * d3  +
					modelTri->vertices[2]->color.y * d1;
					b =
					modelTri->vertices[0]->color.z * d2 +
					modelTri->vertices[1]->color.z * d3 +
					modelTri->vertices[2]->color.z * d1;
					
					//r=1; g=x==xmin; b=x==xmax;
					
					out_[x] = makeColour(r,g,b);
				}
			}
		}
	}
}

static _REG void draw_triangle_3(_A0(tri *t), _A1(triangle *modelTri)) {
	int y = t->ymin-1, ymax = t->ymax+1, idx;
	scalar *zb_, dz, d1_, d2_, d3_;
	colour *out_, col = modelTri->vertices[0]->col;
	int monochrome = col==modelTri->vertices[1]->col && col==modelTri->vertices[2]->col;
	//if(monochrome) col = ~col;
	plotTriangle(t);
	if(y<0) y=0; if(ymax>=t->extra.height) ymax=t->extra.height-1;
	
	idx  = (unsigned short)(t->extra.height-1 - y) * t->extra.width;
	zb_  = &t->extra.zbuf[idx];
	out_ = &t->extra.pbuf[idx];
	d1_  = t->nx[0] * ( 0 - t->sx[0] ) + t->ny[0] * ( y - t->sy[0] ); 
	d2_  = t->nx[1] * ( 0 - t->sx[1] ) + t->ny[1] * ( y - t->sy[1] );
	d3_  = t->nx[2] * ( 0 - t->sx[2] ) + t->ny[2] * ( y - t->sy[2] );
	dz   = t->c[1]*t->nx[1] + t->c[2]*t->nx[2] + t->c[0]*t->nx[0];
	
	for(;y<=ymax; ++y) {
		int xmin = t->extra.xmin[y];
		int xmax = t->extra.xmax[y];
		scalar *zb, z, d1, d2,d3;
		colour *out;
		short   i;
		
		t->extra.xmin[y] = t->extra.width;
		t->extra.xmax[y] = 0;
		
		if(xmax>t->xmax) xmax = t->xmax;
		if(xmin<t->xmin) xmin = t->xmin;
		if(xmax<xmin) goto next;
		
		out = out_ + xmin;
		zb  = zb_  + xmin;
		d1  = d1_  + xmin*t->nx[0];
		d2  = d2_  + xmin*t->nx[1];
		d3  = d3_  + xmin*t->nx[2];		
		z = t->c[1]* d2 + t->c[2] * d3 + t->c[0] * d1;
		i = 1 + xmax - xmin;

#if 0
		if(monochrome) {
			while(1 + --i) {
				// Z test
				if(d1>=0 && d2>=0 && d3>=0 && z > *zb ) {
					*zb = z;
					*out = col;
				}
				++out; z += dz; ++zb;
		
				// update with delta in dx direction
				d1 += t->nx[0];
				d2 += t->nx[1];
				d3 += t->nx[2];
			}
			goto next;
		}

		while(1 + --i) {
			// Z test
			if(d1>=0 && d2>=0 && d3>=0 && z > *zb ) {
				scalar r,g,b;
				*zb = z;

				r =
					modelTri->vertices[0]->color.x * d2 +
					modelTri->vertices[1]->color.x * d3 +
					modelTri->vertices[2]->color.x * d1;
				g =
					modelTri->vertices[0]->color.y * d2  +
					modelTri->vertices[1]->color.y * d3  +
					modelTri->vertices[2]->color.y * d1;
				b =
					modelTri->vertices[0]->color.z * d2 +
					modelTri->vertices[1]->color.z * d3 +
					modelTri->vertices[2]->color.z * d1;

				*out = makeColour(r,g,b);
			}
			++out; z += dz; ++zb;
	
			// update with delta in dx direction
			d1 += t->nx[0];
			d2 += t->nx[1];
			d3 += t->nx[2];
		}
#else
		while(!(d1>=0 && d2>=0 && d3>=0) && (1 + --i)) {
			++out; z += dz; ++zb;
			d1 += t->nx[0];
			d2 += t->nx[1];
			d3 += t->nx[2];
		}

		// no real intersection
		if(i<0) goto next;

		if(monochrome) {
			while(1 + --i) {
				if(z > *zb++ ) {
					zb[-1] = z;
					*out = col;
				}
				++out; z += dz;
			}
			goto next;
		}
	
#if 0
		// tests
		while(1 + --i) {
			if(z > *zb++ ) {
				zb[-1] = z;
				*out = (modelTri->ID&255)*0x010101;
			}
			++out; z += dz;
		}
		goto next;
#endif
		
		while(1 + --i) {
			// Z test
			if( z > *zb++ ) {
				scalar r,g,b;
				zb[-1] = z;

				r =
					modelTri->vertices[0]->color.x * d2 +
					modelTri->vertices[1]->color.x * d3 +
					modelTri->vertices[2]->color.x * d1;
				g =
					modelTri->vertices[0]->color.y * d2  +
					modelTri->vertices[1]->color.y * d3  +
					modelTri->vertices[2]->color.y * d1;
				b =
					modelTri->vertices[0]->color.z * d2 +
					modelTri->vertices[1]->color.z * d3 +
					modelTri->vertices[2]->color.z * d1;

				*out = makeColour(r,g,b);
			}
			++out; z += dz;
	
			// update with delta in dx direction
			d1 += t->nx[0]; if(d1<0) break;
			d2 += t->nx[1]; if(d2<0) break;
			d3 += t->nx[2]; if(d3<0) break;
		}
#endif
next:
		zb_  -= t->extra.width; 
		out_ -= t->extra.width;		
		d1_  += t->ny[0];   
		d2_  += t->ny[1]; 
        d3_  += t->ny[2]; 
	}
}

static _REG void draw_triangle_4(_A0(tri *t), _A1(triangle *modelTri)) {
	int y = t->ymin-1, ymax = t->ymax+1, idx;
	scalar *zb_, dz, d1_, d2_, d3_;
	colour *out_, col = modelTri->vertices[0]->col;
	int monochrome = col==modelTri->vertices[1]->col && col==modelTri->vertices[2]->col;
	//if(monochrome) col = ~col;
	
	plotTriangle(t);
	
	if(y<0) y=0; if(ymax>=t->extra.height) ymax=t->extra.height-1;
	
	idx  = (unsigned short)(t->extra.height-1 - y) * t->extra.width;
	zb_  = &t->extra.zbuf[idx];
	out_ = &t->extra.pbuf[idx];
	d1_  = t->nx[0] * ( 0 - t->sx[0] ) + t->ny[0] * ( y - t->sy[0] ); 
	d2_  = t->nx[1] * ( 0 - t->sx[1] ) + t->ny[1] * ( y - t->sy[1] );
	d3_  = t->nx[2] * ( 0 - t->sx[2] ) + t->ny[2] * ( y - t->sy[2] );
	dz   = t->c[1]*t->nx[1] + t->c[2]*t->nx[2] + t->c[0]*t->nx[0];
	
	for(;y<=ymax; ++y) {
		int xmin = t->extra.xmin[y];
		int xmax = t->extra.xmax[y];
		scalar *zb, z, d1,d2,d3;
		scalar *_zb, _z, _d1,_d2,_d3;
		colour *out, *_out;
		int x, _x;
		
		t->extra.xmin[y] = t->extra.width;
		t->extra.xmax[y] = 0;
		
		if(xmax>t->xmax) xmax = t->xmax;
		if(xmin<t->xmin) xmin = t->xmin;
		if(xmax<xmin) goto next;
		
		x   = (xmin+xmax)>>1;
		out = out_ + x;
		zb  = zb_  + x;
		d1  = d1_ + x*t->nx[0];
		d2  = d2_ + x*t->nx[1];
		d3  = d3_ + x*t->nx[2];		
		z   = t->c[1]* d2 + t->c[2] * d3 + t->c[0] * d1;
		// if(d1<0) d1=0;
		// if(d2<0) d2=0;
		// if(d3<0) d3=0;
		
		if(d1<0 || d2<0 || d3<0) {
			x   = t->xmin;
			out = out_ + x;
			zb  = zb_  + x;
			d1  = d1_ + x*t->nx[0];
			d2  = d2_ + x*t->nx[1];
			d3  = d3_ + x*t->nx[2];		
			z   = t->c[1]* d2 + t->c[2] * d3 + t->c[0] * d1;
			
			while((d1<0 || d2<0 || d3<0) && x<=t->xmax) {
				++x;
				++out;
				++zb;
				d1 += t->nx[0];
				d2 += t->nx[1];
				d3 += t->nx[2];
				z  += dz;
			}
			if(x>t->xmax) goto next; // no real intersection
		}

		
		_x   = x;
		_out = out;
		_zb  = zb;
		_d1  = d1;
		_d2  = d2;
		_d3  = d3;
		_z   = z;
		
		// go forward
		for(;x<=t->xmax;++x) {
		//for(;;) {
			if(z > *zb++ ) {
				zb[-1] = z;
				if(!monochrome)	col = makeColour(
					modelTri->vertices[0]->color.x * d2 +
					modelTri->vertices[1]->color.x * d3 +
					modelTri->vertices[2]->color.x * d1,
						
					modelTri->vertices[0]->color.y * d2  +
					modelTri->vertices[1]->color.y * d3  +
					modelTri->vertices[2]->color.y * d1,
						
					modelTri->vertices[0]->color.z * d2 +
					modelTri->vertices[1]->color.z * d3 +
					modelTri->vertices[2]->color.z * d1);
				*out = col;
			}
			++out; z += dz;
			// update with delta in dx direction
			d1 += t->nx[0]; if(d1<0) break;
			d2 += t->nx[1]; if(d2<0) break;
			d3 += t->nx[2]; if(d3<0) break;
		}

		x   = _x;
		out = _out;
		zb  = _zb;
		d1  = _d1;
		d2  = _d2;
		d3  = _d3;
		z   = _z;
		
		// go backward
		for(;--x>=t->xmin;) {
		//for(;;) {
			// update with delta in dx direction
			d1 -= t->nx[0]; if(d1<0) break;
			d2 -= t->nx[1]; if(d2<0) break;
			d3 -= t->nx[2]; if(d3<0) break;
			--out; z -= dz;
			if(z > *--zb ) {
				zb[1] = z;
				if(!monochrome) col = makeColour(
					modelTri->vertices[0]->color.x * d2 +
					modelTri->vertices[1]->color.x * d3 +
					modelTri->vertices[2]->color.x * d1,
					
					modelTri->vertices[0]->color.y * d2  +
					modelTri->vertices[1]->color.y * d3  +
					modelTri->vertices[2]->color.y * d1,
					
					modelTri->vertices[0]->color.z * d2 +
					modelTri->vertices[1]->color.z * d3 +
					modelTri->vertices[2]->color.z * d1);
				*out = col;
			}
		}
		
next:
		zb_  -= t->extra.width; 
		out_ -= t->extra.width;		
		d1_  += t->ny[0];   
		d2_  += t->ny[1]; 
        d3_  += t->ny[2]; 
	}
}

int _REG find_span(_A0(tri *t)) {
	real d0=t->d[0], d1=t->d[1], d2=t->d[2];
	int x = 0, xmax=t->xmax - t->x;
	while(some_neg(d0,d1,d2)) {
		if(++x>xmax) return 0;
		d0 += t->dx[0];
		d1 += t->dx[1];
		d2 += t->dx[2];
	}
	t->d[0]=d0; t->d[1]=d1; t->d[2]=d2;
	t->zb += x; t->out += x; t->x += x;
	return 1;
}

void _REG draw_span_mono(_A0(tri *t), _D0(colour col)) {
	scalar z = real2scalar(t->c[1]*t->d[1] + t->c[2]*t->d[2] + t->c[0]*t->d[0]), *zb = t->zb;
	real d0=t->d[0], d1=t->d[1], d2=t->d[2];
	colour *out=t->out;
//	int x=t->x;
//	do {
	for(;;) {
		if(z > *zb++ ) {
			zb[-1] = z;
			*out = col;
		}
		// update with delta in dx direction
		if((d0 += t->dx[0])<0) break;
		if((d1 += t->dx[1])<0) break;
		if((d2 += t->dx[2])<0) break;
		z += t->dz; ++out;
	}
//	} while(++x<=t->xmax);
}

void _REG draw_span(_A0(tri *t), _A1(triangle *modelTri)) {
	scalar z = real2scalar(t->c[0]*t->d[0] + t->c[1]*t->d[1] + t->c[2]*t->d[2]), *zb = t->zb;
	colour *out=t->out;
	real d0=t->d[0], d1=t->d[1], d2=t->d[2];
//	int x=t->x;
//	do {
	for(;;) {
		if(z > *zb++ ) {
			zb[-1] = z;
			*out = makeColour(
					real2scalar(
					modelTri->vertices[0]->color.x * d1 +
					modelTri->vertices[1]->color.x * d2 +
					modelTri->vertices[2]->color.x * d0),
					real2scalar(	
					modelTri->vertices[0]->color.y * d1  +
					modelTri->vertices[1]->color.y * d2  +
					modelTri->vertices[2]->color.y * d0),
					real2scalar(
					modelTri->vertices[0]->color.z * d1 +
					modelTri->vertices[1]->color.z * d2 +
					modelTri->vertices[2]->color.z * d0));
		}
		// update with delta in dx direction
		if((d0 += t->dx[0])<0) break;
		if((d1 += t->dx[1])<0) break;
		if((d2 += t->dx[2])<0) break;
		z += t->dz;	++out; 
	}
//	} while(++x<=t->xmax);
}

void _REG draw_triangle_5(_A0(tri *t), _A1(triangle *modelTri)) {
	int y = t->ymin, idx;
	scalar *zb_, d1_, d2_, d3_;
	colour *out_; 
	colour col = modelTri->vertices[0]->col;
	int monochrome = col==modelTri->vertices[1]->col && col==modelTri->vertices[2]->col;
	//if(monochrome) col = ~col;
	
	if(t->ymax - t->ymin<=4) {
		int i = t->ymin;
		while(i<=t->ymax) t->extra.xmin[i++] = t->xmin;
	} else plotTriangle(t);
	
	idx   = (unsigned short)(t->extra.height-1 - y) * t->extra.width;
	zb_   = &t->extra.zbuf[idx];
	out_  = &t->extra.pbuf[idx];
	d1_   = t->nx[0] * ( 0 - t->sx[0] ) + t->ny[0] * ( y - t->sy[0] ); 
	d2_   = t->nx[1] * ( 0 - t->sx[1] ) + t->ny[1] * ( y - t->sy[1] );
	d3_   = t->nx[2] * ( 0 - t->sx[2] ) + t->ny[2] * ( y - t->sy[2] );
	t->dz = t->c[0]*t->nx[0] + t->c[1]*t->nx[1] + t->c[2]*t->nx[2];
	
	for(;y<=t->ymax; ++y) {
		int x = t->extra.xmin[y]-1;
		
		t->extra.xmin[y] = t->extra.width;
		if(x<t->xmin) x = t->xmin;
		
		t->x    = x;
		t->out  = out_ + x;			out_ -= t->extra.width;
		t->zb   = zb_  + x; 		zb_  -= t->extra.width; 
		t->d[0] = scalar2real(d1_ + x*t->nx[0]); d1_  += t->ny[0];
		t->d[1] = scalar2real(d2_ + x*t->nx[1]); d2_  += t->ny[1];
		t->d[2] = scalar2real(d3_ + x*t->nx[2]); d3_  += t->ny[2];
		
		if(find_span(t)) {
			if(monochrome)	
				draw_span_mono(t,col);
			else 
				draw_span(t,modelTri);
		}
	}
}

void rasterize(model* m, buffer* pbuf, scalar* zbuf) {
	static tri t;
	
	// local copy for faster access
	t.extra.width  = pbuf->width;
	t.extra.height = pbuf->size;
	t.extra.pbuf   = pbuf->data;
	t.extra.zbuf   = zbuf;

	if(!t.extra.xmin) {
		int *p, l;
		
		t.extra.xmin = p = malloc((l=t.extra.height)*sizeof(*p));
		while(l--) *p++ = t.extra.width;
		
//		t.extra.xmax = p = malloc((l=t.extra.height)*sizeof(*p));
//		while(l--) *p++ = 0;
	}
	
	// The actual rasterizer
	while(modelTrianglesLeft(m)) {
		triangle *modelTri = modelNextTriangle(m);
		
		// Backface cull
		if(
			(modelTri->vertices[1]->proj_point.x - modelTri->vertices[0]->proj_point.x) *
			(modelTri->vertices[2]->proj_point.y - modelTri->vertices[0]->proj_point.y) -
			(modelTri->vertices[2]->proj_point.x - modelTri->vertices[0]->proj_point.x) *
			(modelTri->vertices[1]->proj_point.y - modelTri->vertices[0]->proj_point.y)
			< 0
		) {
			continue;
		}
	
		project_onto_screen(&t, modelTri);
		
		// reject if triange is out of screen
		if(t.ymin >= t.ymax || t.xmin >= t.xmax) continue;

		compute_normals(&t);
		compute_inverse_barycentric_coordinates(&t);	
//		draw_triangle(&t, modelTri);
//		draw_triangle_2(&t, modelTri);
//		draw_triangle_3(&t, modelTri);
//		draw_triangle_4(&t, modelTri);
		draw_triangle_5(&t, modelTri);
	}
}
