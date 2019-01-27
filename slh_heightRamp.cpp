#include "shader.h" 
#include "math.h" 
#include "auxil/slh_aux.h"

struct slh_heightRamp
{ 
	miScalar black_height;
	miScalar white_height;		
}; 

extern "C" DLLEXPORT
int slh_heightRamp_version(void) { return 1; } 

extern "C" DLLEXPORT
miBoolean slh_heightRamp ( miColor *result, miState *state, struct slh_heightRamp *params  ) 
{ 
	miScalar black_height = *mi_eval_scalar(&params->black_height);
	miScalar white_height = *mi_eval_scalar(&params->white_height);
	
	miScalar out_value = 0.0;

	out_value = (state->point.y - white_height)/(black_height - white_height);
	
	result->r = out_value;
   	result->g = out_value;
   	result->b = out_value;
   	result->a = 1.0;
	
	return miTRUE; 
} 
