#include "slh_aux.h"
#include "slh_pbrt.h"
#include "core/reflection.h"
#include <iostream>

#include <vector>

using namespace std;
using namespace pbrt;

struct slh_plastic_params
{
	miScalar	eta;
	miColor		reflect_k;
	miScalar	roughness;
	miColor		diffuse_k;
	miScalar	sigma;
	int			samples;
	miVector	bump;
	int			i_light;
	int			n_light;
	miTag		lights[1];
};



extern "C" DLLEXPORT
int slh_plastic_version(void) { return 1; }

extern "C" DLLEXPORT
miBoolean slh_plastic(miColor *result, miState *state, struct slh_plastic_params *params)
{
	miVector bump_normal = *mi_eval_vector(&params->bump);

	if (bump_normal.x != 0 || bump_normal.y != 0 || bump_normal.z != 0)
		state->normal = bump_normal;


	if (state->inv_normal)
		state->normal = -state->normal;

	CoordinateSystem(state->normal, &state->derivs[0], &state->derivs[1]);

	// Evaluate parameters
	miScalar	eta = *mi_eval_scalar(&params->eta);
	miColor		reflect_k = *mi_eval_color(&params->reflect_k);
	miColor		diffuse_k = *mi_eval_color(&params->diffuse_k);
	miScalar	roughness = *mi_eval_scalar(&params->roughness);
	int			samples = *mi_eval_integer(&params->samples);

	miColor reflect_res = BLA, diffuse_res = BLA;;

	if (notBlack(reflect_k)) {
		if (roughness == 0.f) {
			reflect_res = spec_dielectric_reflection(state, reflect_k, eta);
		}
		else {
			reflect_res = glossy_dielectric_reflection(state, reflect_k, eta, roughness, samples);
		}
	}

	if (notBlack(diffuse_k)) {
		//Evaluate Params
		miScalar sigma = *mi_eval_scalar(&params->sigma);
		int array_offset = *mi_eval_integer(&params->i_light);
		int light_count = *mi_eval_integer(&params->n_light);
		miTag *lights = mi_eval_tag(params->lights) + array_offset;

		diffuse_res = orenNayar_diffuse(state, diffuse_k, sigma, light_count, lights);
	}

	*result = reflect_res + diffuse_res;
	result->a = 1;

	return miTRUE;
}
