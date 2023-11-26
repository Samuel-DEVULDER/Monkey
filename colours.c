/**
 * Colour handling.
 * (c) L. Diener 2010,
 * adapted to the AmigaOS by S.Devulder 2018.
 */

#include "colours.h"

_REG vec3 *getRGB(_A0(vec3 *r), _D0(colour c)) {
	return( makeVec3(r, (c>>16)&255, (c>>8)&255, c&255 ) );
}

