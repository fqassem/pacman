#include "GameObject.h"

GameObject::GameObject(std::string objPath)
{
	this->mesh = glmReadOBJ(objPath);
	//glmUnitize(mesh);
	glmFacetNormals(mesh);
	glmVertexNormals(mesh, 89);
	this->heading = Vector3(0, 0, 0);
	this->position = Vector3(0, 0, 0);
	this->visible = true;
}
