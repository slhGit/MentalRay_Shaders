#include "slh_pbrt.h"
#include "reflection.h"

using namespace std;
using namespace pbrt;

// Calculate specular dielectric reflection
miColor spec_dielectric_reflection(miState *state, miColor& reflect_k, miScalar eta) {
	if (PastReflDepth(state) || PastTraceDepth(state))
		return BLA;


	//Setup BSDF
	FresnelDielectric fresnel(1.f, eta);
	SpecularReflection refl(reflect_k, &fresnel);

	Vector3f wi, wo = miWorldToLocal(state,-state->dir);
	miColor trace_res = BLA;

	miVector trace_dir;
	miScalar pdf = 0;

	// Evaluate BSDF
	miScalar temp = AbsDot(state->dir, state->normal);

	// Reflection
	miColor f = refl.Sample_f(wo, &wi, Point2f(0, 0), &pdf, NULL);

	if (pdf) {
		trace_dir = miLocalToWorld(state, wi);

		if (!mi_trace_reflection(&trace_res, state, &trace_dir))
			mi_trace_environment(&trace_res, state, &trace_dir);
	}

	return trace_res * (f * temp / pdf);
}


// Calculate glossy dielectric reflection
miColor glossy_dielectric_reflection(miState *state, miColor& reflect_k, miScalar eta, miScalar roughness, int samples) {
	if (PastReflDepth(state) || PastTraceDepth(state))
		return BLA;

	// Setup BSDF
	FresnelDielectric fresnel(1.f, eta);
	TrowbridgeReitzDistribution distrib(roughness, roughness);
	MicrofacetReflection refl(reflect_k, &distrib, &fresnel);


	Vector3f wi, wo = miWorldToLocal(state, -state->dir);
	miColor refl_sum = BLA, trace_res = BLA;

	// Setup Sampling
	miScalar level = state->reflection_level + state->refraction_level;
	const miUint nSamp = level == 0 ? samples : level == 1 ? std::max(samples / 2, 1) : 1;
	double samp[2];
	int sample_number = 0;

	miScalar temp = AbsDot(state->dir, state->normal);
	while (mi_sample(samp, &sample_number, state, 2, &nSamp)) {
		miVector trace_dir;
		miScalar pdf = 0;

		// Evaluate BSDF
		miColor f = refl.Sample_f(wo, &wi, Point2f(samp[0], samp[1]), &pdf, NULL);

		if (pdf) {
			trace_dir = miLocalToWorld(state, wi);

			if (!mi_trace_reflection(&trace_res, state, &trace_dir))
				mi_trace_environment(&trace_res, state, &trace_dir);

			refl_sum += trace_res * (f * temp / pdf);
		}
	}

	return refl_sum / (miScalar)nSamp;
}



// Calculate specular dielectric transmission
miColor spec_dielectric_transmission(miState *state, miColor& refract_k, miScalar eta) {
	if (PastRefrDepth(state) || PastTraceDepth(state))
		return BLA;

	miColor ret = BLA;

	//Setup BSDF
	FresnelDielectric fresnel(1.f, eta);
	SpecularTransmission refr(refract_k, 1.f, eta, TransportMode::Radiance);

	Vector3f wi, wo = miWorldToLocal(state, -state->dir);

	miColor trace_res = BLA;
	miScalar pdf = 0;

	// Evaluate BSDF
	miScalar temp = AbsDot(state->dir, state->normal);

	// Reflection
	miColor f = refr.Sample_f(wo, &wi, Point2f(0, 0), &pdf, NULL);

	if (pdf){
		miVector trace_dir = miLocalToWorld(state, wi);
		mi_trace_refraction(&trace_res, state, &trace_dir);

		ret = trace_res * (f * temp / pdf);
	}

	return ret;
}

// Calculate glossy dielectric transmission
miColor glossy_dielectric_transmission(miState *state, miColor& refract_k, miScalar eta, miScalar roughness, int samples) {
	if (PastRefrDepth(state) || PastTraceDepth(state))
		return BLA;

	// Setup BSDF
	FresnelDielectric fresnel(1.f, eta);
	TrowbridgeReitzDistribution distrib(roughness, roughness);
	MicrofacetTransmission tran(refract_k, &distrib, 1.f, eta, TransportMode::Radiance);

	Vector3f wi, wo = miWorldToLocal(state, -state->dir);
	miColor refr_sum = BLA, trace_res = BLA;

	// Setup Sampling
	miScalar level = state->reflection_level + state->refraction_level;
	const miUint nSamp = level == 0 ? samples : level == 1 ? std::max(samples / 2, 1) : 1;
	double samp[2];
	int sample_number = 0;

	int skip = 0;

	miScalar dot = AbsDot(state->dir, state->normal);
	while (mi_sample(samp, &sample_number, state, 2, &nSamp)) {
		miVector trace_dir;
		miScalar pdf = 0;

		// Evaluate BSDF
		miColor f = tran.Sample_f(wo, &wi, Point2f(samp[0], samp[1]), &pdf, NULL);

		if (pdf) {
			trace_dir = miLocalToWorld(state, wi);

			mi_trace_refraction(&trace_res, state, &trace_dir);
			refr_sum += trace_res * (f * dot / pdf);
		}
		else {
			skip++;
		}
	}

	miColor ret = refr_sum / (miScalar)nSamp;
	return ret;
}


// Calculate specular metal reflection
miColor spec_metal_reflection(miState *state, miColor& eta, miColor& k) {
	miColor ret = BLA;


	if (PastReflDepth(state) || PastTraceDepth(state))
		return BLA;


	// Setup BSDF
	FresnelConductor frMf(WHI, eta, k);
	SpecularReflection bxdf(WHI, &frMf);

	Vector3f wi, wo = miWorldToLocal(state, -state->dir);

	miColor refl_res = BLA, refl_sum = BLA;

	miScalar pdf = 0;

	// Evaluate BSDF
	miColor f = bxdf.Sample_f(wo, &wi, Point2f(0, 0), &pdf, NULL);

	// Trace reflection
	if (pdf) {
		miVector refl_dir = miLocalToWorld(state, wi);

		if (!mi_trace_reflection(&refl_res, state, &refl_dir))
			mi_trace_environment(&refl_res, state, &refl_dir);

		ret = refl_res * (f * AbsDot(state->dir, state->normal) / pdf);
	}

	ret.a = 1.0;
	
	return ret;
}


// Calculate glossy metal reflection
miColor glossy_metal_reflection(miState *state, miColor& eta, miColor& k, miScalar roughness, int samples) {
	if (PastReflDepth(state) || PastTraceDepth(state))
		return BLA;

	miColor ret = BLA;
	// Setup BSDF
	FresnelConductor frMf(WHI, eta, k);
	TrowbridgeReitzDistribution distrib(roughness, roughness);
	MicrofacetReflection bxdf(WHI, &distrib, &frMf);

	Vector3f wi, wo = miWorldToLocal(state, -state->dir);

	miColor refl_res = BLA, refl_sum = BLA;

	// Setup Sampling
	miScalar level = state->reflection_level + state->refraction_level;
	const miUint nSamp = level == 0 ? samples : level == 1 ? std::max(samples / 2, 1) : 1;
	double samp[2];
	int sample_number = 0;

	while (mi_sample(samp, &sample_number, state, 2, &nSamp)) {
		miVector refl_dir;
		miScalar pdf = 0;

		// Evaluate BSDF
		miColor f = bxdf.Sample_f(wo, &wi, Point2f(samp[0], samp[1]), &pdf, NULL);

		// Trace reflection
		if (pdf) {
			refl_dir = miLocalToWorld(state, wi);

			if (!mi_trace_reflection(&refl_res, state, &refl_dir))
				mi_trace_environment(&refl_res, state, &refl_dir);

			refl_sum += refl_res * (f * AbsDot(state->dir, state->normal) / pdf);
		}
	}

	ret = refl_sum / (miScalar)nSamp;
	ret.a = 1.0;

	return ret;
}

// Calculate lambertian diffuse
miColor lambertian_diffuse(miState *state, miColor& diffuse_k, int light_count, miTag *lights) {
	miColor ret = BLA;

	mi_compute_avg_radiance(&ret, state, 'f', NULL);
	ret *= diffuse_k;

	
	//Setup BSDF
	LambertianReflection diff(diffuse_k);

	int light_sample_count;
	miColor light_color, sum = BLA;
	miVector light_dir;
	miScalar dot_nl;

	miScalar temp = AbsDot(state->dir, state->normal);
	Vector3f wi, wo = miWorldToLocal(state, -state->dir);

	for (int i = 0; i < light_count; i++, lights++) {
		light_sample_count = 0;
		while (mi_sample_light(&light_color, &light_dir, &dot_nl, state, *lights, &light_sample_count)) {
			wi = miWorldToLocal(state, light_dir);
			miColor f = diff.f(wo, wi);

			sum += light_color * f * dot_nl;
		}
		if (light_sample_count) {
			ret += sum / light_sample_count;
		}
	}

	return ret;
}


// Calculate Oren Nayar diffuse
miColor orenNayar_diffuse(miState *state, miColor& diffuse_k, miScalar sigma, int light_count, miTag *lights) {
	miColor ret = BLA;

	// Compute global illumination
	mi_compute_avg_radiance(&ret, state, 'f', NULL);
	ret *= diffuse_k;

	//Setup BSDF
	OrenNayar diff(diffuse_k,sigma);


	//Sample lights
	int light_sample_count;
	miColor light_color, sum = BLA;
	miVector light_dir;
	miScalar dot_nl;

	Vector3f wi, wo = miWorldToLocal(state, -state->dir);
	miScalar temp = AbsDot(state->dir, state->normal);

	for (int i = 0; i < light_count; i++, lights++) {
		light_sample_count = 0;
		while (mi_sample_light(&light_color, &light_dir, &dot_nl, state, *lights, &light_sample_count)) {
			wi = miWorldToLocal(state, light_dir);
			miColor f = diff.f(wo, wi);

			sum += light_color * f * dot_nl;
		}
		if (light_sample_count) {
			ret += sum / light_sample_count;
		}
		sum = BLA;
	} 

	return ret;
}
