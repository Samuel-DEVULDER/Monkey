/**
 * Shared memory buffers.
 * 2011 edit: Except not because NaCL.
 * (c) L. Diener 2010
 * adapted to amigaOS by S.Devulder 2018
 */

#include "buffers.h"
#include "vectors.h"
#include "bmp_handler.h"

#include <stdlib.h>
/*#include <sys/ipc.h>
#include <sys/shm.h>*/

buffer *makeBuffer(buffer *b, int width, int height) {
	b->width = (width+15)&~15;
	b->firstLine = 0;
	b->size = height;
	/*b.memid = shmget(
		(key_t)0,
		sizeof( colour ) * width * height,
		0660|IPC_CREAT|IPC_PRIVATE
	);*/
	b->data = (colour*)malloc(sizeof( colour ) * b->width * b->size);
	return b;
}

buffer *partialBuffer(buffer *b, int index, int parts) {
	buffer *p = malloc(sizeof(buffer));
	p->width = b->width;
	p->data  = b->data;
	p->size  = b->size;
	p->size /= parts;
	p->firstLine = b->firstLine;
	p->firstLine += p->size * index;
	return p;
}

static int yoffset(buffer *b, int y) {
	return (unsigned short)b->width * (unsigned short)(b->size - y);
}

void setPixel(buffer *b, int x, int y, colour c) {
	b->data[x + yoffset(b,y)] = c;
}


void clear(buffer *b) {
	memset(b->data, 0, yoffset(b,0)*sizeof(colour));
}

void writeToImage(buffer *b, const char* filename) {
	int x,y;
	bmp_init( filename, b->width, b->size );
	for( y = b->firstLine; y < b->size; y++ ) {
		for( x = 0; x < b->width; x++ ) {
			vec3 c; getRGB(&c, b->data[x+yoffset(b,y)] );
			bmp_pixel( (int)c.x, (int)c.y, (int)c.z );
		}
	}
	bmp_close();
}

void freeBuffer(buffer *b) {
	/*shmdt( b->data );	
	shmctl( b.memid, IPC_RMID, 0 );*/
	if(b->data) {free(b->data);b->data = NULL;}
}


