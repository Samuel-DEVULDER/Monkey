/**
 * Colour handling.
 * (c) L. Diener 2010, 
 * adapted to amigaOS by S.Devulder 2018
 */

#ifndef __COLOURS_H__
#define __COLOURS_H__

#include <math.h>
#include "scalars.h"
#include "vectors.h"

typedef unsigned long colour; /* ARGB */

#define COLOUR_WHITE ((colour)0xFFFFFF)
#define COLOUR_BLACK ((colour)0x000000)

#define makeColour(r,g,b) ((colour)((int)((r)*255)*65536 + (int)((g)*255)*256 + (int)((b)*255)))
_REG vec3 *getRGB(_A0(vec3 *out), _D0(colour c));

#endif
