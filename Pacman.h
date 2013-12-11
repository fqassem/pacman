#pragma once

#include "GameObject.h"

class Pacman : public GameObject
{
public:
	Pacman() : GameObject("meshes/pacman.obj") {}
	~Pacman() {}
private:
	bool alive;
};