#pragma once

#include "Mesh.h"
#include "Vector3.h"

class GameObject
{
public:
	GameObject(std::string objPath);
	~GameObject() {}
	GLMmodel* getMesh() {return this->mesh;}
private:
	Vector3 position;
	Vector3 heading;
	GLMmodel* mesh;
	bool visible;
};