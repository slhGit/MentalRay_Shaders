#include "slh_aux.h"
#include "slh_vectors.h"
#include "core/geometry.h"

using namespace std;


// NEEDED TO COMPILE WITH NEWER VERSIONS OF VISUAL STUDIO
#if defined(_MSC_VER) && _MSC_VER >= 1900
// use the legacy stdio defs 
#pragma comment(lib, "legacy_stdio_definitions.lib") 

// however the above doesn't fix the __imp___iob_func link error
#include <stdio.h>
extern "C" { FILE __iob_func[3] = { *stdin,*stdout,*stderr }; }
#endif




// deriver is based on Daniel Rind's deriver
void deriver(miState *state, miInteger mode, miVector dir, miVector pole, miScalar rotation)
{
	miVector	north, u, v;
	//miVector	old_u, old_v;
	float	d, rot;

	if (mode != 0)
	{
		switch (mode)
		{
		case 1:
			// we want to use a constant direction for all rendered points
			// just grab the input and normalize it
			north = dir;
			mi_vector_normalize(&north);
			break;
		case 2:
			// north pole is given, compute local north direction from that
			north = pole;
			mi_vector_sub(&north, &north, &state->point);
			mi_vector_normalize(&north);
			break;
		default:
			break;
		}

		// get "matching" factor
		d = mi_vector_dot(&north, &state->normal);

		// remove partial normal from north pole to yield a shortened v vector
		v.x = north.x - d * state->normal.x;
		v.y = north.y - d * state->normal.y;
		v.z = north.z - d * state->normal.z;

		// stretch to 1.0
		mi_vector_normalize(&v);

		// now rotate, if necessary
		rot = rotation;

		if (fmodf(rot, 1.0f) != 0.0f)
		{
			miMatrix	rotax;

			mi_matrix_rotate_axis(rotax, &state->normal, (float)(rot * M_PI * 2.0f));
			mi_vector_transform(&v, &v, rotax);
		}


		// get u by calculating the cross product
		mi_vector_prod(&u, &state->normal, &v);

		// swap them in
		state->derivs[0] = u;
		state->derivs[1] = v;
	}
}

miColor schlickApprox(miState* state, miVector& outDir, miColor& k)
{
	miScalar mult = std::pow((1.0f - abs(Dot(state->normal, outDir))), 5);
	miColor ret;

	ret.r = (k.r + (1.0 - k.r) * mult);
	ret.g = (k.g + (1.0 - k.g) * mult);
	ret.b = (k.b + (1.0 - k.b) * mult);
	ret.a = 1;

	return ret;
}

miColor schlickFresnel(miState* state, miVector& outDir, miColor& eta, miColor& k) 
{
	miColor temp = eta * eta + k * k;
	miColor RpTmp = eta * 2.f;

	miColor spec_color = (temp - RpTmp + 1.f) / (temp + RpTmp + 1.f);


	miScalar mult = std::pow((1.0f - abs(Dot(state->normal, outDir))), 5);
	miColor ret;

	ret.r = (spec_color.r + (1.0 - spec_color.r) * mult);
	ret.g = (spec_color.g + (1.0 - spec_color.g) * mult);
	ret.b = (spec_color.b + (1.0 - spec_color.b) * mult);
	ret.a = 1;

	return ret;
}

miColor frDiel(miState* state, miVector& outDir) 
{
	miColor ret;
	
	miScalar cost = abs(Dot(state->normal, outDir));
	miScalar cosi = abs(Dot(state->normal, state->dir));

	miScalar Rpr = ((state->ior_in * cosi) - (state->ior * cost)) / ((state->ior_in * cosi) + (state->ior * cost));
	miScalar Rpl = ((state->ior * cosi) - (state->ior_in * cost)) / ((state->ior * cosi) + (state->ior_in * cost));

	miScalar temp = (Rpr*Rpr + Rpl * Rpl) * 0.5f;

	ret.r = ret.g = ret.b = temp;
	ret.a = 1;

	return ret;
}


miColor frCond(miState* state, miVector& outDir, miColor& eta, miColor& k) 
{
	miColor ret;

	miScalar cosi = abs(Dot(state->normal, state->dir));
	miScalar cosi2 = cosi * cosi;

	miColor k2 = k * k;
	miColor eta2 = eta * eta;

	miColor temp_f = eta2 + k2;
	miColor temp = temp_f + cosi2;

	miColor RpTmp = eta * 2.f * cosi;
	
	miColor Rpl = (temp - RpTmp + 1.f) / (temp + RpTmp + 1.f);
	miColor Rpr = (temp_f - RpTmp + cosi2) / (temp_f + RpTmp + cosi2);


	ret = (Rpr + Rpl) * 0.5f;
	ret.a = 1;

	return ret;
}
