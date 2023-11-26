/**
 * A very simple software rasterizer.
 * (c) L. Diener 2011
 * adapted to amigaOS by S.Devulder 2018
 */

#include "rasterizer.h"

#define SCREEN_X(p) (int)(((p)+1)*((scalar)(width/2)))
#define SCREEN_Y(p) (int)(((p)+1)*((scalar)(height/2)))

typedef struct {
	scalar x, y;
	scalar w;
	vec3   c;
} point;

typedef struct {
	unsigned short width;
	unsigned short height;
	
	scalar *zbuf; // actually 1/z buf
	colour *pbuf;
	
	point p[3];

	int xmin;
	int xmax;
	int ymin;
	int ymax;

	int monochrome;
	colour col;
	
	struct bounds {
		point min, max;
	} *bounds;
	
} tri;

int prepare(tri *t, triangle* modelTri) {
	int i, width = t->width, height = t->height;
	
	// project & find min/max
	t->xmin = width-1;
	t->ymin = height-1;
	t->xmax = 0;
	t->ymax = 0;
	
	for( i = 0; i < 3; i++ ) {
		int x = SCREEN_X( modelTri->vertices[i]->proj_point.x );
		int y = SCREEN_Y( modelTri->vertices[i]->proj_point.y );
		if(x<t->xmin) t->xmin = x;
		if(y<t->ymin) t->ymin = y;
		if(x>t->xmax) t->xmax = x;
		if(y>t->ymax) t->ymax = y;
		t->p[i].x = x;
		t->p[i].y = y;
		t->p[i].w = 1.0f / modelTri->vertices[i]->proj_point.z;
	}
	
	// reject if triange is out of screen
	if(t->ymin > t->ymax || t->xmin > t->xmax) return 0;

	// determine monochrome
	t->col = modelTri->vertices[0]->col;
	t->monochrome = t->col==modelTri->vertices[1]->col && 
				    t->col==modelTri->vertices[2]->col;

	// if not prepare colors
	if(!t->monochrome) {
		t->p[0].c.x = modelTri->vertices[0]->color.x*0xFF0000;
		t->p[1].c.x = modelTri->vertices[1]->color.x*0xFF0000;
		t->p[2].c.x = modelTri->vertices[2]->color.x*0xFF0000;

		t->p[0].c.y = modelTri->vertices[0]->color.y*0xFF00;
		t->p[1].c.y = modelTri->vertices[1]->color.y*0xFF00;
		t->p[2].c.y = modelTri->vertices[2]->color.y*0xFF00;
			
		t->p[0].c.z = modelTri->vertices[0]->color.z*0xFF;
		t->p[1].c.z = modelTri->vertices[1]->color.z*0xFF;
		t->p[2].c.z = modelTri->vertices[2]->color.z*0xFF;
	}
	
	// setup bounds info
	if(!t->bounds) {
		struct bounds *p; int l;
		
		t->bounds = p = malloc((l=t->height)*sizeof(*p));
		while(l--) {
			p->min.x = t->width;
			p->max.x = -1;
			++p;
		}
	}
	
	// all done
	return 1;
}

static void plot(tri *t, int y, scalar x, scalar w, scalar r, scalar g, scalar b) {
	if((unsigned)y < (unsigned)t->height) {
		struct bounds *p = &t->bounds[y];
		if(x<p->min.x) {
			p->min.x   = x;
			p->min.w   = w;
			if(!t->monochrome) {
			p->min.c.x = r;
			p->min.c.y = g;
			p->min.c.z = b;				
			}
		}
		if(x>p->max.x) {
			p->max.x   = x;
			p->max.w   = w;
			if(!t->monochrome) {
			p->max.c.x = r;
			p->max.c.y = g;
			p->max.c.z = b;				
			}
		}
		// t->pbuf[(t->height-1 - y)*t->width + x] = -1;
	}
}

static _REG void plot_line(_A0(tri *t), _D0(int i), _D1(int j)) {
	scalar iLen, x, w, r, g, b, dx, dw, dr, dg, db, len;
	int y = t->p[i].y, k;
	
	// horiz line
	if(y == t->p[j].y) {
		plot(t, y, t->p[i].x, t->p[i].w, t->p[i].c.x, t->p[i].c.y, t->p[i].c.z);
		plot(t, y, t->p[j].x, t->p[j].w, t->p[j].c.x, t->p[j].c.y, t->p[j].c.z);
		return;
	} 
	
	// order so that y[i] < y[j]
	if(y>t->p[j].y) {
		int k = j; j = i; i = k;
	}
	
	y   = t->p[i].y;
	len = t->p[j].y - y;
	iLen = 1.0f / len;
	k = 1+len;
	
	 x  = t->p[i].x;  r  = t->p[i].c.x;  g  = t->p[i].c.y;  b  = t->p[i].c.z;  w  = t->p[i].w;
	dx  = t->p[j].x; dr  = t->p[j].c.x; dg  = t->p[j].c.y; db  = t->p[j].c.z; dw  = t->p[j].w;
	dx -= x;         dr -= r;           dg -= g;           db -= b;           dw -= w;
	dx *= iLen;      dr *= iLen;        dg *= iLen;        db *= iLen;        dw *= iLen;
	
	while(1 + --k) {
		plot(t,y,x,w,r,g,b);
		x += dx; ++y; w += dw; r += dr; g += dg; b += db;	
	}
}

_REG void plot_triangle(_A0(tri *t))  {
	plot_line(t, 0, 1);
	plot_line(t, 1, 2);
	plot_line(t, 2, 0);
}

_REG void crop_triangle(_A0(tri *t))  {
	int y;
	for(y=t->ymin; y<=t->ymax; ++y) {
		if(t->bounds[y].min.x < 0) {
			scalar rat = 1.0/(t->bounds[y].max.x - t->bounds[y].min.x);
			rat *= 0 - t->bounds[y].min.x;
			t->bounds[y].min.x    = 0;
			t->bounds[y].min.w   += (t->bounds[y].max.w   - t->bounds[y].min.w)  *rat;
			if(!t->monochrome) {
			t->bounds[y].min.c.x += (t->bounds[y].max.c.x - t->bounds[y].min.c.x)*rat;
			t->bounds[y].min.c.y += (t->bounds[y].max.c.y - t->bounds[y].min.c.y)*rat;
			t->bounds[y].min.c.z += (t->bounds[y].max.c.z - t->bounds[y].min.c.z)*rat;			
			}
		}
		if(t->bounds[y].max.x >= t->width) {
			scalar rat = 1.0/(t->bounds[y].max.x - t->bounds[y].min.x);
			rat *= t->width-1 - t->bounds[y].max.x;
			t->bounds[y].max.x    = t->width-1;
			t->bounds[y].max.w   += (t->bounds[y].max.w   - t->bounds[y].min.w)  *rat;
			if(!t->monochrome) {
			t->bounds[y].max.c.x += (t->bounds[y].max.c.x - t->bounds[y].min.c.x)*rat;
			t->bounds[y].max.c.y += (t->bounds[y].max.c.y - t->bounds[y].min.c.y)*rat;
			t->bounds[y].max.c.z += (t->bounds[y].max.c.z - t->bounds[y].min.c.z)*rat;				
			}
		}
	}
}

static _REG void draw_horiz_mono(_A0(tri *t), _A1(struct bounds *p)) {
	int x = ceil(p->min.x);
	scalar *zb = t->zbuf + x;
	colour *pb = t->pbuf + x;
	scalar iLen = 1.0f/(p->max.x - p->min.x);
	scalar w, dw, f=x-p->min.x;
	int i = 1+floor(p->max.x) - x;

	// reset min/max
	p->max.x = -1;
	p->min.x = t->width;
	
	// prepare next line
	t->zbuf -= t->width;
	t->pbuf -= t->width;
	
	 w  = p->min.w;
	
	if(i<=1) {
		if(w > *zb) {
			*zb = w;
			*pb = t->col;
		}
	} else {
		dw  = p->max.w;
		dw -= w;
		dw *= iLen;	
		 // w += dw*f;
		while(1 + --i) {
			if(w > *zb++) {
				zb[-1] = w;
				*pb = t->col;
			}
			w += dw; ++pb;
		}		
	}	
}

static _REG void draw_horiz(_A0(tri *t), _A1(struct bounds *p)) {
	int x = ceil(p->min.x);
	scalar *zb = t->zbuf + x;
	colour *pb = t->pbuf + x;
	scalar iLen = 1.0f/(p->max.x - p->min.x);
	scalar r, g, b, w, dr, dg, db, dw, f=x-p->min.x;
	int i = 1+floor(p->max.x) - x;

    r  = p->min.c.x;  g  = p->min.c.y;  b  = p->min.c.z;  w  = p->min.w;
	
	// reset min/max
	p->max.x = -1;
	p->min.x = t->width;
	
	// prepare next line
	t->zbuf -= t->width;
	t->pbuf -= t->width;
	
	if(i<=1) {
		// dw  = p->max.w;
		// dw -= w;
		// dw *= iLen;	
		 // w += dw*f;	 
		if(w > *zb) {
			int t;
			*zb = w;
			t = r; ((long*)pb) [0] = t;
			t = g; ((short*)pb)[1] = t;
			t = b; ((char*)pb) [3] = t;
		}
	} else {
		dr  = p->max.c.x; dg  = p->max.c.y; db  = p->max.c.z; dw  = p->max.w;
		dr -= r;          dg -= g;          db -= b;          dw -= w;
		dr *= iLen;       dg *= iLen;       db *= iLen;       dw *= iLen;		

		r += dr*f;
		g += dg*f;
		b += db*f;
		//w += dw*f;

		// draw
		while(1 + --i) {
			if(w > *zb++) {
				int t;
				zb[-1] = w;
				t = r; ((long*)pb) [0] = t;
				t = g; ((short*)pb)[1] = t;
				t = b; ((char*)pb) [3] = t;
			}
			r += dr; g += dg; b += db;	w += dw; ++pb;
		}		
	}	
}

void _REG draw_triangle(_A0(tri *t)) {
	struct bounds *b = &t->bounds[t->ymin];
	int idx = (unsigned short)(t->height-1 - t->ymin) * t->width;
	scalar *zb = t->zbuf;
	colour *pb = t->pbuf;
	int l = 1 + t->ymax - t->ymin;
	
	t->zbuf += idx;
	t->pbuf += idx;
	
	plot_triangle(t); 
	// crop_triangle(t);

	if(t->monochrome) {
		while(1 + --l) {
			draw_horiz_mono(t, b);
			++b;
		}				
	} else {
		while(1 + --l) {
			draw_horiz(t, b);
			++b;
		}		
	}
	
	t->zbuf = zb;
	t->pbuf = pb;
}

void rasterize(model* m, buffer* pbuf, scalar* zbuf) {
	static tri t;
	
	// local copy for faster access
	t.width  = pbuf->width;
	t.height = pbuf->size;
	t.pbuf   = pbuf->data;
	t.zbuf   = zbuf;

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
		) continue;
	
		if(prepare(&t, modelTri))
			draw_triangle(&t);
	}
}
