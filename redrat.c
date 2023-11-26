/**
 * redrat model.
 */
#include "models.h"

#define SURFACE_FLAG	1
#define SIDE_FLAG 		2
#define BOTTOM_FLAG 	3
#define PARAM_FLAG 		4

#ifndef M_PI
#define M_PI 3.1415926
#endif

extern void *alloc(int size);

static int cmp_fcn(const void *a_, const void *b_) {
	const param_vertex *a = a_, *b = b_;
	int d = vec3Cmp(&a->vertex.point, &b->vertex.point);
	return d ? d : a->flags - b->flags;
}

static void build_list(model *mdl, struct aa_node *nd) {
	if(nd) {
		param_vertex *v = (param_vertex*)nd->data;
		v->vertex.next = mdl->vertices;
		mdl->vertices = &v->vertex;

		build_list(mdl, nd->left);
		build_list(mdl, nd->right);
	}
}

static const vertex *add(aa_tree *tree, float u, float v, float w, int flags) {
	param_vertex *p = alloc(sizeof(*p)), *p2;
	scalar sqrt2 = scalarSqrt(2);
	scalar phi = -2*M_PI*u;
	scalar x = scalarCos(phi)*v*sqrt2, y = scalarSin(phi)*v*sqrt2;
	
	scalar s=scalarAbs(x);
	scalar t=scalarAbs(y);
	if(t>s) s=t;
	
	if(s>1) { // clip
		x /= s;
		y /= s;
	}
	
	p->u     = u;
	p->v     = v;
	p->flags = flags;
	
	p->vertex.point.x = x;
	p->vertex.point.z = y;
	p->vertex.point.y = w;
	/* normal are computed later */
	
	p2 = aa_add(tree, p);
	if(p2!=p) free(p);

	return &p2->vertex;
}

/*
   1---2
   | \ |
   4---3
*/
static int quad(aa_tree *tree, triangle *tri, int id, 
			double u1, double v1, int f1,
			double u2, double v2, int f2,
			double u3, double v3, int f3,
			double u4, double v4, int f4) {
#define W(flag) ((flag)&PARAM_FLAG?0.0f:-0.25f)
	const vertex *p1 = add(tree, u1, v1, W(f1), f1);
	const vertex *p2 = add(tree, u2, v2, W(f2), f2);
	const vertex *p3 = add(tree, u3, v3, W(f3), f3);
	const vertex *p4 = add(tree, u4, v4, W(f4), f4);
	
	if(p1!=p2 && p1!=p3 && p2!=p3) {
		tri[id].ID = id;
		tri[id].vertices[0] = p1;
		tri[id].vertices[1] = p2;
		tri[id].vertices[2] = p3;
		++id;
	}
	
	if(p1!=p3 && p1!=p4 && p3!=p4) {
		tri[id].ID = id;
		tri[id].vertices[0] = p1;
		tri[id].vertices[1] = p3;
		tri[id].vertices[2] = p4;
		++id;
	}
	
	return id;
}

static int tria(aa_tree *tree, triangle *tri, int id, 
			double u1, double v1, int f1,
			double u2, double v2, int f2,
			double u3, double v3, int f3) {
	const vertex *p1 = add(tree, u1, v1, W(f1), f1);
	const vertex *p2 = add(tree, u2, v2, W(f2), f2);
	const vertex *p3 = add(tree, u3, v3, W(f3), f3);
	
	if(p1!=p2 && p2!=p3 && p1!=p3) {
		tri[id].ID = id;
		tri[id].vertices[0] = p1;
		tri[id].vertices[1] = p2;
		tri[id].vertices[2] = p3;		
		++id;
	}	
	return id;
}

model *makeRedratMesh(model *newModel, int w, int h) {
	scalar _w = scalarDiv(1.0,w), _h = scalarDiv(1.0,h-1);
	aa_tree tree; int i, j, id=0;
	triangle *tri;

	newModel->triangleCount = 2 + w + 2*(h-2)*w + 2*w;
	newModel->curTriangle = 0;
	newModel->triangles = alloc(sizeof(triangle)*newModel->triangleCount);
	
	aa_init(&tree, cmp_fcn);
	
	/* create triangles */
	
	/* flat bottom */
	id = quad(&tree, newModel->triangles, id,
		7.0/8,1, BOTTOM_FLAG,
		5.0/8,1, BOTTOM_FLAG,
		3.0/8,1, BOTTOM_FLAG,
		1.0/8,1, BOTTOM_FLAG); 
	
	/* surface */
	for(i=w; --i>=0;) {
		id = tria(&tree, newModel->triangles, id,
			0,        0, SURFACE_FLAG + PARAM_FLAG,
			(i+0)*_w,_h, SURFACE_FLAG + PARAM_FLAG,
			(i+1)*_w,_h, SURFACE_FLAG + PARAM_FLAG
		);
	}
	for(i=w; --i>=0;) 
	for(j=h-1; --j>=1;) {
		id=quad(&tree, newModel->triangles, id, 
			(i+0)*_w,(j+0)*_h, SURFACE_FLAG + PARAM_FLAG,
			(i+0)*_w,(j+1)*_h, SURFACE_FLAG + PARAM_FLAG,
			(i+1)*_w,(j+1)*_h, SURFACE_FLAG + PARAM_FLAG,
			(i+1)*_w,(j+0)*_h, SURFACE_FLAG + PARAM_FLAG);
	}

	/* sides */ 
	for(i=w; --i>=0;) {
		id=quad(&tree, newModel->triangles, id, 
			(i+1)*_w,1, SIDE_FLAG + PARAM_FLAG,
			(i+0)*_w,1, SIDE_FLAG + PARAM_FLAG,
			(i+0)*_w,1, SIDE_FLAG,
			(i+1)*_w,1, SIDE_FLAG);
	}

	newModel->triangleCount = id;
	printf("Vertices: %d, Triangles: %d\n", tree.size, newModel->triangleCount);
	
	build_list(newModel, tree.root);	
	aa_exit(&tree);
	
	/* create default normal */
	tri = newModel->triangles;
	for(i=newModel->triangleCount; --i>=0;++tri) {
		vec3 a, b, c, *t;
		sub3(&a, &tri->vertices[1]->point, &tri->vertices[0]->point);
		sub3(&b, &tri->vertices[2]->point, &tri->vertices[0]->point);
		cross(&c, &b, &a);
		// normalize3(&c);
		// vecPrintf3(&tri->vertices[0]->point);
		// vecPrintf3(&tri->vertices[1]->point);
		// vecPrintf3(&tri->vertices[2]->point);
		// vecPrintf3(&a);
		// vecPrintf3(&b);
		// vecPrintf3(&c);
		// printf("\n");
	
		t=(void*)&tri->vertices[0]->normal; *t = c;
		t=(void*)&tri->vertices[1]->normal; *t = c;
		t=(void*)&tri->vertices[2]->normal; *t = c;
	}
	return newModel;
}

static double t;

static _REG double z(_FP0(double x), _FP1(double y)) {
	double d2 = x*x+y*y, d=sqrt(d2);
	return scalarSin(5*M_PI*d/log(2+d)-t)/(1+d)*.2+.1;
	// return 0;
}

#define DELTA (1/4096.0)
static _REG double dz_dx(_FP0(double x), _FP1(double y)) {
	return (z(x+DELTA,y)-z(x-DELTA,y))/(2*DELTA);
}

static _REG double dz_dy(_FP0(double x), _FP1(double y)) {
	return (z(x,y+DELTA)-z(x,y-DELTA))/(2*DELTA);
}

void animRedrat(model *model) {
	vertex *v;
	for(v=model->vertices; v; v=v->next) {
		int flags = ((param_vertex*)v)->flags;
		if(flags & PARAM_FLAG) {
			v->point.y = z(v->point.x, v->point.z);
		
			if(flags & SURFACE_FLAG) {
				// x,y,z(x,y)
				// 1,0,dz_dx
				// 0,1,dz_dy
				
				v->normal.x = -dz_dx(v->point.x, v->point.z);
				v->normal.z =  dz_dy(v->point.x, v->point.z);
				v->normal.y = 1;
				// normalize3(&v->normal);
			}
		}	
	}
	t+=0.15;
}
