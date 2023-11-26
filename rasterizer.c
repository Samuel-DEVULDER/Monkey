/**
 * A very simple software rasterizer.
 * (c) L. Diener 2011
 * adapted to amigaOS by S.Devulder 2018
 */

#include "rasterizer.h"

#define SCREEN_X(p) (((p)+1)*((scalar)(width/2)))
#define SCREEN_Y(p) (((p)+1)*((scalar)(height/2)))

typedef struct {
	unsigned int width;
	unsigned int height;
	
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
	int bounds_size;
} tri;

extern unsigned char flat,wire;	

static _REG int projection(_A0(tri *t), _A1(const triangle* modelTri)) {
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
	if(t->ymin > t->ymax || t->xmin > t->xmax) return 0;
	
	if(t->ymin<0)       t->ymin = 0;
	if(t->ymax>=height) t->ymax = height-1;
	
	return 1;
}

static _REG int gradient(_A0(tri *t)) {
	scalar det, a, b, c, d;
	a = t->y[1]-t->y[2]; b = t->x[2]-t->x[1];
	c = t->y[2]-t->y[0]; d = t->x[0]-t->x[2];
	
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

static _REG void color(_A0(tri *t), _A1(const triangle* modelTri)) {
	// determine monochrome
	t->col = modelTri->vertices[0]->col;
	t->monochrome = t->col==modelTri->vertices[1]->col && 
				    t->col==modelTri->vertices[2]->col;
	
	// if not prepare colors
	if(!t->monochrome && flat) {
		scalar z=255/3.0f;
		int c;
		c = z*(modelTri->vertices[0]->color.x + 
		       modelTri->vertices[1]->color.x + 
			   modelTri->vertices[2]->color.x);
		c<<=8;
		c+= z*(modelTri->vertices[0]->color.y + 
		       modelTri->vertices[1]->color.y + 
			   modelTri->vertices[2]->color.y);
		
		c<<=8;
		c+= z*(modelTri->vertices[0]->color.z + 
		       modelTri->vertices[1]->color.z + 
			   modelTri->vertices[2]->color.z);
		t->col = c;
		t->monochrome = 1;
		return;
	}
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

int _REG prepare(_A0(tri *t), _A1(const triangle* modelTri)) {
	if(!projection(t, modelTri)) return 0;
	if(!gradient(t)) return 0;
	
	color(t, modelTri);
	
	// setup bounds info
	if(t->bounds && t->bounds_size!=t->height) {
		free(t->bounds);
		t->bounds = NULL;
	}
	if(!t->bounds) {
		int i = t->bounds_size=t->height, j = t->width, k=-1;
		struct bounds *b;
		
		t->bounds = b = malloc(i*sizeof(*b));
		do {
			b->min = j;
			b->max = k;
			++b;
		} while(--i);
	}
	
	// all done
	return 1;
}

static _REG void plot_x(_A0(tri *t), _D0(int y), _FP0(double x)) {
	if((unsigned)y < (unsigned)t->height) {
		struct bounds *p = &t->bounds[y];
		if(x<p->min) p->min = x;
		if(x>p->max) p->max = x;
		// if(0) {
			// scalar d0,d1,d2;
			// d0 = t->d[0] + (y-t->ymin)*t->dy[0] + x*t->dx[0];
			// d1 = t->d[1] + (y-t->ymin)*t->dy[1] + x*t->dx[1];
			// d2 = 1 - d1 - d0;
			// printf("line=%f %f %f\n", d0, d1, d2);			
		// }
		// t->pbuf[(t->height-1 - y)*t->width + x] = -1;
	}
}

static _REG void plot_line_x(_A0(tri *t), _D0(int i), _D1(int j)) {
	double x = t->x[i], dx;
	int y = t->y[i], k = t->y[j] - y, dy;
	
	// horiz line
	if(k == 0) {
		plot_x(t, y, t->x[i]);
		plot_x(t, y, t->x[j]);
		return;
	}

	dx = (t->x[j]-x)/k; dy = 1;
	
	if(k<0) {k=-k; dx=-dx; dy=-dy;}
	do {
		plot_x(t,y,x);
		x += dx; y += dy;
	} while(--k);
	plot_x(t,y,x);
}

#if M68K==0
#define plot_line plot_line_xx
_REG void plot_line(_A0(tri *t), _D0(int i), _D1(int j)) {
	double x = t->x[i], dx;
	int y = t->y[i], k = t->y[j] - y, dy;
	struct bounds *p = &t->bounds[y];
	
	// horiz line
	if(k == 0) {
		if((unsigned)y < (unsigned)t->height) {
			if(x<p->min) p->min = x;
			if(x>p->max) p->max = x;
			x = t->x[j];
			if(x<p->min) p->min = x;
			if(x>p->max) p->max = x;
		}
		return;
	}

	dx = (t->x[j]-x)/k; dy = 1;
	
	if(k<0) {k=-k; dx=-dx; dy=-dy;}
	if(y<0) {k += y; x -= y*dx; p = &t->bounds[0];if(k<0) return;}
	if(k>=t->height-1) k=t->height-1;
	
	do {
		if(x<p->min) p->min = x;
		if(x>p->max) p->max = x;
		x += dx; p += dy;
	} while(--k>0);
	if(x<p->min) p->min = x;
	if(x>p->max) p->max = x;
}
#else
extern _REG void plot_line(_A0(tri *t), _D0(int i), _D1(int j));
#endif

_REG void plot_triangle(_A0(tri *t))  {
	plot_line(t, 0, 1);
	plot_line(t, 1, 2);
	plot_line(t, 2, 0);
}

#define FLOOR(x) ((int)(x))
#define CEIL(x)	 ((int)((x)+.9999999999))

// #define FLOOR(x)	floor(x)
// #define CEIL(x)		ceil(x)

_REG void crop_triangle(_A0(tri *t))  {
	short k = t->ymax - t->ymin;
	int   m = t->width-1;
	struct bounds *b = &t->bounds[t->ymin];
	double v;
	for(;--k>=-1; ++b) {
		v = CEIL(b->min-.001);
		if(v<0) v = 0;
		b->min = v;
		
		v = FLOOR(b->max+.001);
		if(v>m) v = m;
		b->max = v;
	}
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

#if M68K==0
#define draw_triangle draw_triangle_x
void _REG draw_triangle(_A0(tri *t)) {
	int w = t->width;
	int idx = (unsigned short)(t->height-1 - t->ymin) * (unsigned short)w;
	scalar *zb = t->zbuf;
	colour *pb = t->pbuf;
	struct bounds *b = &t->bounds[t->ymin];
	int l = 1 + t->ymax - t->ymin;
	void _REG (*f)(_A0(tri *t), _A1(struct bounds *B)) = t->monochrome ? draw_span_mono : draw_span;	
	
	plot_triangle(t); 
	crop_triangle(t);

	t->zbuf += idx;
	t->pbuf += idx;	
	while(1 + --l) {
		// if(b->min <= b->max) {
			(*f)(t,b);
		// }
		b->min = w-1; b->max = 0; ++b;
		t->d[0] += t->dy[0]; t->d[1] += t->dy[1];
		t->zbuf -= w; t->pbuf -= w;
	}				
	
	t->zbuf = zb;
	t->pbuf = pb;
}
#else
extern void _REG draw_triangle(_A0(tri *t));
#endif

void rasterize(model* m, buffer* pbuf, scalar* zbuf) {
	static tri t;
	
	// local copy for faster access
	t.width  = pbuf->width;
	t.height = pbuf->size;
	t.pbuf   = pbuf->data;
	t.zbuf   = zbuf;
	
	// reset z-buffer
	do {
		int i = t.width*t.height; 
		scalar *f = zbuf;
		while(1 + --i) *f++ = FLT_MIN;
	} while(0);

	// The actual rasterizer
	while(modelTrianglesLeft(m)) {
		const triangle *modelTri = modelNextTriangle(m);
		
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

static inline _REG void plot(_A0(tri *t), _D0(int x), _D1(int y)) {
	if((unsigned)y < (unsigned)t->height && 
	   (unsigned)x < (unsigned)t->width) {
		t->pbuf[(t->height-1 - y)*t->width + x] = -1;
	}
}

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

static int wu_plot(tri *t, int x, int y, double col) {
	if((unsigned)y < (unsigned)t->height && 
	   (unsigned)x < (unsigned)t->width) {
		int c = col*255, *p = (void*)&t->pbuf[(t->height-1 - y)*t->width + x];
		c *= 0x10101;
		if(c>*p) *p = c;
	}
}

static double fpart(double x) {
	return x-(int)x;
}

static void drawLine(tri *t, int x0, int y0, int x1, int y1) {
	int dx = x1 - x0;
    int dy = y1 - y0;
	double gradient, intery;
	int steep, i;

	if(dx<0) dx = -dx;
	if(dy<0) dy = -dy;
	
	steep = dy>dx;
    if(steep) {x0^=y0; x1^=y1; y0^=x0; y1^=x1; x0^=y0; x1^=y1;}
	if(x0>x1) {x0^=x1; y0^=y1; x1^=x0; y1^=y0; x0^=x1; y0^=y1;}
    
	dx = x1 - x0;
	dy = y1 - y0;
	gradient = dx==0?1:((double)dy)/dx;
	
	intery = y0;
	
    // main loop
	if(steep) {
		for(i=x0; i<=x1; ++i) {
			int x = intery;
        	double f = intery-x;
			wu_plot(t, x,   i, 1-f);
			wu_plot(t, x+1, i,   f);
			intery += gradient;
		}
	} else {
		for(i=x0; i<=x1; ++i) {
			int y = intery;
        	double f = intery-y;
			wu_plot(t, i,   y, 1-f);
			wu_plot(t, i, 1+y,   f);
			intery += gradient;		
		}	
	}
}

_REG void plotTriangle(_A0(tri *t))  {
	plotLine(t, t->x[0], t->y[0], t->x[1], t->y[1]);
	plotLine(t, t->x[0], t->y[0], t->x[2], t->y[2]);
	plotLine(t, t->x[2], t->y[2], t->x[1], t->y[1]);
}

void wireframe(model* m, buffer* pbuf, scalar* zbuf) {
	static tri t;
	
	// local copy for faster access
	t.width  = pbuf->width;
	t.height = pbuf->size;
	t.pbuf   = pbuf->data;
	t.zbuf   = zbuf;

	// The actual rasterizer
	while(modelTrianglesLeft(m)) {
		const triangle *modelTri = modelNextTriangle(m);
		if(projection(&t, modelTri)) plotTriangle(&t);
	}
}
