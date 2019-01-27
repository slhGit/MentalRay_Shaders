#include "slh_aux.h"

struct slh_dispersion
{
	miScalar	ior;
	miColor		refraction_color;
	miScalar	scatter;
	int         samples;
};


// used when calculating R G B refractions
static double lb[3] = { 0.0,0.2,0.6 };
static double ub[3] = { 0.4,0.8,1.0 };
static miColor RGB_VEC[3] = { RED,GRE,BLU };



extern "C" DLLEXPORT
int slh_dispersion_version(void) { return 1; }

extern "C" DLLEXPORT
miBoolean slh_dispersion(miColor *result, miState *state, struct slh_dispersion *params) 
{
	if (PastTraceDepth(state)) {
		*result = BLA;
		return miTRUE;
	}


	miState *s = state; //save state as we'll be reset it each refract trace.
	bool entering = miaux_ray_is_entering(state, NULL); //is ray entering material
	
	// variables used throughout
	miColor	 reflect_res = BLA, refract_res = BLA;
	miVector trace_dir;

	// calculate fresnel
	miScalar	ior = *mi_eval_scalar(&params->ior);
	miaux_set_state_refraction_indices(state, ior);
	
	mi_refraction_dir(&trace_dir, state, state->ior_in, state->ior);
	miColor reflect_mult = frDiel(state, trace_dir);


	// REFLECTION
	if (!PastReflDepth(state) && notBlack(reflect_mult))
	{
		mi_reflection_dir(&trace_dir, state);

		if (!mi_trace_reflection(&reflect_res, state, &trace_dir))
			mi_trace_environment(&reflect_res, state, &trace_dir);
	}


	// REFRACTION
	if (!PastRefrDepth(state) && (reflect_mult.r < 1.0f || reflect_mult.g < 1.0f || reflect_mult.b < 1.0f))
	{
		miScalar scatter = *mi_eval_scalar(&params->scatter);

		if (!entering || scatter <= 0.0001) { // if ray is exiting material, or scatter is too small regular refraction
			miaux_set_state_refraction_indices(state, ior);

			if (mi_refraction_dir(&trace_dir, state, state->ior_in, state->ior)) 
				mi_trace_refraction(&refract_res, state, &trace_dir);
			else { 
				mi_reflection_dir(&trace_dir, state);
				if (!mi_trace_reflection(&refract_res, state, &trace_dir))
					mi_trace_environment(&refract_res, state, &trace_dir);
			}
		}
		else { // if ray is entering, split refraction for RGB spliting
			int samples = *mi_eval_integer(&params->samples);
			miColor refract_color = *mi_eval_color(&params->refraction_color);

			miColor calc = BLA, sum = BLA;

			const miUint nSamp = samples;
			double samp[1];

			// reflect for each color
			for (int i = 0; i < 3; i++) // 0 = R, 1 = G, 2 = B
			{
				int sample_number = 0;
				while (mi_sample(samp, &sample_number, state, 1, &nSamp))
				{
					miScalar disp_ior = ior + scatter * miaux_fit(*samp, 0.0, 1.0, lb[i], ub[i]); // pick random IOR per color.

					miaux_set_state_refraction_indices(state, disp_ior);
					if (mi_refraction_dir(&trace_dir, state, state->ior_in, state->ior))
						mi_trace_refraction(&calc, state, &trace_dir);
					else {
						mi_reflection_dir(&trace_dir, state);
						if (!mi_trace_reflection(&calc, state, &trace_dir))
							mi_trace_environment(&calc, state, &trace_dir);
					}


					sum += (calc * RGB_VEC[i]);

					state = s;
					calc = BLA;
				}
			}

			// refract result
			miScalar inv_mult = 1.0 / (miScalar)samples;
			refract_res = sum * inv_mult * refract_color; 
		}
	}

	// RESULT
	if (HasNan(reflect_res))
		reflect_res = BLA;
	if (HasNan(refract_res))
		refract_res = BLA;

	result->r = (reflect_res.r * reflect_mult.r) + (refract_res.r * (1.0 - reflect_mult.r));
	result->g = (reflect_res.g * reflect_mult.g) + (refract_res.g * (1.0 - reflect_mult.g));
	result->b = (reflect_res.b * reflect_mult.b) + (refract_res.b * (1.0 - reflect_mult.b));
	result->a = 1.0;


	return miTRUE;
}

