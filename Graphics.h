#pragma once

#define GLEW_STATIC

#include <stdlib.h>
#include <Gl/glew.h>
#include <GL/glut.h>
#include <GL/GLU.h>
#include <vector>
#include <iostream>
#include "GameObject.h"
#include <glm.hpp>
#include <gtc/type_ptr.hpp>

class Graphics
{
public:
	Graphics() {}
	~Graphics() {}
	bool init();
	void initShaders();
	void Graphics::render(std::vector<GameObject*>& objects);
private:
	GLuint meshVAO;
	GLuint vertexShader, fragmentShader;
	GLuint shaderProgram;
	GLuint vtxVBO, normalsVBO;
	char* Graphics::textFileRead(char *fn);
};