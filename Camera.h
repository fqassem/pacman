#pragma once

#include "Vector3.h"

class Camera
{
public:
	Camera() {}
	~Camera() {}
	void setUp(Vector3 up) {this->up = up;}
	void setUp(float upX, float upY, float upZ) {this->up = Vector3(upX, upY, upZ);}
	void setLookAt(Vector3 lookAt) {this->lookAt = lookAt;}
	void setLookAt(float lookAtX, float lookAtY, float lookAtZ) {this->lookAt = Vector3(lookAtX, lookAtY, lookAtZ);}
	void setPosition(Vector3 position) {this->position = position;}
	void setPosition(float posX, float posY, float posZ) {this->position = Vector3(posX, posY, posZ);}
	void setFOV(float FOV) {this->fov = FOV;}
	void setAspectRatio(float aspectRatio) {this->aspectRatio = aspectRatio;}
	void setNearZ(float nearZ) {this->nearZ = nearZ;}
	void setFarZ(float farZ) {this->farZ = farZ;}

	void look() {glLoadIdentity(); gluLookAt(position.x, position.y, position.z, lookAt.x, lookAt.y, lookAt.z, up.x, up.y, up.z);}
	void setPerspective() {gluPerspective(fov, aspectRatio, nearZ, farZ);}
private:
	Vector3 up;
	Vector3 lookAt;
	Vector3 center;
	Vector3 position;
	
	float fov;
	float aspectRatio;
	float nearZ;
	float farZ;
};
