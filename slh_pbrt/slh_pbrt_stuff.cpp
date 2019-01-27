#include "shader.h"
#include "math.h"
#include "slh_aux.h"
#include "slh_pbrt.h"
#include <algorithm>
#include <iostream>

using namespace pbrt;

struct slh_metal_schlick
{
	miColor		color;
};

extern "C" DLLEXPORT
int slh_metal_schlick_version(void) { return 1; }

extern "C" DLLEXPORT
miBoolean slh_metal_schlick(miColor *result, miState *state, struct slh_metal_schlick *params)
{
	miColor color = *mi_eval_color(&params->color);

	miVector refl_dir;
	miColor reflect_res;

	mi_reflection_dir(&refl_dir, state);

	if (!mi_trace_reflection(&reflect_res, state, &refl_dir))
		mi_trace_environment(&reflect_res, state, &refl_dir);

	miColor fresnel = schlickApprox(state, refl_dir, color);

	result->r = reflect_res.r * fresnel.r;
	result->g = reflect_res.g * fresnel.g;
	result->b = reflect_res.b * fresnel.b;
	result->a = 1.0;
	
	return miTRUE;
}




