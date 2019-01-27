//
// Functions that wrap PBRT classes to use with Mental Ray
//


#ifndef SLH_PBRT
#define SLH_PBRT

#include "slh_aux.h"
#include <iostream>


// Dielectric reflection and transmission
miColor spec_dielectric_reflection(miState *state, miColor& reflect_k, miScalar eta);
miColor glossy_dielectric_reflection(miState *state, miColor& reflect_k, miScalar eta, miScalar roughness, int samples);
miColor spec_dielectric_transmission(miState *state, miColor& refract_k, miScalar eta);
miColor glossy_dielectric_transmission(miState *state, miColor& refract_k, miScalar eta, miScalar roughness, int samples);

// Metal reflection
miColor spec_metal_reflection(miState *state, miColor& eta, miColor& k);
miColor glossy_metal_reflection(miState *state, miColor& eta, miColor& k, miScalar roughness, int samples);

// Diffuse lighting 
miColor lambertian_diffuse(miState *state, miColor& diffuse_k, int light_count, miTag *light);
miColor orenNayar_diffuse(miState *state, miColor& diffuse_k, miScalar sigma, int light_count, miTag *light);


// Convert between PBRT and Mental Ray types
inline pbrt::Normal3f ToPBRTNormal(const miVector &A) { return pbrt::Normal3f(A.x, A.y, A.z); }
inline pbrt::Vector3f ToPBRTVector(miVector &A) { return pbrt::Vector3f(A.x, A.y, A.z); }
inline pbrt::RGBSpectrum ToPBRTSpectrum(const miColor &A) { pbrt::Float RGB[3] = { A.r, A.g, A.b }; return pbrt::RGBSpectrum::FromRGB(RGB); }

inline miVector ToMiVector(const pbrt::Vector3f &A) { return { A.x, A.y, A.z }; }
inline miVector ToMiVector(const pbrt::Normal3f &A) { return { A.x, A.y, A.z }; }
inline miColor ToMiColor(const pbrt::RGBSpectrum &A) { pbrt::Float RGB[3]; A.ToRGB(RGB); return { RGB[0], RGB[1], RGB[2], 1.f }; }



// 
// Functions to convert vectors from world space to local space
// 
// World space is considered the space the mental ray vectors are described in
// Local space is that in which the normal at the current intersection is [0,0,1]
//
// This function assumes state->derivs is filled in.
// If they're not, the functions deriver() or CoordinateSystem() can be used to fill them in.
//

inline pbrt::Vector3f miWorldToLocal(miState *state, const miVector &v) {
	return pbrt::Vector3f(Dot(v, state->derivs[0]), Dot(v, state->derivs[1]), Dot(v, state->normal));
}

inline miVector miLocalToWorld(miState *state, const pbrt::Vector3f &v) {
	return { state->derivs[0].x * v.x + state->derivs[1].x * v.y + state->normal.x * v.z,
		state->derivs[0].y * v.x + state->derivs[1].y * v.y + state->normal.y * v.z,
		state->derivs[0].z * v.x + state->derivs[1].z * v.y + state->normal.z * v.z };
}


#endif