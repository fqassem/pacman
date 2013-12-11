#pragma once

#include "GameObject.h"

class Noms : public GameObject
{
public:
	Noms(int i, int j) : GameObject("meshes/nom.obj") {this->setX(i); this->setY(j);}
	~Noms() {}
};