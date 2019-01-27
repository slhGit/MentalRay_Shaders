#ifndef SLH_VECTORS_H
#define SLH_VECTORS_H

#include "shader.h"
#include <iostream>

// Operators
inline miVector operator-(const miVector& A) { return { -A.x, -A.y, -A.z }; }

inline miVector operator+(const miVector &A, const miVector &B) { return { A.x + B.x, A.y + B.y, A.z + B.z }; }
inline miVector operator-(const miVector &A, const miVector &B) { return { A.x - B.x, A.y - B.y, A.z - B.z }; }

inline miVector operator*(const miVector &A, miScalar B) { return { A.x * B, A.y * B, A.z * B }; }
inline miVector operator*(miScalar B, const miVector &A) { return A * B; }

inline miVector operator/(const miVector &A, miScalar B) { miScalar inv = 1.0 / B; return A * inv; }
inline miVector operator/(miScalar B, const miVector& A) { return A / B; }


inline miVector& operator*=(miVector& A, miScalar B) { A.x *= B; A.y *= B; A.z *= B; return A; }
inline miVector& operator/=(miVector& A, miScalar B) { miScalar inv = 1.0 / B; return A *= inv; }


inline bool operator==(const miVector &A, const miVector &B) { return A.x == B.x && A.y == B.y && A.z == B.z; }
inline bool operator!=(const miVector &A, const miVector &B) { return !(A == B); }


inline std::ostream& operator<<(std::ostream &out, const miVector &A) {
	out << "[ " << A.x << ", " << A.y << ", " << A.z << " ]";

	return out;
}


// Misc vector functions
inline miVector Abs(const miVector& A) {
	return { std::abs(A.x), std::abs(A.y),std::abs(A.z) };
}
inline miScalar Dot(const miVector& A, const miVector& B) {
	return  A.x*B.x + A.y*B.y + A.z*B.z;
}

inline miScalar Magnitude(const miVector& A) {
	return sqrt((A.x*A.x) + (A.y*A.y) + (A.z*A.z));
}
inline miVector Normalize(const miVector& A) {
	return A / Magnitude(A);
}

inline miScalar AbsDot(const miVector& A, const miVector& B) {
	return fabs(Dot(A, B));
}

inline miVector Cross(const miVector& A, const miVector& B) {
	miScalar v1x = A.x, v1y = A.y, v1z = A.z;
	miScalar v2x = B.x, v2y = B.y, v2z = B.z;

	return { (v1y * v2z) - (v1z * v2y), (v1z * v2x) - (v1x * v2z), (v1x * v2y) - (v1y * v2x) };
}

template <typename T>
T MinComponent(const miVector &v) {
	return std::min(v.x, std::min(v.y, v.z));
}

#endif