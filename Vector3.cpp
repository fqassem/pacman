#include "Vector3.h"

Vector3 operator + (const Vector3& v1, const Vector3& v2) { return Vector3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z); }
Vector3 operator - (const Vector3& v1, const Vector3& v2) { return Vector3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z); }
Vector3 operator * (const Vector3& v1, const Vector3& v2); //per component multiplcation
Vector3 operator * (const Vector3& v1, float f) { return Vector3(v1.x *f, v1.y * f, v1.z * f); }
Vector3 operator * (double f, const Vector3& v1) { return Vector3(v1.x *f, v1.y * f, v1.z * f); }
Vector3 operator / (const Vector3& v1, float f) { return Vector3(v1.x / f, v1.y / f, v1.z / f); }