#include "slh_aux.h"
#include "slh_pbrt.h"
#include <iostream>

using namespace std;
using namespace pbrt;

struct slh_glass_params
{
	miScalar	eta;
	miColor		reflect_k;
	miScalar	reflection_roughness;
	int			reflection_samples;
	miColor		refract_k;
	miScalar	transmission_roughness;
	int			transmission_samples;
	miVector	bump;
};


extern "C" DLLEXPORT
int slh_glass_version(void) { return 1; }

extern "C" DLLEXPORT
miBoolean slh_glass(miColor *result, miState *state, struct slh_glass_params *params)
{
	miVector bump_normal = *mi_eval_vector(&params->bump);
	if (bump_normal.x != 0 || bump_normal.y != 0 || bump_normal.z != 0)
		state->normal = bump_normal;

	if (state->inv_normal)
		state->normal = -state->normal;

	CoordinateSystem(state->normal, &state->derivs[0], &state->derivs[1]);

	//Evaluate Parameters
	miScalar	eta = *mi_eval_scalar(&params->eta);
	miColor		reflect_k = *mi_eval_color(&params->reflect_k);
	miColor		refract_k = *mi_eval_color(&params->refract_k);

	miColor refl_res = BLA, refr_res = BLA;
	
	if (notBlack(reflect_k)) {
		miScalar r_roughness = *mi_eval_scalar(&params->reflection_roughness);
		if (r_roughness > 0.f) {
			int	samples = *mi_eval_integer(&params->reflection_samples);
			refl_res = glossy_dielectric_reflection(state, reflect_k, eta, r_roughness, samples);
		}
		else
			refl_res = spec_dielectric_reflection(state, reflect_k, eta);
	}

	if (notBlack(refract_k)) {
		miScalar t_roughness = *mi_eval_scalar(&params->transmission_roughness);
		if (t_roughness > 0.f) {
			int	samples = *mi_eval_integer(&params->transmission_samples);
			refr_res = glossy_dielectric_transmission(state, refract_k, eta, t_roughness, samples);
		}
		else
			refr_res = spec_dielectric_transmission(state, refract_k, eta);
	}

	*result = refl_res + refr_res;
	result->a = 1.f;

	return miTRUE;
}

