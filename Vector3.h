#pragma once

#include <cmath>
#include <iostream>

class Vector3
{
public:
	double x, y, z;
	Vector3() { x = 0; y = 0; z = 0;}
	Vector3(double fx, double fy, double fz) { x = fx; y = fy; z = fz;}	
	Vector3 cross(const Vector3& v) const { return Vector3( y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);}
	void cross(const Vector3& v, Vector3& result) const;
		//vector operations
	double length() const {return sqrt(lengthSquared());}
	double lengthSquared() const { return x * x + y * y + z * z;}
	void normalize() { double f = length(); x /= f; y /= f; z /= f;}
	double dot(const Vector3& v) const { return x * v.x + y * v.y + z * v.z; }
	void toString() { std::cout << x << "," << y << "," << z << std::endl;}

};// class definition
