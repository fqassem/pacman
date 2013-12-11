
#define GLEW_STATIC

#include <Windows.h>
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
#include "Wall.h"
#include <iomanip>
#include <glm.hpp>

#define FPS 5

/*timer*/
bool startclock = true;       // True if clock is running
int hour=0, mins=0, sec=0;     // time variables
string timestring="00:00:00";

/* Game specific */
short level = 1;
int nomsCount = 0;
int score = 0;
const int gameMapX = 21; // row #
const int gameMapY = 24; // column #

/* GFX specific */
Vector3 eye(17, 11, 35);
Vector3 lookAt(11, 11, 0);
const Vector3 up(0, 0, 1);
const float nearZ = 1;
const float farZ = 1000;
int screenWidth = 500;
int screenHeight = 500;
/* End Constants */

/* */
bool movingLeft, movingRight, movingUp, movingDown;
Graphics graphics;
Sound sound;
Camera camera;
Pacman* pacman = new Pacman();
std::vector<GameObject*> objects;
GameObject* gameMap[gameMapX][gameMapY];
void readLevel();
void move();

void renderScene()
{
	camera.look();
	camera.setPerspective();
	move();
	graphics.render(objects);
	sound.update();
}

void myTimer(int v)     // timer function to update time
{
    ostringstream timestream;      // used to format time string
    if (startclock) sec++;         // increment second
    if (sec==60) { mins++; sec=0; } // increment minute
    if (mins==60) { hour=hour++ % 24; mins=0; } //increment hour
    timestream << std::setfill('0') << std::setw(2) << hour <<":" << std::setw(2)
               << mins << ":" << setw(2) << sec;
    timestring=timestream.str();  //convert stream to string
    glutPostRedisplay();
    glutTimerFunc(1000/v, myTimer, v);  //repost timer 
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

void move()
{
	if(movingUp)
	{
		if(dynamic_cast<Wall*>(gameMap[(int)pacman->getX() - 1][(int)pacman->getY()]) == NULL)
			{
				if(dynamic_cast<Noms*>(gameMap[(int)pacman->getX() - 1][(int)pacman->getY()]) != NULL)
				{
					if(dynamic_cast<Noms*>(gameMap[(int)pacman->getX() - 1][(int)pacman->getY()])->getVisible() == true)
					{
						score += 10;
						nomsCount--;
						//sound.playSound("eat.wav", 0);
						gameMap[(int)pacman->getX() - 1][(int)pacman->getY()]->setVisible(false);
					}
				}
				pacman->setX(pacman->getX() - 1);
			}
	}
	else if(movingDown)
	{
		if(dynamic_cast<Wall*>(gameMap[(int)pacman->getX() + 1][(int)pacman->getY()]) == NULL)
			{
				if(dynamic_cast<Noms*>(gameMap[(int)pacman->getX() + 1][(int)pacman->getY()]) != NULL)
				{
					if(dynamic_cast<Noms*>(gameMap[(int)pacman->getX() + 1][(int)pacman->getY()])->getVisible() == true)
					{
						score += 10;
						nomsCount--;
						//sound.playSound("eat.wav", 0);
						gameMap[(int)pacman->getX() +  1][(int)pacman->getY()]->setVisible(false);
					}
				}
				pacman->setX(pacman->getX() + 1);
			}
	}
	else if(movingLeft)
	{
		if(dynamic_cast<Wall*>(gameMap[(int)pacman->getX()][(int)pacman->getY() - 1]) == NULL)
		{
			if(dynamic_cast<Noms*>(gameMap[(int)pacman->getX()][(int)pacman->getY() - 1]) != NULL)
			{
				if(dynamic_cast<Noms*>(gameMap[(int)pacman->getX()][(int)pacman->getY() - 1])->getVisible() == true)
				{
					score += 10;
					nomsCount--;
					//sound.playSound("eat.wav", 0);
					gameMap[(int)pacman->getX()][(int)pacman->getY() - 1]->setVisible(false);
				}
			}
			pacman->setY(pacman->getY() - 1);
		}
	}
	else if(movingRight)
	{
		if(dynamic_cast<Wall*>(gameMap[(int)pacman->getX()][(int)pacman->getY() + 1]) == NULL)
		{
			if(dynamic_cast<Noms*>(gameMap[(int)pacman->getX()][(int)pacman->getY() + 1]) != NULL)
			{
				if(dynamic_cast<Noms*>(gameMap[(int)pacman->getX()][(int)pacman->getY() + 1])->getVisible() == true)
				{
					score += 10;
					nomsCount--;
					//sound.playSound("eat.wav", 0);
					gameMap[(int)pacman->getX()][(int)pacman->getY() + 1]->setVisible(false);
				}
			}
			pacman->setY(pacman->getY() + 1);
		}
	}
	std::cout << nomsCount << std::endl;
	if(nomsCount == 0)
	{
		level++;
		readLevel();
	}
}

void special_keyboard(int key, int x, int y)
{
	switch (key) 
	{
		case GLUT_KEY_UP:
			movingUp = true;
			movingLeft = movingRight = movingDown = false;
			break;
		case GLUT_KEY_DOWN:
			movingDown = true;
			movingLeft = movingUp = movingRight = false;
			break;
		case GLUT_KEY_LEFT:
			movingLeft = true;
			movingRight = movingUp = movingDown = false;
			break;
		case GLUT_KEY_RIGHT:
			movingRight = true;
			movingLeft = movingUp = movingDown = false;
			break;
		default:
			break;
	}
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
	readLevel();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	objects.push_back(pacman);
	for(int i = 0; i < gameMapX; i++)
	{
		for(int j = 0; j < gameMapY; j++)
		{
			objects.push_back(gameMap[i][j]);
		}
	}
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

	//sound.playBackgroundMusic("canary.wav");
	// register callbacks
	glutReshapeFunc(resize);
	glutDisplayFunc(renderScene);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special_keyboard);
	glutTimerFunc(1000,myTimer,FPS);


	initGame();
	setupCamera();
	setupLights();
	glutMainLoop();
	return 0;
}


/*
* Defines game map
* "x" is a wall
* " " is a space
* "." is a nom, rep'd as '1' in memory
*/
void readLevel()
{
	char* fileName;
	switch(level)
	{
	case 1:
		fileName = "level1.txt";
		break;
	case 2:
		MessageBox(NULL, "You win!", "Game Over", NULL);
		exit(10);
		break;
	}
	ifstream levelFile(fileName);
	if(levelFile.is_open())
	{
		string line;
		for (int i = 0; levelFile.good() && i < gameMapX; i++)
		{
			levelFile >> line;
			for(int j = 0; j < line.length(); j++){
				switch (line.at(j))
				{
					case 'x' : 
						gameMap[i][j] = new Wall(i, j);
						break;
					case ' ' :
						gameMap[i][j] = NULL;
						break;
					case '.' :
						gameMap[i][j] = new Noms(i, j);
						nomsCount++;
						break;
					case '0':
						gameMap[i][j] = NULL;
						pacman->setPosition(Vector3(i, j, 0));
						break;
				}				
			}
		}
	}
	std::cout << nomsCount << std::endl;
	movingLeft = movingRight = movingUp = movingDown = false;
	sound.playSound("sounds/pacmanIntro.wav", 0);
}