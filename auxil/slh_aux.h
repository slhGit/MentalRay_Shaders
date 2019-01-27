//
// Misc functions and definitions
//

#ifndef SLH_AUX_H
#define SLH_AUX_H

#include "shader.h"
#include "slh_vectors.h"
#include "slh_colors.h"
#include "miaux.h"

#include "core/pbrt.h"
#include "core/geometry.h"
#include "core/spectrum.h"

#include <iostream>
#include <algorithm>
#include <cmath>



#ifdef DEBUG
#define miASSERT assert
#else
#define miASSERT(x)
#endif


#ifndef PI
#define PI 3.14159265359
#endif


// if current trace depth is higher than the options depth
inline bool PastReflDepth(miState *state) { return state->reflection_level > state->options->reflection_depth; }
inline bool PastRefrDepth(miState *state) { return state->refraction_level > state->options->refraction_depth; }
inline bool PastTraceDepth(miState *state) { return state->reflection_level + state->refraction_level > state->options->trace_depth; }


// colors
static miColor WHI = { 1,1,1,1 };
static miColor RED = { 1,0,0,1 };
static miColor YEL = { 1,1,0,1 };
static miColor GRE = { 0,1,0,1 };
static miColor CYA = { 0,1,1,1 };
static miColor BLU = { 0,0,1,1 };
static miColor BLA = { 0,0,0,1 };



// check if a color isn't black
inline bool notBlack(miColor &A) { return !(A.r == 0.0 && A.g == 0.0 && A.b == 0.0); }


// deriver is based on someone else's code that I found a long time ago and don't remember, sorry.
void deriver(miState *state, miInteger mode, miVector dir, miVector pole, miScalar rotation);


// fresnel functions
miColor schlickFresnel(miState* state, miVector& outDir, miColor& eta, miColor& k);
miColor schlickApprox(miState* state, miVector& outDir, miColor& k);
miColor frDiel(miState* state, miVector& outDir); 
miColor frCond(miState* state, miVector& outDir, miColor& eta, miColor& k);


// creates a coordinate system from v1, and populates v2 and v3 with the other axis
inline void CoordinateSystem(const miVector &v1, miVector *v2, miVector *v3) {
	miVector temp;
	miScalar inv;
	if (std::abs(v1.x) > std::abs(v1.y)) {
		temp = { -v1.z, 0, v1.x };
		inv = 1.f / std::sqrt(v1.x * v1.x + v1.z * v1.z);
	}
	else {
		temp = { 0, v1.z, -v1.y };
		inv = 1.f / std::sqrt(v1.y * v1.y + v1.z * v1.z);
	}
	*v2 = temp * inv;
	*v3 = Cross(v1, *v2);
}


#endif