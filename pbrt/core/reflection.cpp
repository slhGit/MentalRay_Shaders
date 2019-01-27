
/*
    pbrt source code is Copyright(c) 1998-2016
                        Matt Pharr, Greg Humphreys, and Wenzel Jakob.

    This file is part of pbrt.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

// core/reflection.cpp*
#include "reflection.h"
#include "spectrum.h"
//#include "sampler.h"
#include "sampling.h"
#include "interpolation.h"
//#include "scene.h"
//#include "interaction.h"
//#include "stats.h"
//#include "material.h"

#include "slh_aux.h"

#include <stdarg.h>

namespace pbrt {

// BxDF Utility Functions
miScalar FrDielectric(miScalar cosThetaI, miScalar etaI, miScalar etaT) {
    cosThetaI = Clamp(cosThetaI, -1, 1);
    // Potentially swap indices of refraction
    bool entering = cosThetaI > 0.f;
    if (!entering) {
        std::swap(etaI, etaT);
        cosThetaI = std::abs(cosThetaI);
    }

    // Compute _cosThetaT_ using Snell's law
    miScalar sinThetaI = std::sqrt(std::max((miScalar)0, 1 - cosThetaI * cosThetaI));
    miScalar sinThetaT = etaI / etaT * sinThetaI;

    // Handle total internal reflection
    if (sinThetaT >= 1) return 1;
    miScalar cosThetaT = std::sqrt(std::max((miScalar)0, 1 - sinThetaT * sinThetaT));
    miScalar Rparl = ((etaT * cosThetaI) - (etaI * cosThetaT)) /
                  ((etaT * cosThetaI) + (etaI * cosThetaT));
    miScalar Rperp = ((etaI * cosThetaI) - (etaT * cosThetaT)) /
                  ((etaI * cosThetaI) + (etaT * cosThetaT));
    return (Rparl * Rparl + Rperp * Rperp) / 2;
}

// https://seblagarde.wordpress.com/2013/04/29/memo-on-fresnel-equations/
miColor FrConductor(miScalar cosThetaI, const miColor &etai,
                     const miColor &etat, const miColor &k) {
    cosThetaI = Clamp(cosThetaI, -1, 1);
    miColor eta = etat / etai;
    miColor etak = k / etai;

    miScalar cosThetaI2 = cosThetaI * cosThetaI;
    miScalar sinThetaI2 = 1. - cosThetaI2;
    miColor eta2 = eta * eta;
    miColor etak2 = etak * etak;

    miColor t0 = eta2 - etak2 - sinThetaI2;
    miColor a2plusb2 = Sqrt(t0 * t0 + 4 * eta2 * etak2);
    miColor t1 = a2plusb2 + cosThetaI2;
    miColor a = Sqrt(0.5f * (a2plusb2 + t0));
    miColor t2 = (miScalar)2 * cosThetaI * a;
    miColor Rs = (t1 - t2) / (t1 + t2);

    miColor t3 = cosThetaI2 * a2plusb2 + sinThetaI2 * sinThetaI2;
    miColor t4 = t2 * sinThetaI2;
    miColor Rp = Rs * (t3 - t4) / (t3 + t4);

    return (miScalar)0.5 * (Rp + Rs);
}

// BxDF Method Definitions
miColor ScaledBxDF::f(const Vector3f &wo, const Vector3f &wi) const {
    return scale * bxdf->f(wo, wi);
}

miColor ScaledBxDF::Sample_f(const Vector3f &wo, Vector3f *wi,
                              const Point2f &sample, miScalar *pdf,
                              BxDFType *sampledType) const {
    miColor f = bxdf->Sample_f(wo, wi, sample, pdf, sampledType);
    return scale * f;
}

miScalar ScaledBxDF::Pdf(const Vector3f &wo, const Vector3f &wi) const {
    return bxdf->Pdf(wo, wi);
}

std::string ScaledBxDF::ToString() const {
    return std::string("[ ScaledBxDF bxdf: ") + bxdf->ToString() +
           std::string(" scale: ") + MiToString(scale) + std::string(" ]");
}

Fresnel::~Fresnel() {}
miColor FresnelConductor::Evaluate(miScalar cosThetaI) const {

    return FrConductor(std::abs(cosThetaI), etaI, etaT, k);
}

std::string FresnelConductor::ToString() const {
    return std::string("[ FresnelConductor etaI: ") + MiToString(etaI) +
           std::string(" etaT: ") + MiToString(etaT) + std::string(" k: ") +
           MiToString(k) + std::string(" ]");
}

miColor FresnelDielectric::Evaluate(miScalar cosThetaI) const {
	miScalar v = FrDielectric(cosThetaI, etaI, etaT);
	return { v, v, v, 1.0 };
}

std::string FresnelDielectric::ToString() const {
    return StringPrintf("[ FrenselDielectric etaI: %f etaT: %f ]", etaI, etaT);
}

miColor SpecularReflection::Sample_f(const Vector3f &wo, Vector3f *wi,
                                      const Point2f &sample, miScalar *pdf,
                                      BxDFType *sampledType) const {
    // Compute perfect specular reflection direction
	*wi = Vector3f(-wo.x, -wo.y, wo.z);
    *pdf = 1;
    return fresnel->Evaluate(CosTheta(*wi)) * R / AbsCosTheta(*wi);
}

std::string SpecularReflection::ToString() const {
    return std::string("[ SpecularReflection R: ") + MiToString(R) +
           std::string(" fresnel: ") + fresnel->ToString() + std::string(" ]");
}

miColor SpecularTransmission::Sample_f(const Vector3f &wo, Vector3f *wi,
                                        const Point2f &sample, miScalar *pdf,
                                        BxDFType *sampledType) const {
    // Figure out which $\eta$ is incident and which is transmitted
    bool entering = CosTheta(wo) > 0;
    miScalar etaI = entering ? etaA : etaB;
    miScalar etaT = entering ? etaB : etaA;

    // Compute ray direction for specular transmission
    if (!Refract(wo, Faceforward(Normal3f(0, 0, 1), wo), etaI / etaT, wi))
        return BLA;
    *pdf = 1;
    miColor ft = T * (WHI - fresnel.Evaluate(CosTheta(*wi)));
    // Account for non-symmetry with transmission to different medium
    if (mode == TransportMode::Radiance) ft *= (etaI * etaI) / (etaT * etaT);
    return ft / AbsCosTheta(*wi);
}

std::string SpecularTransmission::ToString() const {
    return std::string("[ SpecularTransmission: T: ") + MiToString(T) +
           StringPrintf(" etaA: %f etaB: %f ", etaA, etaB) +
           std::string(" fresnel: ") + fresnel.ToString() +
           std::string(" mode : ") +
           (mode == TransportMode::Radiance ? std::string("RADIANCE")
                                            : std::string("IMPORTANCE")) +
           std::string(" ]");
}

miColor LambertianReflection::f(const Vector3f &wo, const Vector3f &wi) const {
    return R * InvPi;
}

std::string LambertianReflection::ToString() const {
    return std::string("[ LambertianReflection R: ") + MiToString(R) +
           std::string(" ]");
}

miColor LambertianTransmission::f(const Vector3f &wo,
                                   const Vector3f &wi) const {
    return T * InvPi;
}

std::string LambertianTransmission::ToString() const {
    return std::string("[ LambertianTransmission T: ") + MiToString(T) +
           std::string(" ]");
}

miColor OrenNayar::f(const Vector3f &wo, const Vector3f &wi) const {
    miScalar sinThetaI = SinTheta(wi);
    miScalar sinThetaO = SinTheta(wo);
    // Compute cosine term of Oren-Nayar model
    miScalar maxCos = 0;
    if (sinThetaI > 1e-4 && sinThetaO > 1e-4) {
        miScalar sinPhiI = SinPhi(wi), cosPhiI = CosPhi(wi);
        miScalar sinPhiO = SinPhi(wo), cosPhiO = CosPhi(wo);
        miScalar dCos = cosPhiI * cosPhiO + sinPhiI * sinPhiO;
        maxCos = std::max((miScalar)0, dCos);
    }

    // Compute sine and tangent terms of Oren-Nayar model
    miScalar sinAlpha, tanBeta;
    if (AbsCosTheta(wi) > AbsCosTheta(wo)) {
        sinAlpha = sinThetaO;
        tanBeta = sinThetaI / AbsCosTheta(wi);
    } else {
        sinAlpha = sinThetaI;
        tanBeta = sinThetaO / AbsCosTheta(wo);
    }
    return R * InvPi * (A + B * maxCos * sinAlpha * tanBeta);
}

std::string OrenNayar::ToString() const {
    return std::string("[ OrenNayar R: ") + MiToString(R) +
           StringPrintf(" A: %f B: %f ]", A, B);
}

miColor MicrofacetReflection::f(const Vector3f &wo, const Vector3f &wi) const {
    miScalar cosThetaO = AbsCosTheta(wo), cosThetaI = AbsCosTheta(wi);
    Vector3f wh = wi + wo;
    // Handle degenerate cases for microfacet reflection
    if (cosThetaI == 0 || cosThetaO == 0) return BLA;
    if (wh.x == 0 && wh.y == 0 && wh.z == 0) return BLA;
    wh = Normalize(wh);
    miColor F = fresnel->Evaluate(Dot(wi, wh));
    return R * distribution->D(wh) * distribution->G(wo, wi) * F /
           (4 * cosThetaI * cosThetaO);
}

std::string MicrofacetReflection::ToString() const {
    return std::string("[ MicrofacetReflection R: ") + MiToString(R) +
           std::string(" distribution: ") + distribution->ToString() +
           std::string(" fresnel: ") + fresnel->ToString() + std::string(" ]");
}

miColor MicrofacetTransmission::f(const Vector3f &wo,
                                   const Vector3f &wi) const {
    if (SameHemisphere(wo, wi)) return BLA;  // transmission only

    miScalar cosThetaO = CosTheta(wo);
    miScalar cosThetaI = CosTheta(wi);
    if (cosThetaI == 0 || cosThetaO == 0) return BLA;

    // Compute $\wh$ from $\wo$ and $\wi$ for microfacet transmission
    miScalar eta = CosTheta(wo) > 0 ? (etaB / etaA) : (etaA / etaB);
    Vector3f wh = Normalize(wo + wi * eta);
    if (wh.z < 0) wh = -wh;

    miColor F = fresnel.Evaluate(Dot(wo, wh));

    miScalar sqrtDenom = Dot(wo, wh) + eta * Dot(wi, wh);
    miScalar factor = (mode == TransportMode::Radiance) ? (1 / eta) : 1;

    return (WHI - F) * T *
           std::abs(distribution->D(wh) * distribution->G(wo, wi) * eta * eta *
                    AbsDot(wi, wh) * AbsDot(wo, wh) * factor * factor /
                    (cosThetaI * cosThetaO * sqrtDenom * sqrtDenom));
}

std::string MicrofacetTransmission::ToString() const {
    return std::string("[ MicrofacetTransmission T: ") + MiToString(T) +
           std::string(" distribution: ") + distribution->ToString() +
           StringPrintf(" etaA: %f etaB: %f", etaA, etaB) +
           std::string(" fresnel: ") + fresnel.ToString() +
           std::string(" mode : ") +
           (mode == TransportMode::Radiance ? std::string("RADIANCE")
                                            : std::string("IMPORTANCE")) +
           std::string(" ]");
}

FresnelBlend::FresnelBlend(const miColor &Rd, const miColor &Rs,
                           MicrofacetDistribution *distribution)
    : BxDF(BxDFType(BSDF_REFLECTION | BSDF_GLOSSY)),
      Rd(Rd),
      Rs(Rs),
      distribution(distribution) {}
miColor FresnelBlend::f(const Vector3f &wo, const Vector3f &wi) const {
    auto pow5 = [](miScalar v) { return (v * v) * (v * v) * v; };
    miColor diffuse = (28.f / (23.f * Pi)) * Rd * (WHI - Rs) *
                       (1 - pow5(1 - .5f * AbsCosTheta(wi))) *
                       (1 - pow5(1 - .5f * AbsCosTheta(wo)));
    Vector3f wh = wi + wo;
    if (wh.x == 0 && wh.y == 0 && wh.z == 0) return BLA;
    wh = Normalize(wh);
    miColor specular =
        distribution->D(wh) /
        (4 * AbsDot(wi, wh) * std::max(AbsCosTheta(wi), AbsCosTheta(wo))) *
        SchlickFresnel(Dot(wi, wh));
    return diffuse + specular;
}

std::string FresnelBlend::ToString() const {
    return std::string("[ FresnelBlend Rd: ") + MiToString(Rd) +
           std::string(" Rs: ") + MiToString(Rs) +
           std::string(" distribution: ") + distribution->ToString() +
           std::string(" ]");
}

miColor FourierBSDF::f(const Vector3f &wo, const Vector3f &wi) const {
    // Find the zenith angle cosines and azimuth difference angle
    miScalar muI = CosTheta(-wi), muO = CosTheta(wo);
    miScalar cosPhi = CosDPhi(-wi, wo);

    // Compute Fourier coefficients $a_k$ for $(\mui, \muo)$

    // Determine offsets and weights for $\mui$ and $\muo$
    int offsetI, offsetO;
    miScalar weightsI[4], weightsO[4];
    if (!bsdfTable.GetWeightsAndOffset(muI, &offsetI, weightsI) ||
        !bsdfTable.GetWeightsAndOffset(muO, &offsetO, weightsO))
        return BLA;

    // Allocate storage to accumulate _ak_ coefficients
    miScalar *ak = ALLOCA(miScalar, bsdfTable.mMax * bsdfTable.nChannels);
    memset(ak, 0, bsdfTable.mMax * bsdfTable.nChannels * sizeof(miScalar));

    // Accumulate weighted sums of nearby $a_k$ coefficients
    int mMax = 0;
    for (int b = 0; b < 4; ++b) {
        for (int a = 0; a < 4; ++a) {
            // Add contribution of _(a, b)_ to $a_k$ values
            miScalar weight = weightsI[a] * weightsO[b];
            if (weight != 0) {
                int m;
                const miScalar *ap = bsdfTable.GetAk(offsetI + a, offsetO + b, &m);
                mMax = std::max(mMax, m);
                for (int c = 0; c < bsdfTable.nChannels; ++c)
                    for (int k = 0; k < m; ++k)
                        ak[c * bsdfTable.mMax + k] += weight * ap[c * m + k];
            }
        }
    }

    // Evaluate Fourier expansion for angle $\phi$
    miScalar Y = std::max((miScalar)0, Fourier(ak, mMax, cosPhi));
    miScalar scale = muI != 0 ? (1 / std::abs(muI)) : (miScalar)0;

    // Update _scale_ to account for adjoint light transport
    if (mode == TransportMode::Radiance && muI * muO > 0) {
        float eta = muI > 0 ? 1 / bsdfTable.eta : bsdfTable.eta;
        scale *= eta * eta;
    }
	if (bsdfTable.nChannels == 1) {
		miScalar v = Y * scale;
		return { v,v,v,1.0 };
	}
    else {
        // Compute and return RGB colors for tabulated BSDF
        miScalar R = Fourier(ak + 1 * bsdfTable.mMax, mMax, cosPhi);
        miScalar B = Fourier(ak + 2 * bsdfTable.mMax, mMax, cosPhi);
        miScalar G = 1.39829f * Y - 0.100913f * B - 0.297375f * R;
        miScalar rgb[3] = {R * scale, G * scale, B * scale};
		return { R,G,B,1.0 };
    }
}

std::string FourierBSDF::ToString() const {
    return StringPrintf("[ FourierBSDF eta: %f mMax: %d nChannels: %d nMu: %d ",
                        bsdfTable.eta, bsdfTable.mMax, bsdfTable.nChannels,
                        bsdfTable.nMu) +
           std::string(" mode : ") +
           (mode == TransportMode::Radiance ? std::string("RADIANCE")
                                            : std::string("IMPORTANCE")) +
           std::string(" ]");
}

bool FourierBSDFTable::GetWeightsAndOffset(miScalar cosTheta, int *offset,
                                           miScalar weights[4]) const {
    return CatmullRomWeights(nMu, mu, cosTheta, offset, weights);
}

miColor BxDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                        miScalar *pdf, BxDFType *sampledType) const {
    // Cosine-sample the hemisphere, flipping the direction if necessary
    *wi = CosineSampleHemisphere(u);
    if (wo.z < 0) wi->z *= -1;
    *pdf = Pdf(wo, *wi);
    return f(wo, *wi);
}

miScalar BxDF::Pdf(const Vector3f &wo, const Vector3f &wi) const {
    return SameHemisphere(wo, wi) ? AbsCosTheta(wi) * InvPi : 0;
}

miColor LambertianTransmission::Sample_f(const Vector3f &wo, Vector3f *wi,
                                          const Point2f &u, miScalar *pdf,
                                          BxDFType *sampledType) const {
    *wi = CosineSampleHemisphere(u);
    if (wo.z > 0) wi->z *= -1;
    *pdf = Pdf(wo, *wi);
    return f(wo, *wi);
}

miScalar LambertianTransmission::Pdf(const Vector3f &wo,
                                  const Vector3f &wi) const {
    return !SameHemisphere(wo, wi) ? AbsCosTheta(wi) * InvPi : 0;
}

miColor MicrofacetReflection::Sample_f(const Vector3f &wo, Vector3f *wi,
                                        const Point2f &u, miScalar *pdf,
                                        BxDFType *sampledType) const {
    // Sample microfacet orientation $\wh$ and reflected direction $\wi$
    if (wo.z == 0) return BLA;
    Vector3f wh = distribution->Sample_wh(wo, u);
    *wi = Reflect(wo, wh);
	if (!SameHemisphere(wo, *wi)) {
		return BLA;
	}

    // Compute PDF of _wi_ for microfacet reflection
    *pdf = distribution->Pdf(wo, wh) / (4 * Dot(wo, wh));
    return f(wo, *wi);
}

miScalar MicrofacetReflection::Pdf(const Vector3f &wo, const Vector3f &wi) const {
    if (!SameHemisphere(wo, wi)) return 0;
    Vector3f wh = Normalize(wo + wi);
    return distribution->Pdf(wo, wh) / (4 * Dot(wo, wh));
}

miColor MicrofacetTransmission::Sample_f(const Vector3f &wo, Vector3f *wi,
                                          const Point2f &u, miScalar *pdf,
                                          BxDFType *sampledType) const {
    if (wo.z == 0) return BLA;
    Vector3f wh = distribution->Sample_wh(wo, u);

    miScalar eta = CosTheta(wo) > 0 ? (etaA / etaB) : (etaB / etaA);
    if (!Refract(wo, (Normal3f)wh, eta, wi)) return BLA;
    *pdf = Pdf(wo, *wi);

    return f(wo, *wi);
}

miScalar MicrofacetTransmission::Pdf(const Vector3f &wo,
                                  const Vector3f &wi) const {
    if (SameHemisphere(wo, wi)) return 0;
    // Compute $\wh$ from $\wo$ and $\wi$ for microfacet transmission
    miScalar eta = CosTheta(wo) > 0 ? (etaB / etaA) : (etaA / etaB);
    Vector3f wh = Normalize(wo + wi * eta);

    // Compute change of variables _dwh\_dwi_ for microfacet transmission
    miScalar sqrtDenom = Dot(wo, wh) + eta * Dot(wi, wh);
    miScalar dwh_dwi =
        std::abs((eta * eta * Dot(wi, wh)) / (sqrtDenom * sqrtDenom));
    return distribution->Pdf(wo, wh) * dwh_dwi;
}

miColor FresnelBlend::Sample_f(const Vector3f &wo, Vector3f *wi,
                                const Point2f &uOrig, miScalar *pdf,
                                BxDFType *sampledType) const {
    Point2f u = uOrig;
    if (u[0] < .5) {
        u[0] = std::min(2 * u[0], OneMinusEpsilon);
        // Cosine-sample the hemisphere, flipping the direction if necessary
        *wi = CosineSampleHemisphere(u);
        if (wo.z < 0) wi->z *= -1;
    } else {
        u[0] = std::min(2 * (u[0] - .5f), OneMinusEpsilon);
        // Sample microfacet orientation $\wh$ and reflected direction $\wi$
        Vector3f wh = distribution->Sample_wh(wo, u);
        *wi = Reflect(wo, wh);
        if (!SameHemisphere(wo, *wi)) return BLA;
    }
    *pdf = Pdf(wo, *wi);
    return f(wo, *wi);
}

miScalar FresnelBlend::Pdf(const Vector3f &wo, const Vector3f &wi) const {
    if (!SameHemisphere(wo, wi)) return 0;
    Vector3f wh = Normalize(wo + wi);
    miScalar pdf_wh = distribution->Pdf(wo, wh);
    return .5f * (AbsCosTheta(wi) * InvPi + pdf_wh / (4 * Dot(wo, wh)));
}

miColor FresnelSpecular::Sample_f(const Vector3f &wo, Vector3f *wi,
                                   const Point2f &u, miScalar *pdf,
                                   BxDFType *sampledType) const {


    miScalar F = FrDielectric(CosTheta(wo), etaA, etaB);
    if (u[0] < F) {
        // Compute specular reflection for _FresnelSpecular_

        // Compute perfect specular reflection direction
        *wi = Vector3f(-wo.x, -wo.y, wo.z);
        if (sampledType)
            *sampledType = BxDFType(BSDF_SPECULAR | BSDF_REFLECTION);
        *pdf = F;
        return F * R / AbsCosTheta(*wi);
    } else {
        // Compute specular transmission for _FresnelSpecular_

        // Figure out which $\eta$ is incident and which is transmitted
        bool entering = CosTheta(wo) > 0;
        miScalar etaI = entering ? etaA : etaB;
        miScalar etaT = entering ? etaB : etaA;

        // Compute ray direction for specular transmission
        if (!Refract(wo, Faceforward(Normal3f(0, 0, 1), wo), etaI / etaT, wi))
            return BLA;
        miColor ft = T * (1 - F);

        // Account for non-symmetry with transmission to different medium
		if (mode == TransportMode::Radiance) {
			miScalar temp = (etaI * etaI) / (etaT * etaT);
			ft *= temp;
		}
        if (sampledType)
            *sampledType = BxDFType(BSDF_SPECULAR | BSDF_TRANSMISSION);
        *pdf = 1 - F;
        return ft / AbsCosTheta(*wi);
    }
}

std::string FresnelSpecular::ToString() const {
    return std::string("[ FresnelSpecular R: ") + MiToString(R) +
           std::string(" T: ") + MiToString(T) +
           StringPrintf(" etaA: %f etaB: %f ", etaA, etaB) +
           std::string(" mode : ") +
           (mode == TransportMode::Radiance ? std::string("RADIANCE")
                                            : std::string("IMPORTANCE")) +
           std::string(" ]");
}

miColor FourierBSDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u, miScalar *pdf, BxDFType *sampledType) const {
    // Sample zenith angle component for _FourierBSDF_
    miScalar muO = CosTheta(wo);
    miScalar pdfMu;
    miScalar muI = SampleCatmullRom2D(bsdfTable.nMu, bsdfTable.nMu, bsdfTable.mu,
                                   bsdfTable.mu, bsdfTable.a0, bsdfTable.cdf,
                                   muO, u[1], nullptr, &pdfMu);

    // Compute Fourier coefficients $a_k$ for $(\mui, \muo)$

    // Determine offsets and weights for $\mui$ and $\muo$
    int offsetI, offsetO;
    miScalar weightsI[4], weightsO[4];
    if (!bsdfTable.GetWeightsAndOffset(muI, &offsetI, weightsI) ||
        !bsdfTable.GetWeightsAndOffset(muO, &offsetO, weightsO))
        return BLA;

    // Allocate storage to accumulate _ak_ coefficients
    miScalar *ak = ALLOCA(miScalar, bsdfTable.mMax * bsdfTable.nChannels);
    memset(ak, 0, bsdfTable.mMax * bsdfTable.nChannels * sizeof(miScalar));

    // Accumulate weighted sums of nearby $a_k$ coefficients
    int mMax = 0;
    for (int b = 0; b < 4; ++b) {
        for (int a = 0; a < 4; ++a) {
            // Add contribution of _(a, b)_ to $a_k$ values
            miScalar weight = weightsI[a] * weightsO[b];
            if (weight != 0) {
                int m;
                const miScalar *ap = bsdfTable.GetAk(offsetI + a, offsetO + b, &m);
                mMax = std::max(mMax, m);
                for (int c = 0; c < bsdfTable.nChannels; ++c)
                    for (int k = 0; k < m; ++k)
                        ak[c * bsdfTable.mMax + k] += weight * ap[c * m + k];
            }
        }
    }

    // Importance sample the luminance Fourier expansion
    miScalar phi, pdfPhi;
    miScalar Y = SampleFourier(ak, bsdfTable.recip, mMax, u[0], &pdfPhi, &phi);
    *pdf = std::max((miScalar)0, pdfPhi * pdfMu);

    // Compute the scattered direction for _FourierBSDF_
    miScalar sin2ThetaI = std::max((miScalar)0, 1 - muI * muI);
    miScalar norm = std::sqrt(sin2ThetaI / Sin2Theta(wo));
    if (std::isinf(norm)) norm = 0;
    miScalar sinPhi = std::sin(phi), cosPhi = std::cos(phi);
    *wi = -Vector3f(norm * (cosPhi * wo.x - sinPhi * wo.y),
                    norm * (sinPhi * wo.x + cosPhi * wo.y), muI);

    // Mathematically, wi will be normalized (if wo was). However, in
    // practice, floating-point rounding error can cause some error to
    // accumulate in the computed value of wi here. This can be
    // catastrophic: if the ray intersects an object with the FourierBSDF
    // again and the wo (based on such a wi) is nearly perpendicular to the
    // surface, then the wi computed at the next intersection can end up
    // being substantially (like 4x) longer than normalized, which leads to
    // all sorts of errors, including negative spectral values. Therefore,
    // we normalize again here.
    *wi = Normalize(*wi);

    // Evaluate remaining Fourier expansions for angle $\phi$
    miScalar scale = muI != 0 ? (1 / std::abs(muI)) : (miScalar)0;
    if (mode == TransportMode::Radiance && muI * muO > 0) {
        float eta = muI > 0 ? 1 / bsdfTable.eta : bsdfTable.eta;
        scale *= eta * eta;
    }

	if (bsdfTable.nChannels == 1) { 
		miScalar temp = (Y * scale);
		return { temp, temp, temp, 1.f };
	};
    miScalar R = Fourier(ak + 1 * bsdfTable.mMax, mMax, cosPhi);
    miScalar B = Fourier(ak + 2 * bsdfTable.mMax, mMax, cosPhi);
    miScalar G = 1.39829f * Y - 0.100913f * B - 0.297375f * R;
    miScalar rgb[3] = {R * scale, G * scale, B * scale};
	return { R,G,B,1.0 };
}

miScalar FourierBSDF::Pdf(const Vector3f &wo, const Vector3f &wi) const {
    // Find the zenith angle cosines and azimuth difference angle
    miScalar muI = CosTheta(-wi), muO = CosTheta(wo);
    miScalar cosPhi = CosDPhi(-wi, wo);

    // Compute luminance Fourier coefficients $a_k$ for $(\mui, \muo)$
    int offsetI, offsetO;
    miScalar weightsI[4], weightsO[4];
    if (!bsdfTable.GetWeightsAndOffset(muI, &offsetI, weightsI) ||
        !bsdfTable.GetWeightsAndOffset(muO, &offsetO, weightsO))
        return 0;
    miScalar *ak = ALLOCA(miScalar, bsdfTable.mMax);
    memset(ak, 0, bsdfTable.mMax * sizeof(miScalar));
    int mMax = 0;
    for (int o = 0; o < 4; ++o) {
        for (int i = 0; i < 4; ++i) {
            miScalar weight = weightsI[i] * weightsO[o];
            if (weight == 0) continue;

            int order;
            const miScalar *coeffs =
                bsdfTable.GetAk(offsetI + i, offsetO + o, &order);
            mMax = std::max(mMax, order);

            for (int k = 0; k < order; ++k) ak[k] += *coeffs++ * weight;
        }
    }

    // Evaluate probability of sampling _wi_
    miScalar rho = 0;
    for (int o = 0; o < 4; ++o) {
        if (weightsO[o] == 0) continue;
        rho +=
            weightsO[o] *
            bsdfTable.cdf[(offsetO + o) * bsdfTable.nMu + bsdfTable.nMu - 1] *
            (2 * Pi);
    }
    miScalar Y = Fourier(ak, mMax, cosPhi);
    return (rho > 0 && Y > 0) ? (Y / rho) : 0;
}

miColor BxDF::rho(const Vector3f &w, int nSamples, const Point2f *u) const {
    miColor r = BLA;
    for (int i = 0; i < nSamples; ++i) {
        // Estimate one term of $\rho_\roman{hd}$
        Vector3f wi;
        miScalar pdf = 0;
        miColor f = Sample_f(w, &wi, u[i], &pdf);
        if (pdf > 0) r += f * AbsCosTheta(wi) / pdf;
    }
    return r / nSamples;
}

miColor BxDF::rho(int nSamples, const Point2f *u1, const Point2f *u2) const {
    miColor r = BLA;
    for (int i = 0; i < nSamples; ++i) {
        // Estimate one term of $\rho_\roman{hh}$
        Vector3f wo, wi;
        wo = UniformSampleHemisphere(u1[i]);
        miScalar pdfo = UniformHemispherePdf(), pdfi = 0;
        miColor f = Sample_f(wo, &wi, u2[i], &pdfi);
        if (pdfi > 0)
            r += f * AbsCosTheta(wi) * AbsCosTheta(wo) / (pdfo * pdfi);
    }
    return r / (Pi * nSamples);
}

// BSDF Method Definitions
miColor BSDF::f(const Vector3f &woW, const Vector3f &wiW,
                 BxDFType flags) const {
    //ProfilePhase pp(Prof::BSDFEvaluation);
    Vector3f wi = WorldToLocal(wiW), wo = WorldToLocal(woW);
    if (wo.z == 0) return BLA;
    bool reflect = Dot(wiW, ng) * Dot(woW, ng) > 0;
    miColor f = BLA;
    for (int i = 0; i < nBxDFs; ++i)
        if (bxdfs[i]->MatchesFlags(flags) &&
            ((reflect && (bxdfs[i]->type & BSDF_REFLECTION)) ||
             (!reflect && (bxdfs[i]->type & BSDF_TRANSMISSION))))
            f += bxdfs[i]->f(wo, wi);
    return f;
}

miColor BSDF::rho(int nSamples, const Point2f *samples1,
                   const Point2f *samples2, BxDFType flags) const {
    miColor ret = BLA;
    for (int i = 0; i < nBxDFs; ++i)
        if (bxdfs[i]->MatchesFlags(flags))
            ret += bxdfs[i]->rho(nSamples, samples1, samples2);
    return ret;
}

miColor BSDF::rho(const Vector3f &wo, int nSamples, const Point2f *samples,
                   BxDFType flags) const {
    miColor ret = BLA;
    for (int i = 0; i < nBxDFs; ++i)
        if (bxdfs[i]->MatchesFlags(flags))
            ret += bxdfs[i]->rho(wo, nSamples, samples);
    return ret;
}

miColor BSDF::Sample_f(const Vector3f &woWorld, Vector3f *wiWorld, const Point2f &u, miScalar *pdf, BxDFType type, BxDFType *sampledType) const {


    //ProfilePhase pp(Prof::BSDFSampling);
    // Choose which _BxDF_ to sample
    int matchingComps = NumComponents(type);

    if (matchingComps == 0) {
        *pdf = 0;
        if (sampledType) 
			*sampledType = BxDFType(0);
        return BLA;
    }
    int comp = std::min((int)std::floor(u[0] * matchingComps), matchingComps - 1);

    // Get _BxDF_ pointer for chosen component
    BxDF *bxdf = nullptr;
    int count = comp;
    for (int i = 0; i < nBxDFs; ++i)
        if (bxdfs[i]->MatchesFlags(type) && count-- == 0) {
            bxdf = bxdfs[i];
            break;
        }
    //CHECK_NOTNULL(bxdf);
    //VLOG(2) << "BSDF::Sample_f chose comp = " << comp << " / matching = " << matchingComps << ", bxdf: " << bxdf->ToString();

    // Remap _BxDF_ sample _u_ to $[0,1)^2$
    Point2f uRemapped(std::min(u[0] * matchingComps - comp, OneMinusEpsilon), u[1]);

    // Sample chosen _BxDF_
    Vector3f wi, wo = WorldToLocal(woWorld);
    if (wo.z == 0) return BLA;
    *pdf = 0;
    if (sampledType) *sampledType = bxdf->type;
	miColor f = bxdf->Sample_f(wo, &wi, uRemapped, pdf, sampledType);

    //cout << "For wo = " << wo << ", sampled f = " << f << ", pdf = "
    //       << *pdf << ", ratio = " << ((*pdf > 0) ? (f / *pdf) : BLA)
    //        << ", wi = " << wi << endl;
    if (*pdf == 0) {
        if (sampledType) *sampledType = BxDFType(0);

        return BLA;
    }
    *wiWorld = LocalToWorld(wi);

    // Compute overall PDF with all matching _BxDF_s
    if (!(bxdf->type & BSDF_SPECULAR) && matchingComps > 1)
        for (int i = 0; i < nBxDFs; ++i)
            if (bxdfs[i] != bxdf && bxdfs[i]->MatchesFlags(type))
                *pdf += bxdfs[i]->Pdf(wo, wi);
    if (matchingComps > 1) *pdf /= matchingComps;


    // Compute value of BSDF for sampled direction
    if (!(bxdf->type & BSDF_SPECULAR)) {
        bool reflect = Dot(*wiWorld, ng) * Dot(woWorld, ng) > 0;
        f = BLA;
        for (int i = 0; i < nBxDFs; ++i)
            if (bxdfs[i]->MatchesFlags(type) &&
                ((reflect && (bxdfs[i]->type & BSDF_REFLECTION)) ||
                 (!reflect && (bxdfs[i]->type & BSDF_TRANSMISSION))))
                f += bxdfs[i]->f(wo, wi);
    }
    //VLOG(2) << "Overall f = " << f << ", pdf = " << *pdf << ", ratio = "
    //        << ((*pdf > 0) ? (f / *pdf) : miColor(0.));

	
    return f;
}

miScalar BSDF::Pdf(const Vector3f &woWorld, const Vector3f &wiWorld,
                BxDFType flags) const {
    //ProfilePhase pp(Prof::BSDFPdf);
    if (nBxDFs == 0.f) return 0.f;
    Vector3f wo = WorldToLocal(woWorld), wi = WorldToLocal(wiWorld);
    if (wo.z == 0) return 0.;
    miScalar pdf = 0.f;
    int matchingComps = 0;
    for (int i = 0; i < nBxDFs; ++i)
        if (bxdfs[i]->MatchesFlags(flags)) {
            ++matchingComps;
            pdf += bxdfs[i]->Pdf(wo, wi);
        }
    miScalar v = matchingComps > 0 ? pdf / matchingComps : 0.f;
    return v;
}

std::string BSDF::ToString() const {
    std::string s = StringPrintf("[ BSDF eta: %f nBxDFs: %d", eta, nBxDFs);
    for (int i = 0; i < nBxDFs; ++i)
        s += StringPrintf("\n  bxdfs[%d]: ", i) + bxdfs[i]->ToString();
    return s + std::string(" ]");
}

std::string TypeString[5] = { "Reflection", "Transmission", "Diffuse", "Glossy", "Specular" };
std::ostream& operator<<(std::ostream& out, pbrt::BxDFType A) {
	out << (A == 0 ? "None" : "");
	for (int i = 0; i < 5; i++) {
		if (A & (1 << i))
			out << TypeString[i] << " ";
	}
	return out;
};


}  // namespace pbrt
