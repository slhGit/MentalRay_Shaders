#include "slh_aux.h"
#include "slh_pbrt.h"
#include "core/reflection.h"

using namespace std;
using namespace pbrt;


struct slh_metal_params
{
	miColor		eta;
	miColor		k;
	miScalar	roughness;
	int			samples;
	miVector	bump;
};

extern "C" DLLEXPORT
int slh_metal_version(void) { return 1; }

extern "C" DLLEXPORT
miBoolean slh_metal(miColor *result, miState *state, struct slh_metal_params *params)
{
	miVector bump_normal = *mi_eval_vector(&params->bump);

	if (bump_normal.x != 0 || bump_normal.y != 0 || bump_normal.z != 0) {
		state->normal = bump_normal;
	}

	if (state->inv_normal) {
		state->normal = -state->normal;
	}

	CoordinateSystem(state->normal, &state->derivs[0], &state->derivs[1]);

	// Evaluate parameters
	miColor eta = *mi_eval_color(&params->eta);
	miColor k = *mi_eval_color(&params->k);
	miScalar roughness = *mi_eval_scalar(&params->roughness);


	if (roughness == 0.f) {
		*result = spec_metal_reflection(state, eta, k);
	}
	else {
		int samples = *mi_eval_integer(&params->samples);
		*result = glossy_metal_reflection(state,eta,k,roughness,samples);
	}

	return miTRUE;
}

