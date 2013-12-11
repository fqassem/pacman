#pragma once

#include "GameObject.h"

class Wall : public GameObject
{
public:
	Wall(int i, int j) : GameObject("meshes/wall.obj") {this->setX(i); this->setY(j);}
	~Wall() {}
};