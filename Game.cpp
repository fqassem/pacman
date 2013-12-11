
#define GLEW_STATIC

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include "Graphics.h"
#include "Sound.h"
#include "Camera.h"
#include "Pacman.h"
#include "Noms.h"


/* Constants */
Vector3 eye(0, 0, 5);
Vector3 lookAt(0, 0, 1);
const Vector3 up(0, 1, 0);
const float nearZ = 1;
const float farZ = 1000;
int screenWidth = 500;
int screenHeight = 500;
/* End Constants */

Graphics graphics;
Sound sound;
Camera camera;
Pacman pacman;
std::vector<GameObject> objects;
bool gameMap[40][40];

void renderScene()
{
	camera.look();
	camera.setPerspective();
	graphics.render(objects);
	sound.update();
}

void resize(int width, int height)
{
	if(screenHeight == 0)
		screenHeight = 1;

	float ratio = 1.0 * screenWidth / screenHeight;

	// Reset the coordinate system before modifying
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	// Set the viewport to be the entire window
    glViewport(0, 0, screenWidth, screenHeight);

	// Set the correct perspective.
	gluPerspective(45, ratio, 1, 1000);
	glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y) 
{
	if (key == 27) 
		exit(0);
}

void setupCamera()
{
	camera.setLookAt(lookAt);
	camera.setUp(up);
	camera.setPosition(eye);
	camera.setNearZ(nearZ);
	camera.setFarZ(farZ);
	camera.setFOV(45);
}

void setupLights()
{
	GLfloat light_ambient[] = {.5, .5, .5, .5};
	GLfloat light_diffuse[] = {1.0, 1.0, 1.0, 1.0 };
	GLfloat light_specular[] = {0.50, .50, .50, 1.0};
	GLfloat light_position[] = {0, 50, 0, 0 };

	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
}

void initGame()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	objects.push_back(pacman);
}

int main(int argc, char** argr)
{
	glutInit(&argc,argr);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(screenWidth, screenHeight);
	glutInitWindowPosition(50, 50);
	glutCreateWindow("Texture Shell");
	
	if(graphics.init() == false)
		exit(10);
	if(sound.init() == false)
		exit(10);

	sound.playBackgroundMusic("canary.wav");
	// register callbacks
	glutReshapeFunc(resize);
	glutDisplayFunc(renderScene);
	glutIdleFunc(renderScene);
	glutKeyboardFunc(keyboard);

	initGame();
	setupCamera();
	setupLights();

	glutMainLoop();
	return 0;
}