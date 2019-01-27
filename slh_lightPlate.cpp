#include "slh_aux.h"  


struct slh_lightPlate 
{ 
    miColor color; 
    miScalar intensity;
	miScalar fg_multiplier;
    miScalar transparency;
}; 
 
extern "C" DLLEXPORT
int slh_lightPlate_version(void) { return 2; } 
 
extern "C" DLLEXPORT
miBoolean slh_lightPlate ( miColor *result, miState *state, struct slh_lightPlate *params  ) 
{ 
   	//DECLARE VARIABLES
   	miColor color = *mi_eval_color(&params->color);
   	miScalar intensity = *mi_eval_scalar(&params->intensity);
   	miScalar transparency = *mi_eval_scalar(&params->transparency);

	//TRANSPARENCY
	miScalar trans_scale;
	miColor trans_result;

	trans_result.r = trans_result.g = trans_result.b = trans_result.a = 0.0;
	trans_scale = 1.0 - transparency;
	if (transparency < 1.0)
 	{
 		mi_trace_transparent(&trans_result, state);
 	}
 			
	// FG_MULTIPLIER
	miScalar fg_multiplier = 1;
	if (state->type == miRAY_FINALGATHER)
		fg_multiplier = *mi_eval_scalar(&params->fg_multiplier);

   	// RESULTS
	miScalar temp = intensity * fg_multiplier * transparency;

   	result->r = (color.r * temp) + (trans_result.r * trans_scale);
   	result->g = (color.g * temp) + (trans_result.g * trans_scale);
   	result->b = (color.b * temp) + (trans_result.b * trans_scale);
   	result->a = transparency + trans_result.a;


return miTRUE; 
} 
