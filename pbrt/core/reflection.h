//
// ADAPTED TO USE WITH MENTAL RAY
//

#define NOMINMAX
#pragma once



#ifndef PBRT_CORE_REFLECTION_H
#define PBRT_CORE_REFLECTION_H




#include "pbrt.h"
#include "geometry.h"
#include "spectrum.h"
#include "microfacet.h"


#include "slh_aux.h"
#include "slh_pbrt.h"

#include <iostream>;
using namespace std;

namespace pbrt {

// Reflection Declarations
miScalar FrDielectric(miScalar cosThetaI, miScalar etaI, miScalar etaT);
miColor FrConductor(miScalar cosThetaI, const miColor &etaI,
                     const miColor &etaT, const miColor &k);

// BSDF Inline Functions
inline miScalar CosTheta(const Vector3f &w) { return w.z; }
inline miScalar Cos2Theta(const Vector3f &w) { return w.z * w.z; }
inline miScalar AbsCosTheta(const Vector3f &w) { return std::abs(w.z); }
inline miScalar Sin2Theta(const Vector3f &w) {
    return std::max((miScalar)0, (miScalar)1 - Cos2Theta(w));
}

inline miScalar SinTheta(const Vector3f &w) { return std::sqrt(Sin2Theta(w)); }

inline miScalar TanTheta(const Vector3f &w) { return SinTheta(w) / CosTheta(w); }

inline miScalar Tan2Theta(const Vector3f &w) {
    return Sin2Theta(w) / Cos2Theta(w);
}

inline miScalar CosPhi(const Vector3f &w) {
    miScalar sinTheta = SinTheta(w);
    return (sinTheta == 0) ? 1 : Clamp(w.x / sinTheta, -1, 1);
}

inline miScalar SinPhi(const Vector3f &w) {
    miScalar sinTheta = SinTheta(w);
    return (sinTheta == 0) ? 0 : Clamp(w.y / sinTheta, -1, 1);
}

inline miScalar Cos2Phi(const Vector3f &w) { return CosPhi(w) * CosPhi(w); }

inline miScalar Sin2Phi(const Vector3f &w) { return SinPhi(w) * SinPhi(w); }

inline miScalar CosDPhi(const Vector3f &wa, const Vector3f &wb) {
    return Clamp(
        (wa.x * wb.x + wa.y * wb.y) / std::sqrt((wa.x * wa.x + wa.y * wa.y) *
                                                (wb.x * wb.x + wb.y * wb.y)),
        -1, 1);
}

inline Vector3f Reflect(const Vector3f &wo, const Vector3f &n) {
    return -wo + 2 * Dot(wo, n) * n;
}

inline bool Refract(const Vector3f &wi, const Normal3f &n, miScalar eta,
                    Vector3f *wt) {
    // Compute $\cos \theta_\roman{t}$ using Snell's law
    miScalar cosThetaI = Dot(n, wi);
    miScalar sin2ThetaI = std::max(miScalar(0), miScalar(1 - cosThetaI * cosThetaI));
    miScalar sin2ThetaT = eta * eta * sin2ThetaI;

    // Handle total internal reflection for transmission
    if (sin2ThetaT >= 1) return false;
    miScalar cosThetaT = std::sqrt(1 - sin2ThetaT);
    *wt = eta * -wi + (eta * cosThetaI - cosThetaT) * Vector3f(n);
    return true;
}

inline bool SameHemisphere(const Vector3f &w, const Vector3f &wp) {
    return w.z * wp.z > 0;
}

inline bool SameHemisphere(const Vector3f &w, const Normal3f &wp) {
    return w.z * wp.z > 0;
}

// BSDF Declarations
enum BxDFType {
    BSDF_REFLECTION = 1 << 0,
    BSDF_TRANSMISSION = 1 << 1,
    BSDF_DIFFUSE = 1 << 2,
    BSDF_GLOSSY = 1 << 3,
    BSDF_SPECULAR = 1 << 4,
    BSDF_ALL = BSDF_DIFFUSE | BSDF_GLOSSY | BSDF_SPECULAR | BSDF_REFLECTION |
               BSDF_TRANSMISSION,
};

std::ostream& operator<<(std::ostream& out, BxDFType A);

struct FourierBSDFTable {
    // FourierBSDFTable Public Data
    miScalar eta;
    int mMax;
    int nChannels;
    int nMu;
    miScalar *mu;
    int *m;
    int *aOffset;
    miScalar *a;
    miScalar *a0;
    miScalar *cdf;
    miScalar *recip;

    // FourierBSDFTable Public Methods
    static bool Read(const std::string &filename, FourierBSDFTable *table);
    const miScalar *GetAk(int offsetI, int offsetO, int *mptr) const {
        *mptr = m[offsetO * nMu + offsetI];
        return a + aOffset[offsetO * nMu + offsetI];
    }
    bool GetWeightsAndOffset(miScalar cosTheta, int *offset,
                             miScalar weights[4]) const;
};

class BSDF {
public:
	// BSDF Public Methods
	BSDF(miState * state, miScalar eta = 1)
		: eta(eta),
		ns(ToPBRTNormal(state->normal)),
		ng(ToPBRTNormal(state->normal)),
		ss(Normalize(ToPBRTVector(state->derivs[0]))),
		ts(Cross(ns, ss)) {}
	void Add(BxDF *b) {
		//CHECK_LT(nBxDFs, MaxBxDFs);
		bxdfs[nBxDFs++] = b;
	}
	int NumComponents(BxDFType flags = BSDF_ALL) const;
	Vector3f WorldToLocal(const Vector3f &v) const {
		return Vector3f(Dot(v, ss), Dot(v, ts), Dot(v, ns));
	}
	Vector3f LocalToWorld(const Vector3f &v) const {
		return Vector3f(ss.x * v.x + ts.x * v.y + ns.x * v.z,
			ss.y * v.x + ts.y * v.y + ns.y * v.z,
			ss.z * v.x + ts.z * v.y + ns.z * v.z);
	}
	miColor f(const Vector3f &woW, const Vector3f &wiW,
		BxDFType flags = BSDF_ALL) const;
	miColor rho(int nSamples, const Point2f *samples1, const Point2f *samples2,
		BxDFType flags = BSDF_ALL) const;
	miColor rho(const Vector3f &wo, int nSamples, const Point2f *samples,
		BxDFType flags = BSDF_ALL) const;
	miColor Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
		miScalar *pdf, BxDFType type = BSDF_ALL,
		BxDFType *sampledType = nullptr) const;
	miScalar Pdf(const Vector3f &wo, const Vector3f &wi,
		BxDFType flags = BSDF_ALL) const;
	std::string ToString() const;

	//Deconstructor made public
	~BSDF() {} 

	// BSDF Public Data
	const miScalar eta;

private:
	// BSDF Private Methods
	

	// BSDF Private Data
	const Normal3f ns, ng;
	const Vector3f ss, ts;
	int nBxDFs = 0;
	static PBRT_CONSTEXPR int MaxBxDFs = 8;
	BxDF *bxdfs[MaxBxDFs];
	friend class MixMaterial;
};


inline std::ostream &operator<<(std::ostream &os, const BSDF &bsdf) {
    os << bsdf.ToString();
    return os;
}

// BxDF Declarations
class BxDF {
  public:
    // BxDF Interface
    virtual ~BxDF() {}
    BxDF(BxDFType type) : type(type) {}
    bool MatchesFlags(BxDFType t) const { return (type & t) == type; }
    virtual miColor f(const Vector3f &wo, const Vector3f &wi) const = 0;
    virtual miColor Sample_f(const Vector3f &wo, Vector3f *wi,
                              const Point2f &sample, miScalar *pdf,
                              BxDFType *sampledType = nullptr) const;
    virtual miColor rho(const Vector3f &wo, int nSamples,
                         const Point2f *samples) const;
    virtual miColor rho(int nSamples, const Point2f *samples1,
                         const Point2f *samples2) const;
    virtual miScalar Pdf(const Vector3f &wo, const Vector3f &wi) const;
    virtual std::string ToString() const = 0;

    // BxDF Public Data
    const BxDFType type;
};

inline std::ostream &operator<<(std::ostream &os, const BxDF &bxdf) {
    os << bxdf.ToString();
    return os;
}

class ScaledBxDF : public BxDF {
  public:
    // ScaledBxDF Public Methods
    ScaledBxDF(BxDF *bxdf, const miColor &scale)
        : BxDF(BxDFType(bxdf->type)), bxdf(bxdf), scale(scale) {}
    miColor rho(const Vector3f &w, int nSamples,
                 const Point2f *samples) const {
        return scale * bxdf->rho(w, nSamples, samples);
    }
    miColor rho(int nSamples, const Point2f *samples1,
                 const Point2f *samples2) const {
        return scale * bxdf->rho(nSamples, samples1, samples2);
    }
    miColor f(const Vector3f &wo, const Vector3f &wi) const;
    miColor Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &sample,
                      miScalar *pdf, BxDFType *sampledType) const;
    miScalar Pdf(const Vector3f &wo, const Vector3f &wi) const;
    std::string ToString() const;

	//~ScaledBxDF() {
	//	if (bxdf) delete bxdf;
	//}

  private:
    BxDF *bxdf;
    miColor scale;
};

class Fresnel {
  public:
    // Fresnel Interface
    virtual ~Fresnel();
    virtual miColor Evaluate(miScalar cosI) const = 0;
    virtual std::string ToString() const = 0;
};

inline std::ostream &operator<<(std::ostream &os, const Fresnel &f) {
    os << f.ToString();
    return os;
}

class FresnelConductor : public Fresnel {
  public:
    // FresnelConductor Public Methods
    miColor Evaluate(miScalar cosThetaI) const;
    FresnelConductor(const miColor &etaI, const miColor &etaT,
                     const miColor &k)
        : etaI(etaI), etaT(etaT), k(k) 
	{
		//cout << etaT << " " << k << "\n";
	}
    std::string ToString() const;

  private:
    miColor etaI, etaT, k;
};

class FresnelDielectric : public Fresnel {
  public:
    // FresnelDielectric Public Methods
    miColor Evaluate(miScalar cosThetaI) const;
    FresnelDielectric(miScalar etaI, miScalar etaT) : etaI(etaI), etaT(etaT) {}
    std::string ToString() const;

  private:
    miScalar etaI, etaT;
};

class FresnelNoOp : public Fresnel {
  public:
    miColor Evaluate(miScalar) const { return WHI; }
    std::string ToString() const { return "[ FresnelNoOp ]"; }
};

class SpecularReflection : public BxDF {
  public:
    // SpecularReflection Public Methods
    SpecularReflection(const miColor &R, Fresnel *fresnel)
        : BxDF(BxDFType(BSDF_REFLECTION | BSDF_SPECULAR)),
          R(R),
          fresnel(fresnel) {}
    miColor f(const Vector3f &wo, const Vector3f &wi) const {
        return BLA;
    }
    miColor Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &sample,
                      miScalar *pdf, BxDFType *sampledType) const;
    miScalar Pdf(const Vector3f &wo, const Vector3f &wi) const { return 0; }
    std::string ToString() const;

	//~SpecularReflection() {
	//	if (fresnel) delete fresnel;
	//}

  private:
    // SpecularReflection Private Data
    const miColor R;
    const Fresnel *fresnel;
};

class SpecularTransmission : public BxDF {
  public:
    // SpecularTransmission Public Methods
    SpecularTransmission(const miColor &T, miScalar etaA, miScalar etaB,
                         TransportMode mode)
        : BxDF(BxDFType(BSDF_TRANSMISSION | BSDF_SPECULAR)),
          T(T),
          etaA(etaA),
          etaB(etaB),
          fresnel(etaA, etaB),
          mode(mode) {}
    miColor f(const Vector3f &wo, const Vector3f &wi) const {
        return BLA;
    }
    miColor Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &sample,
                      miScalar *pdf, BxDFType *sampledType) const;
    miScalar Pdf(const Vector3f &wo, const Vector3f &wi) const { return 0; }
    std::string ToString() const;

  private:
    // SpecularTransmission Private Data
    const miColor T;
    const miScalar etaA, etaB;
    const FresnelDielectric fresnel;
    const TransportMode mode;
};

class FresnelSpecular : public BxDF {
  public:
    // FresnelSpecular Public Methods
    FresnelSpecular(const miColor &R, const miColor &T, miScalar etaA,
                    miScalar etaB, TransportMode mode)
        : BxDF(BxDFType(BSDF_REFLECTION | BSDF_TRANSMISSION | BSDF_SPECULAR)),
          R(R),
          T(T),
          etaA(etaA),
          etaB(etaB),
          mode(mode) {}
    miColor f(const Vector3f &wo, const Vector3f &wi) const {
        return BLA;
    }
    miColor Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                      miScalar *pdf, BxDFType *sampledType) const;
    miScalar Pdf(const Vector3f &wo, const Vector3f &wi) const { return 0; }
    std::string ToString() const;

  private:
    // FresnelSpecular Private Data
    const miColor R, T;
    const miScalar etaA, etaB;
    const TransportMode mode;
};

class LambertianReflection : public BxDF {
  public:
    // LambertianReflection Public Methods
    LambertianReflection(const miColor &R)
        : BxDF(BxDFType(BSDF_REFLECTION | BSDF_DIFFUSE)), R(R) {}
    miColor f(const Vector3f &wo, const Vector3f &wi) const;
    miColor rho(const Vector3f &, int, const Point2f *) const { return R; }
    miColor rho(int, const Point2f *, const Point2f *) const { return R; }
    std::string ToString() const;

  private:
    // LambertianReflection Private Data
    const miColor R;
};

class LambertianTransmission : public BxDF {
  public:
    // LambertianTransmission Public Methods
    LambertianTransmission(const miColor &T)
        : BxDF(BxDFType(BSDF_TRANSMISSION | BSDF_DIFFUSE)), T(T) {}
    miColor f(const Vector3f &wo, const Vector3f &wi) const;
    miColor rho(const Vector3f &, int, const Point2f *) const { return T; }
    miColor rho(int, const Point2f *, const Point2f *) const { return T; }
    miColor Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                      miScalar *pdf, BxDFType *sampledType) const;
    miScalar Pdf(const Vector3f &wo, const Vector3f &wi) const;
    std::string ToString() const;

  private:
    // LambertianTransmission Private Data
    miColor T;
};

class OrenNayar : public BxDF {
  public:
    // OrenNayar Public Methods
    miColor f(const Vector3f &wo, const Vector3f &wi) const;
    OrenNayar(const miColor &R, miScalar sigma)
        : BxDF(BxDFType(BSDF_REFLECTION | BSDF_DIFFUSE)), R(R) {
        sigma = Radians(sigma);
        miScalar sigma2 = sigma * sigma;
        A = 1.f - (sigma2 / (2.f * (sigma2 + 0.33f)));
        B = 0.45f * sigma2 / (sigma2 + 0.09f);
    }
    std::string ToString() const;

  private:
    // OrenNayar Private Data
    const miColor R;
    miScalar A, B;
};

class MicrofacetReflection : public BxDF {
  public:
    // MicrofacetReflection Public Methods
    MicrofacetReflection(const miColor &R,
                         MicrofacetDistribution *distribution, Fresnel *fresnel)
        : BxDF(BxDFType(BSDF_REFLECTION | BSDF_GLOSSY)),
          R(R),
          distribution(distribution),
          fresnel(fresnel) {}
    miColor f(const Vector3f &wo, const Vector3f &wi) const;
    miColor Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                      miScalar *pdf, BxDFType *sampledType) const;
    miScalar Pdf(const Vector3f &wo, const Vector3f &wi) const;
    std::string ToString() const;

	//~MicrofacetReflection() {
	//	if (distribution) delete distribution;
	//	if (fresnel) delete fresnel;
	//}
  private:
    // MicrofacetReflection Private Data
    const miColor R;
    const MicrofacetDistribution *distribution;
    const Fresnel *fresnel;
};

class MicrofacetTransmission : public BxDF {
  public:
    // MicrofacetTransmission Public Methods
    MicrofacetTransmission(const miColor &T,
                           MicrofacetDistribution *distribution, miScalar etaA,
                           miScalar etaB, TransportMode mode)
        : BxDF(BxDFType(BSDF_TRANSMISSION | BSDF_GLOSSY)),
          T(T),
          distribution(distribution),
          etaA(etaA),
          etaB(etaB),
          fresnel(etaA, etaB),
          mode(mode) {}
    miColor f(const Vector3f &wo, const Vector3f &wi) const;
    miColor Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                      miScalar *pdf, BxDFType *sampledType) const;
    miScalar Pdf(const Vector3f &wo, const Vector3f &wi) const;
    std::string ToString() const;

  private:
    // MicrofacetTransmission Private Data
    const miColor T;
    const MicrofacetDistribution *distribution;
    const miScalar etaA, etaB;
    const FresnelDielectric fresnel;
    const TransportMode mode;
};

class FresnelBlend : public BxDF {
  public:
    // FresnelBlend Public Methods
    FresnelBlend(const miColor &Rd, const miColor &Rs,
                 MicrofacetDistribution *distrib);
    miColor f(const Vector3f &wo, const Vector3f &wi) const;
    miColor SchlickFresnel(miScalar cosTheta) const {
        auto pow5 = [](miScalar v) { return (v * v) * (v * v) * v; };
        return Rs + pow5(1 - cosTheta) * (WHI - Rs);
    }
    miColor Sample_f(const Vector3f &wi, Vector3f *sampled_f, const Point2f &u,
                      miScalar *pdf, BxDFType *sampledType) const;
    miScalar Pdf(const Vector3f &wo, const Vector3f &wi) const;
    std::string ToString() const;

	//~FresnelBlend() {
	//	if (distribution) delete distribution;
	//}
  private:
    // FresnelBlend Private Data
    const miColor Rd, Rs;
    MicrofacetDistribution *distribution;
};

class FourierBSDF : public BxDF {
  public:
    // FourierBSDF Public Methods
    miColor f(const Vector3f &wo, const Vector3f &wi) const;
    FourierBSDF(const FourierBSDFTable &bsdfTable, TransportMode mode)
        : BxDF(BxDFType(BSDF_REFLECTION | BSDF_TRANSMISSION | BSDF_GLOSSY)),
          bsdfTable(bsdfTable),
          mode(mode) {}
    miColor Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                      miScalar *pdf, BxDFType *sampledType) const;
    miScalar Pdf(const Vector3f &wo, const Vector3f &wi) const;
    std::string ToString() const;

  private:
    // FourierBSDF Private Data
    const FourierBSDFTable &bsdfTable;
    const TransportMode mode;
};

// BSDF Inline Method Definitions
inline int BSDF::NumComponents(BxDFType flags) const {
    int num = 0;
    for (int i = 0; i < nBxDFs; ++i)
        if (bxdfs[i]->MatchesFlags(flags)) ++num;
    return num;
}

}  // namespace pbrt

#endif  // PBRT_CORE_REFLECTION_H
