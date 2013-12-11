/*
	  Kalpesh Padia, Oct 2012, NCSU
      C++ port of portions of glm.
	  Few portions are retained in C.
	  Most datastructure and other variable names are same/similar.  

	  This port written by Kalpesh Padia
	  kpadia@ncsu.edu
	  glm originally written by Nate Robins 
	  nate@pobox.com http://user.xmission.com/~nate/index.html
*/
#pragma once
#include <stdlib.h>
#include "GL/glut.h"
#include <string>
#include <vector>
using namespace std;

#ifndef M_PI
#define M_PI 3.14159265f
#endif

#define GLM_NONE     (0)            /* render with only vertices */
#define GLM_FLAT     (1 << 0)       /* render with facet normals */
#define GLM_SMOOTH   (1 << 1)       /* render with vertex normals */
#define GLM_TEXTURE  (1 << 2)       /* render with texture coords */
#define GLM_COLOR    (1 << 3)       /* render with colors */
#define GLM_MATERIAL (1 << 4)       /* render with materials */


/* GLMmaterial: Structure that defines a material in a model.
 */
struct GLMmaterial
{
  string			name;                 /* name of material */
  GLfloat			diffuse[4];           /* diffuse component */
  GLfloat			ambient[4];           /* ambient component */
  GLfloat			specular[4];          /* specular component */
  GLfloat			emmissive[4];         /* emmissive component */
  GLfloat			shininess;            /* specular exponent */
};

/* GLMtriangle: Structure that defines a triangle in a model.
 */
struct GLMtriangle {
  GLuint			vindices[3];           /* array of triangle vertex indices */
  GLuint			nindices[3];           /* array of triangle normal indices */
  GLuint			tindices[3];           /* array of triangle texcoord indices*/
  GLuint			findex;                /* index of triangle facet normal */
};

/* GLMgroup: Structure that defines a group in a model.
 */
struct GLMgroup {
  string            name;           /* name of this group */
  GLuint            numtriangles;   /* number of triangles in this group */
  vector<GLuint>    triangles;      /* vector of triangle indices */
  GLuint            material;       /* index to material for group */
  GLMgroup* next;					/* pointer to next group in model */
};

/* GLMmodel: Structure that defines a model.
 */
struct GLMmodel {
  string			pathname;            /* path to this model */
  string			mtllibname;          /* name of the material library */

  GLuint			numvertices;         /* number of vertices in model */
  vector<GLfloat>	vertices;            /* vector of vertices  */

  GLuint			numnormals;          /* number of normals in model */
  vector<GLfloat>	normals;             /* vector of normals */

  GLuint			numtexcoords;        /* number of texcoords in model */
  vector<GLfloat>   texcoords;           /* vector of texture coordinates */

  GLuint			numfacetnorms;       /* number of facetnorms in model */
  vector<GLfloat>	facetnorms;          /* vector of facetnorms */

  GLuint			numtriangles;		/* number of triangles in model */
  vector<GLMtriangle> triangles;		/* vector of triangles */

  GLuint			nummaterials;		/* number of materials in model */
  vector<GLMmaterial> materials;		/* vector of materials */

  GLuint		    numgroups;			/* number of groups in model */
  GLMgroup*			groups;				/* linked list of groups */

  GLfloat			position[3];          /* position of the model */

};


/* glmUnitize: "unitize" a model by translating it to the origin and
 * scaling it to fit in a unit cube around the origin.  Returns the
 * scalefactor used.
 *
 * model - properly initialized GLMmodel structure
 */
GLfloat glmUnitize(GLMmodel* model);

/* glmFacetNormals: Generates facet normals for a model (by taking the
 * cross product of the two vectors derived from the sides of each
 * triangle).  Assumes a counter-clockwise winding.
 *
 * model - initialized GLMmodel structure
 */
GLvoid glmFacetNormals(GLMmodel* model);

/* glmVertexNormals: Generates smooth vertex normals for a model.
 *
 * model - initialized GLMmodel structure
 * angle - maximum angle (in degrees) to smooth across
 */
GLvoid glmVertexNormals(GLMmodel* model, GLfloat angle);

/* glmReadOBJ: Reads a model description from a Wavefront .OBJ file.
 *
 * filename - name of the file containing the Wavefront .OBJ format data.
 */
GLMmodel* glmReadOBJ(string filename);

/* glmDraw: Renders the model to the current OpenGL context using the
 * mode specified.
 *
 * model    - initialized GLMmodel structure
 * mode     - a bitwise OR of values describing what is to be rendered.
 *            GLM_NONE    -  render with only vertices
 *            GLM_FLAT    -  render with facet normals
 *            GLM_SMOOTH  -  render with vertex normals
 *            GLM_TEXTURE -  render with texture coords
 *            GLM_FLAT and GLM_SMOOTH should not both be specified.
 */
GLvoid glmDraw(GLMmodel* model, GLuint mode);