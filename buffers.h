/**
 * Shared memory buffers.
 * (c) L. Diener 2010
 * adapted to amigaOS by S.Devulder 2018
 */

#ifndef __BUFFERS_H__
#define __BUFFERS_H__

#include "scalars.h"
#include "colours.h"

#define SHMKEY "RTSharedMemoryBuffer"

typedef struct {
	int width;
	int firstLine;
	int size;
	// int memid;
	colour* data;
} buffer;

buffer *makeBuffer(buffer *b,int width, int height);
buffer *partialBuffer(buffer *b, int index, int parts);
void setPixel(buffer *b, int x, int y, colour c);
void writeToImage(buffer *b, const char* filename);
void freeBuffer(buffer *b);
void freePartialBuffer(buffer *b);
void clear(buffer *b);

#endif
