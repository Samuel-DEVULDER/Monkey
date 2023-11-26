/**
 * Mesh/model managment functions. Include transforms and flat shading.
 * Raw mesh loader taken from a computer graphics class.
 * (c) L. Diener 2011
 */

#ifndef __MODELS_H__
#define __MODELS_H__

#include <stdio.h>
#include <string.h>

#include "colours.h"
#include "matrices.h"
#include "aa_tree.h"

typedef struct vertex {
	struct vertex *next;
	
	vec3 point;
	vec3 normal;
	
	vec3 proj_point;
	vec3 proj_normal;
	
	vec3 color;
	colour col;
} vertex;

typedef struct {
	int ID;
	const vertex* vertices[3];
} triangle;

typedef struct model {
	vertex *vertices;
	
	int triangleCount;
	int curTriangle;

	triangle* triangles;
} model;

model *makeModelFromMeshFile(model *newModel, const char* file);
model *makeModelFromMesh(model *newModel, float* renderMesh, int tris);
void freeModel(model* m);
//int modelTrianglesLeft(model* m);
//triangle* modelNextTriangle(model* m);
#define modelTrianglesLeft(m) ((m)->curTriangle!=(m)->triangleCount)
#define modelNextTriangle(m)  (&(m)->triangles[(m)->curTriangle++])
int modelTriangleCount(model* m);
void applyTransforms(model* m, matrix *mvMatrixO, matrix *pMatrixO);
void shade(model* m, float lx, float ly, float lz, int c);

#endif
