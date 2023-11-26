#define STR2(x) #x
#define STR(x) STR2(x)

static int dummy=
#include "VERstring.h"

char *VERstring = "\0$VER: " STR(PROGNAME) " 0." STR(REVISION) " (" DATESTR ") Compiled by Samuel DEVULDER\0";
