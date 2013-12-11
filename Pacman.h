#pragma once

#include "GameObject.h"

class Pacman : public GameObject
{
public:
	Pacman() : GameObject("meshes/al.obj") {}
	~Pacman() {}
private:
	bool alive;
};