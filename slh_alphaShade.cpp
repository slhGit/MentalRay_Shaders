#include "slh_aux.h"

extern "C" DLLEXPORT 
int slh_alphaShade_version(void) { return 1; }

extern "C" DLLEXPORT 
miBoolean slh_alphaShade(miColor *result, miState *state, void *params )
{
	result->r = 0.0;
	result->g = 0.0;
	result->b = 0.0;
	result->a = 0.0;
	return miTRUE;
}
