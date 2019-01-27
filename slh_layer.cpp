#include "slh_aux.h"

struct shader_list {
	miScalar	weight;
	miTag		shader;
};

struct slh_layer
{
	int			i_list;
	int			n_list;
	shader_list s_list[1];
	miVector	bump;
};

extern "C" DLLEXPORT
int slh_layer_version(void) { return 1; }

extern "C" DLLEXPORT
miBoolean slh_layer(miColor *result, miState *state, struct slh_layer *params)
{
	// Apply bump map if any
	miVector bump_normal = *mi_eval_vector(&params->bump);

	if (bump_normal.x != 0 || bump_normal.y != 0 || bump_normal.z != 0)
		state->normal = bump_normal;

	// Evaluate params
	int i_off = *mi_eval_integer(&params->i_list);
	int n_off = *mi_eval_integer(&params->n_list);

	miColor res = BLA;

	// Loop over shaders 
	miScalar totalWeight = 1.0; //total possible percentage of cuuent layer
	for (int i = 0; i < n_off; i++) {
		if (totalWeight > 0.f) {
			shader_list *ts = &params->s_list[i + i_off];
			miScalar weight = *mi_eval_scalar(&ts->weight);

			if (weight > 0.f) {
				miTag tag = *mi_eval_tag(&ts->shader);
				miColor shader_res;

				mi_call_shader(&shader_res, miSHADER_MATERIAL, state, tag);
				res += shader_res * totalWeight * weight;
				totalWeight *= 1.f - weight;
			}
		}
	}

	*result = res;

	return miTRUE;
}
