/**
 * Mesh/model managment functions. Include transforms and flat shading.
 * Raw mesh loader taken from a computer graphics class.
 * (c) L. Diener 2011
 * adapted to AmigaOS by S.Devulder 2018.
 */
#include "models.h"

extern void *alloc(int size);

static int vertexCmp(const void *a_, const void *b_) {
	const vertex *a = a_, *b = b_;
	return vec3Cmp(&a->point, &b->point);
}

static void build_list(model *mdl, struct aa_node *nd) {
	if(nd) {
		vertex *v = (vertex*)nd->data;
		v->next = mdl->vertices;
		mdl->vertices = v;

		build_list(mdl, nd->left);
		build_list(mdl, nd->right);
	}
}

static const vertex *add(aa_tree *tree, float *point) {
	vertex *v = alloc(sizeof(*v));
	const vertex *t;
	
	v->point.x = *point++;
	v->point.y = *point++;
	v->point.z = *point++;
	
	v->normal.x = *point++;
	v->normal.y = *point++;
	v->normal.z = *point++;
	
	t = aa_add(tree, v);
	if(t!=v) free(v);

	return t;
}

model *makeModelFromMesh(model *newModel, float* renderMesh, int tris) {
	aa_tree tree;
	int i;
	
	aa_init(&tree, vertexCmp);
	
	newModel->triangleCount = tris;
	newModel->curTriangle = 0;
	newModel->triangles = alloc(sizeof(triangle)*tris);

	for(i = 0; i<tris; ++i) {
		newModel->triangles[i].ID = i;
		newModel->triangles[i].vertices[0] = add(&tree, &renderMesh[18*i]);
		newModel->triangles[i].vertices[1] = add(&tree, &renderMesh[18*i+6]);
		newModel->triangles[i].vertices[2] = add(&tree, &renderMesh[18*i+12]);
	}

	printf("Vertices: %d, Triangles: %d\n", tree.size, tris);
	
	build_list(newModel, tree.root);
	
	aa_exit(&tree);

	return newModel;
}


model *makeModelFromMeshFile(model* newModel, const char* filename) {
	union {char byte[4]; int bigendian;} byteorder;
	FILE *file;
	
	byteorder.bigendian = 1;
	byteorder.bigendian = byteorder.byte[3]==1;

	if(sizeof(float) != 4) {
		fprintf(stderr, "No 4 byte floats.");
		return NULL;
	}

	if ((file=fopen(filename,"rb"))) {
		int triangleCount;
		float *mesh;
		
		fread(&triangleCount, sizeof(int), 1, file);
		if(byteorder.bigendian) LongSwap(&triangleCount);
		
		mesh = alloc(triangleCount * 18 * sizeof(float));
		fread(mesh, 18 * sizeof(float), triangleCount, file);
		if(byteorder.bigendian) {
			unsigned int *ptr = (void*)mesh;
			int num = triangleCount*18;
			while(num--) LongSwap(ptr++);
		}
		fclose(file);
		
		makeModelFromMesh(newModel, mesh, triangleCount);
		free(mesh);
	} else {
		fprintf(stderr, "Error: Couldn't open \"%s\".\n", filename);
		return NULL;
	}
	return newModel;
}

void freeModel(model* m) {
	vertex *v, *n;
	for(v = m->vertices; v; v = n) {
		n = v->next;
		free(v);
	}
	m->vertices = NULL;
	
	free(m->triangles);
	m->triangleCount = 0;
	m->curTriangle = 0;
}

/*
int modelTrianglesLeft(model* m) {
	return(m->curTriangle != m->triangleCount)?-1:0;
}

triangle* modelNextTriangle(model* m) {
	return &m->triangles[m->curTriangle++];
}
*/

int modelTriangleCount(model* m) {
	return m->triangleCount;
}

void applyTransforms(model* m, matrix *mvMatrixO, matrix *pMatrixO) {
	vertex *v;
	
	for(v = m->vertices; v; v=v->next) {
		vec3 transVec;
		matrixApply(&transVec, mvMatrixO, &v->point);
		matrixApplyPerspective(&v->proj_point, pMatrixO, &transVec);
		
		matrixApplyNormal(&v->proj_normal, mvMatrixO, &v->normal);
	}

	m->curTriangle = 0;
}

void shade(model* m, float lx, float ly, float lz, int component) {
	vertex *v;
	
	for(v = m->vertices; v; v=v->next) {
		// Diffuse lighting, white.
		float Lx = lx - v->point.x;
		float Ly = ly - v->point.y;
		float Lz = lz - v->point.z;

		float ilenL = scalarInvSqrt(Lx*Lx + Ly*Ly + Lz*Lz);
		float NdotL;

		Lx *= ilenL;
		Ly *= ilenL;
		Lz *= ilenL;

		NdotL =
			v->proj_normal.x * Lx +
			v->proj_normal.y * Ly +
			v->proj_normal.z * Lz;

		if (NdotL < 0.0f) NdotL = 0.0f;
			
		switch(component) {
			case 0: v->color.x = NdotL; v->col &= makeColour(0,1,1); v->col |= makeColour(NdotL,0,0); break;
			case 1: v->color.y = NdotL; v->col &= makeColour(1,0,1); v->col |= makeColour(0,NdotL,0); break;
			case 2: v->color.z = NdotL; v->col &= makeColour(1,1,0); v->col |= makeColour(0,0,NdotL); break;
		}
	}
}
