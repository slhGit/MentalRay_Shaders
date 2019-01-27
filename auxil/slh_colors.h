#ifndef SLH_COLORS_H
#define SLH_COLORS_H

#include "shader.h"
#include "stringprint.h"
#include <string>

//misc PBRT based ops
inline miColor Sqrt(const miColor& A) {
	return { std::sqrt(A.r), std::sqrt(A.g),  std::sqrt(A.b), 1.0 };
}

inline miColor Abs(const miColor& A) {
	return { std::abs(A.r), std::abs(A.g),  std::abs(A.b), 1.0 };
}

inline std::string MiToString(const miColor& A) {
	return  "[ " + pbrt::StringPrintf("%f", A.r) + ", " + pbrt::StringPrintf("%f", A.r) + ", " + pbrt::StringPrintf("%f", A.r) + " ]";
}

inline bool HasNan(const miColor& A) {
	return isnan(A.r) || isnan(A.g) || isnan(A.b);
}


// miColor x miColor
inline miColor operator+(const miColor& A, const miColor& B) {
	return { A.r + B.r, A.g + B.g, A.b + B.b, 1.f };
}

inline miColor operator-(const miColor& A, const miColor& B) {
	return { A.r - B.r, A.g - B.g, A.b - B.b, 1.f };
}

inline miColor operator*(const miColor& A, const miColor& B) {
	return { A.r * B.r, A.g * B.g, A.b * B.b, 1.f };
}

inline miColor operator/(const miColor& A, const miColor& B) {
	return { A.r / B.r, A.g / B.g, A.b / B.b, 1.f };
}


// templates
template <typename T>
inline miColor operator+(const miColor& A, const T B) {
	return { A.r + B, A.g + B, A.b + B, 1.f };
}

template <typename T>
inline miColor operator-(const miColor& A, const T B) {
	return { A.r - B, A.g - B, A.b - B, 1.f };
}

template <typename T>
inline miColor operator*(const miColor& A, const T B) {
	return { A.r * B, A.g * B, A.b * B, 1.f };
}

template <typename T>
inline miColor operator/(const miColor& A, const T B) {
	miScalar inv = 1. / B;
	return A * inv;
}

template <typename T>
inline miColor operator+(T B, const miColor& A) {
	return A + B;
}

template <typename T>
inline miColor operator-(T B, const miColor& A) {
	return { B - A.r, B - A.g, B - A.b, 1. };
}

template <typename T>
inline miColor operator*(T B, const miColor& A) {
	return A * B;
}

template <typename T>
inline miColor operator/(miScalar B, const miColor& A) {
	return { B / A.r, B / A.g, B / A.b, 1. };
}


// miColor x= miColor
inline miColor& operator+=(miColor& A, const miColor B) {
	A.r += B.r;
	A.g += B.g;
	A.b += B.b;

	return A;
}

inline miColor& operator-=(miColor& A, const miColor B) {
	A.r -= B.r;
	A.g -= B.g;
	A.b -= B.b;

	return A;
}

inline miColor& operator*=(miColor& A, const miColor B) {
	A.r *= B.r;
	A.g *= B.g;
	A.b *= B.b;

	return A;
}

inline miColor& operator/=(miColor& A, const miColor B) {
	A.r /= B.r;
	A.g /= B.g;
	A.b /= B.b;

	return A;
}


// miColor x= miScalar
inline miColor& operator+=(miColor& A, miScalar B) {
	A.r += B;
	A.g += B;
	A.b += B;

	return A;
}

inline miColor& operator-=(miColor& A, miScalar B) {
	A.r -= B;
	A.g -= B;
	A.b -= B;

	return A;
}

inline miColor& operator*=(miColor& A, miScalar B) {
	A.r *= B;
	A.g *= B;
	A.b *= B;

	return A;
}

inline miColor& operator/=(miColor& A, miScalar B) {
	miScalar inv = 1.f / B;
	return A *= inv;

	return A;
}

// equality

inline bool operator!=(const miColor& A, const miColor& B) {
	return A.r != B.r ? true : A.g != B.g ? true : A.b != B.b ? true : false;
}

inline bool operator==(const miColor& A, const miColor& B) {

	return !(A != B);
}

template <typename T>
inline bool operator<(const miColor& A, const T B) {
	return (A.r + A.g + A.b) < B;
}


template <typename T>
inline bool operator<(T B, const miColor& A) {
	return B < (A.r + A.g + A.b);
}


template <typename T>
inline bool operator>(const miColor& A, const T B) {
	return (A.r + A.g + A.b) > B;
}


template <typename T>
inline bool operator>(T B, const miColor& A) {
	return B > (A.r + A.g + A.b);
}




inline std::ostream& operator<<(std::ostream& out, const miColor& A) {
	out << "{ " << A.r << ", " << A.g << ", " << A.b << ", " << A.a << " }";
	return out;
};

#endif