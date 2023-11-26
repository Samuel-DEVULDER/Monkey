/**
 * A very simple software rasterizer.
 * (c) L. Diener 2011
 * adapted to amigaOS by S.Devulder 2018
 */

#include "rasterizer.h"

#define SCREEN_X(p) (((p)+1)*((scalar)(width/2)))
#define SCREEN_Y(p) (((p)+1)*((scalar)(height/2)))

typedef struct {
	unsigned short width;
	unsigned short height;
	
	scalar *zbuf; // actually 1/z buf
	colour *pbuf;
	
	int     x[3];	// screen pos: x
	int     y[3];	//             y
	scalar	w[3];	// inverse dist to screen	
	scalar  r[3];	// color: red
	scalar  g[3];	//        green
	scalar  b[3];   //        blue
	
	// barycentric coordinates (implicit d[2]=1-d[0]-d[1])
	scalar	d[2];
	
	// gradients
	scalar	dx[2];
	scalar	dy[2];
	scalar	wx;
	scalar  rx;
	scalar  gx;
	scalar  bx;
	
	colour col;
	int    monochrome;
	
	int xmin;
	int xmax;
	int ymin;
	int ymax;

	struct bounds {
		scalar min, max;
	} *bounds;
} tri;

static _REG int projection(_A0(tri *t), _A1(triangle* modelTri)) {
	int i, width = t->width, height = t->height;
	
	// project & find min/max
	t->ymin = height-1;
	t->ymax = 0;
	t->xmin = width-1;
	t->xmax = 0;
	
	for( i = 0; i < 3; i++ ) {
		int x = SCREEN_X( modelTri->vertices[i]->proj_point.x );
		int y = SCREEN_Y( modelTri->vertices[i]->proj_point.y ); 
		t->x[i] = x;
		t->y[i] = y;
		t->w[i] = 1.0f / modelTri->vertices[i]->proj_point.z;
		if(x<t->xmin) t->xmin = x;
		if(x>t->xmax) t->xmax = x;
		if(y<t->ymin) t->ymin = y;
		if(y>t->ymax) t->ymax = y;
	}
	
	// reject if triange is out of screen
	if(t->ymin > t->ymax || t->xmin > t->xmax)
		return 0;
	return 1;
}

static _REG int gradient(_A0(tri *t)) {
	scalar det, a, b, c, d;
	a = t->y[1]-t->y[2]; b = t->x[2] - t->x[1];
	c = t->y[2]-t->y[0]; d = t->x[0] - t->x[2];
	
	det = a*d - b*c;
	if(det==0) return 0; else det = 1/det;
	
	a *= det; b *= det; c *= det; d *= det;
	
	t->dx[0] = a; t->dy[0] = b;
	t->dx[1] = c; t->dy[1] = d;
	
	t->wx   = a*(t->w[0]-t->w[2]) + c*(t->w[1]-t->w[2]);

	t->d[0] = a*(0-t->x[2]) + b*(t->ymin-t->y[2]);
	t->d[1] = c*(0-t->x[2]) + d*(t->ymin-t->y[2]);
	
	return 1;
}

static _REG void color(_A0(tri *t), _A1(triangle* modelTri)) {
	// determine monochrome
	t->col = modelTri->vertices[0]->col;
	t->monochrome = t->col==modelTri->vertices[1]->col && 
				    t->col==modelTri->vertices[2]->col;

	// if not prepare colors
	if(!t->monochrome) {
		t->r[0] = modelTri->vertices[0]->color.x*0xFF0000;
		t->r[1] = modelTri->vertices[1]->color.x*0xFF0000;
		t->r[2] = modelTri->vertices[2]->color.x*0xFF0000;

		t->g[0] = modelTri->vertices[0]->color.y*0xFF00;
		t->g[1] = modelTri->vertices[1]->color.y*0xFF00;
		t->g[2] = modelTri->vertices[2]->color.y*0xFF00;
			
		t->b[0] = modelTri->vertices[0]->color.z*0xFF;
		t->b[1] = modelTri->vertices[1]->color.z*0xFF;
		t->b[2] = modelTri->vertices[2]->color.z*0xFF;
		
		t->rx = (t->r[0]-t->r[2])*t->dx[0] + (t->r[1]-t->r[2])*t->dx[1];
		t->gx = (t->g[0]-t->g[2])*t->dx[0] + (t->g[1]-t->g[2])*t->dx[1];
		t->bx = (t->b[0]-t->b[2])*t->dx[0] + (t->b[1]-t->b[2])*t->dx[1];
		
		t->r[0] += 0x8000;
		t->r[1] += 0x8000;
		t->r[2] += 0x8000;

		t->g[0] += 0x80;
		t->g[1] += 0x80;
		t->g[2] += 0x80;
		
		t->b[0] += .5;
		t->b[1] += .5;
		t->b[2] += .5;
	}
}

int _REG prepare(_A0(tri *t), _A1(triangle* modelTri)) {
	if(!projection(t, modelTri)) return 0;
	if(!gradient(t)) return 0;
	
	color(t, modelTri);
	
	// setup bounds info
	if(!t->bounds) {
		short i = t->height;
		struct bounds *b;
		
		t->bounds = b = malloc(i*sizeof(*b));
		while(1 + --i) {
			b->min = t->width;
			b->max = -1;
			++b;
		}
	}
	
	// all done
	return 1;
}

static _REG void plot(_A0(tri *t), _D0(int y), _FP0(double x)) {
	if((unsigned)y < (unsigned)t->height) {
		struct bounds *p = &t->bounds[y];
		if(x<p->min) p->min = x;
		if(x>p->max) p->max = x;
		if(0) {
			scalar d0,d1,d2;
			d0 = t->d[0] + (y-t->ymin)*t->dy[0] + x*t->dx[0];
			d1 = t->d[1] + (y-t->ymin)*t->dy[1] + x*t->dx[1];
			d2 = 1 - d1 - d0;
			printf("line=%f %f %f\n", d0, d1, d2);			
		}
		// t->pbuf[(t->height-1 - y)*t->width + x] = -1;
	}
}

static _REG void plot_line(_A0(tri *t), _D0(int i), _D1(int j)) {
	double x = t->x[i], dx;
	int y = t->y[i], k = t->y[j] - y;
	
	// horiz line
	if(k == 0) {
		plot(t, y, t->x[i]);
		plot(t, y, t->x[j]);
		return;
	}

	dx = (t->x[j]-x)/k;
	if(k<0) {
		do {
			plot(t,y,x);
			x -= dx; --y;
		} while(++k);
		plot(t,y,x);
	} else {
		do {
			plot(t,y,x);
			x += dx; ++y;
		} while(--k);
		plot(t,y,x);
	}
}

_REG void plot_triangle(_A0(tri *t))  {
	plot_line(t, 0, 1);
	plot_line(t, 1, 2);
	plot_line(t, 2, 0);
}

_REG void crop_triangle(_A0(tri *t))  {
	int y, m = t->width-1;
	for(y=t->ymin; y<=t->ymax; ++y) {
		int min =  ceil(t->bounds[y].min-.001);
		int max = floor(t->bounds[y].max+.001);
		//printf("%d %d\n", min, max);
		if(min<0)    min = 0;
		if(max>m)    max = m;
		// if(min>max)  min = max;
		t->bounds[y].min = min;
		t->bounds[y].max = max;
	}
	// for(y=t->ymin; y<t->ymax; ++y) {
		// if(t->bounds[y].min>t->bounds[y].max) {
			// t->bounds[y].min = m;
			// t->bounds[y].max = 0;
		// } else break;
	// }
	// t->ymin = y;
	// for(y=t->ymax; y>t->ymin; --y) {
		// if(t->bounds[y].min>t->bounds[y].max) {
			// t->bounds[y].min = m;
			// t->bounds[y].max = 0;
		// } else break;
	// }
	// t->ymax = y;
}

#if M68K==0
#define draw_span_mono	x_draw_span_mono
#define draw_span		x_draw_span

void _REG draw_span_mono(_A0(tri *t), _A1(struct bounds *B)) {
	int k = B->min;
	
	scalar *zb = t->zbuf + k;
	colour *pb = t->pbuf + k;
	
	scalar d0 = k*t->dx[0] + t->d[0];
	scalar d1 = k*t->dx[1] + t->d[1];

	if((k=1+B->max-k)>0) {
	scalar d2 = 1-d0-d1;
	
	scalar w = t->w[0]*d0 + t->w[1]*d1 + t->w[2]*d2;

	while(1 + --k) {
		if(w > *zb++ ) {
			zb[-1] = w;
			*pb = t->col;
		}
		++pb; w += t->wx;
	}}
}

void _REG draw_span(_A0(tri *t), _A1(struct bounds *B)) {
	int k = B->min;
	
	scalar *zb = t->zbuf + k;
	colour *pb = t->pbuf + k;
	
	scalar d0 = k*t->dx[0] + t->d[0];
	scalar d1 = k*t->dx[1] + t->d[1];
	
	if((k=1+B->max-k)>0) {
	scalar d2 = 1-d0-d1;	

	scalar w = t->w[0]*d0 + t->w[1]*d1 + t->w[2]*d2;
	scalar r = t->r[0]*d0 + t->r[1]*d1 + t->r[2]*d2;
	scalar g = t->g[0]*d0 + t->g[1]*d1 + t->g[2]*d2;
	scalar b = t->b[0]*d0 + t->b[1]*d1 + t->b[2]*d2;

	//printf("min/max=%d/%d\n", min,max);
	
	while(1 + --k) {
		if(w > *zb++ ) {
			int t;
			zb[-1] = w;
			t = r; ((long*)pb) [0] = t;
			t = g; ((short*)pb)[1] = t;
			t = b; ((char*)pb) [3] = t;
		}
		++pb; w += t->wx; r += t->rx; g += t->gx; b += t->bx;
	}}
}
#else
extern void _REG draw_span_mono(_A0(tri *t), _A1(struct bounds *B));
extern void _REG draw_span(_A0(tri *t), _A1(struct bounds *B));	
#endif

void _REG draw_triangle(_A0(tri *t)) {
	int idx = (unsigned short)(t->height-1 - t->ymin) * t->width;
	scalar *zb = t->zbuf;
	colour *pb = t->pbuf;
	struct bounds *b = &t->bounds[t->ymin];
	int l = 1+t->ymax - t->ymin;
	
	plot_triangle(t); 
	crop_triangle(t);

	t->zbuf += idx;
	t->pbuf += idx;	
	
	while(1 + --l) {
		// if(b->min <= b->max) {
			if(t->monochrome)
				draw_span_mono(t, b); 
			else
				draw_span(t, b); 			
		// }
		b->min = t->width-1; b->max = 0; ++b;
		t->d[0] += t->dy[0]; t->d[1] += t->dy[1];
		t->zbuf -= t->width; t->pbuf -= t->width;
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
