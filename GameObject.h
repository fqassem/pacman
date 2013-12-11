#pragma once

#include "Mesh.h"
#include "Vector3.h"

class GameObject
{
public:
	GameObject(std::string objPath);
	virtual ~GameObject() {}
	virtual GLMmodel* getMesh() {return this->mesh;}
	virtual float getX() {return position.x;}
	virtual float getY() {return position.y;}
	virtual float getZ() {return position.z;}
	void setX(float x) {position.x = x;}
	virtual void setY(float y) {position.y = y;}
	virtual void setZ(float z) {position.z = z;}
	virtual void setPosition(Vector3 pos) {this->position = pos;}
	virtual void setVisible(bool vis) {this->visible = vis;}
	virtual bool getVisible() {return this->visible;}
private:
	Vector3 position;
	Vector3 heading;
	GLMmodel* mesh;
	bool visible;
};