#include "Graphics.h"

bool Graphics::init()
{
	GLenum err;
	if ((err = glewInit()) != GLEW_OK)
	{
        std::cout << "GLEW not initalized" << std::endl;
		return false;
	}

	if (glewIsSupported("GL_VERSION_2_0"))
		printf("Ready for OpenGL 2.0\n");
	else 
	{
		printf("OpenGL 2.0 not supported\n");
		return false;
	}

	glClearColor(0, 0, 0, 0);
	initShaders();
	return true;
}

void Graphics::initShaders()
{
	GLenum ErrorCheckValue = glGetError();

	const char *vsSource = textFileRead("shaders/vertShader.vert");
	const char *fsSource = textFileRead("shaders/fragShader.frag");

	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vsSource, NULL);
	glCompileShader(vertexShader);

	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fsSource, NULL);
	glCompileShader(fragmentShader);

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);

	ErrorCheckValue = glGetError();
	if (ErrorCheckValue != GL_NO_ERROR)
	{
		fprintf(
			stderr,
			"ERROR: Could not create the shaders: %s \n",
			gluErrorString(ErrorCheckValue)
		);

		exit(-1);
	}
}

void Graphics::render(std::vector<GameObject*>& objects)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgram);
	GLMtriangle* triangle;
	GLMgroup* group;
	GLMmaterial* material;
	GLfloat* uv1;
	GLfloat* uv2;
	GLfloat* uv3;
	GLMmodel* mesh;
	for (std::vector<GameObject*>::iterator it = objects.begin() ; it != objects.end(); ++it)
	{
		if((*it) == NULL)
			continue;
		if((*it)->getVisible() == false)
			continue;
		glPushMatrix();
		glTranslatef((*it)->getX(), (*it)->getY(), (*it)->getZ());
		glBegin(GL_TRIANGLES);
		mesh = (*it)->getMesh();
		group = mesh->groups;
		while (group) 
		{
			material = &mesh->materials[group->material];
			glColor4fv(material->diffuse);
			glMaterialfv(GL_FRONT, GL_AMBIENT, material->ambient);
			glMaterialfv(GL_FRONT, GL_DIFFUSE, material->diffuse);
			glMaterialfv(GL_FRONT, GL_SPECULAR, material->specular);
			glMaterialf(GL_FRONT, GL_SHININESS, material->shininess);
			for(int i = 0; i < group->numtriangles; i++)
			{
				triangle = &mesh->triangles[group->triangles[i]];
				uv1 = &mesh->vertices.at(3 * triangle->vindices[0]);
				uv2 = &mesh->vertices.at(3 * triangle->vindices[1]);
				uv3 = &mesh->vertices.at(3 * triangle->vindices[2]);

				glNormal3fv(&mesh->normals[3 * triangle->nindices[0]]);
				glTexCoord2f(uv1[0] + .5, uv1[1] - .5);
				glVertex3fv(&mesh->vertices.at(3 * triangle->vindices[0]));
			
				glNormal3fv(&mesh->normals[3 * triangle->nindices[1]]);
				glTexCoord2f(uv2[0] + .5, uv2[1] - .5);
				glVertex3fv(&mesh->vertices.at(3 * triangle->vindices[1]));

				glNormal3fv(&mesh->normals[3 * triangle->nindices[2]]);
				glTexCoord2f(uv3[0] + .5, uv3[1] - .5);
				glVertex3fv(&mesh->vertices.at(3 * triangle->vindices[2]));
			}
			group = group->next;
		}
		glEnd();
		glPopMatrix();
	}
	glutSwapBuffers();
}

char* Graphics::textFileRead(char *fn)
{

	FILE *fp;
	char *content = NULL;

	int count=0;

	if (fn != NULL) {
		fp = fopen(fn,"rt");

		if (fp != NULL) {
      
      fseek(fp, 0, SEEK_END);
      count = ftell(fp);
      rewind(fp);

			if (count > 0) {
				content = (char *)malloc(sizeof(char) * (count+1));
				count = fread(content,sizeof(char),count,fp);
				content[count] = '\0';
			}
			fclose(fp);
		}
	}
	return content;
}