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


#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <iostream>
#include "mesh.h"

using namespace std;

#define T(x) (model->triangles[(x)])


/* GLMnode: general purpose node */
struct GLMnode {
    GLuint index;
    GLboolean averaged;
    GLMnode* next;
};


/* glmMax: returns the maximum of two floats */
static GLfloat glmMax(GLfloat a, GLfloat b)
{
    if (b > a)
        return b;
    return a;
}

/* glmAbs: returns the absolute value of a float */
static GLfloat glmAbs(GLfloat f)
{
    if (f < 0)
        return -f;
    return f;
}

/* glmDot: compute the dot product of two vectors
 *
 * u - array of 3 GLfloats (GLfloat u[3])
 * v - array of 3 GLfloats (GLfloat v[3])
 */
static GLfloat glmDot(GLfloat* u, GLfloat* v)
{
    assert(u); 
	assert(v);

    return u[0]*v[0] + u[1]*v[1] + u[2]*v[2];
}

static GLfloat glmDot_v(vector<GLfloat> u, vector<GLfloat> v)
{
    assert(!u.empty()); 
	assert(!v.empty());

    return u[0]*v[0] + u[1]*v[1] + u[2]*v[2];
}

/* glmCross: compute the cross product of two vectors
 *
 * u - array of 3 GLfloats (GLfloat u[3])
 * v - array of 3 GLfloats (GLfloat v[3])
 * n - vector of 3 GLfloats (GLfloat n[3]) to return the cross product in
 */
vector<GLfloat> glmCross(GLfloat* u, GLfloat* v)
{
	vector<GLfloat> n;
	assert(u); assert(v); assert(n.empty());

	n.push_back(u[1]*v[2] - u[2]*v[1]);
	n.push_back(u[2]*v[0] - u[0]*v[2]);
	n.push_back(u[0]*v[1] - u[1]*v[0]);

	return n;
}

vector<GLfloat> glmCross_v(vector<GLfloat> u, vector<GLfloat> v)
{
	vector<GLfloat> n;
	assert(!u.empty()); assert(!v.empty()); assert(n.empty());

	n.push_back(u[1]*v[2] - u[2]*v[1]);
	n.push_back(u[2]*v[0] - u[0]*v[2]);
	n.push_back(u[0]*v[1] - u[1]*v[0]);

	return n;
}

/* glmNormalize: normalize a vector
 *
 * v - array of 3 GLfloats (GLfloat v[3]) to be normalized
 */
static GLvoid glmNormalize(GLfloat* v)
{
    GLfloat l;

    assert(v);

    l = (GLfloat)sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    v[0] /= l;
    v[1] /= l;
    v[2] /= l;
}

/* glmEqual: compares two vectors and returns GL_TRUE if they are
 * equal (within a certain threshold) or GL_FALSE if not. An epsilon
 * that works fairly well is 0.000001.
 *
 * u - array of 3 GLfloats (GLfloat u[3])
 * v - array of 3 GLfloats (GLfloat v[3])
 */
static GLboolean glmEqual(GLfloat* u, GLfloat* v, GLfloat epsilon)
{
    if (glmAbs(u[0] - v[0]) < epsilon &&
        glmAbs(u[1] - v[1]) < epsilon &&
        glmAbs(u[2] - v[2]) < epsilon)
    {
        return GL_TRUE;
    }
    return GL_FALSE;
}

/* glmFindGroup: Find a group in the model */
GLMgroup* glmFindGroup(GLMmodel* model, string name)
{
    GLMgroup* group;

    assert(model);

    group = model->groups;
    while(group) {
        if (name == group->name)
            break;
        group = group->next;
    }

    return group;
}

/* glmAddGroup: Add a group to the model */
GLMgroup* glmAddGroup(GLMmodel* model, string name)
{
    GLMgroup* group;

    group = glmFindGroup(model, name);
    if (!group) {
        group = new GLMgroup();
        group->name = name;
        group->material = 0;
        group->numtriangles = 0;
        group->next = model->groups;
        model->groups = group;
        model->numgroups++;
    }

    return group;
}

/* glmFindGroup: Find a material in the model */
GLuint glmFindMaterial(GLMmodel* model, string name)
{
    GLuint i;

    /* XXX doing a linear search on a string key'd list is pretty lame,
    but it works and is fast enough for now. */
    for (i = 0; i < model->nummaterials; i++) {
        if (model->materials[i].name == name)
            return i;
    }

    /* didn't find the name, so print a warning and return the default
    material (0). */
    cout<<"glmFindMaterial():  can't find material \""<<name<<"\"."<<endl;
    return 0;
}


/* glmDirName: return the directory given a path
 *
 * path - filesystem path
 *
 * NOTE: the return value should be free'd.
 */
static string
glmDirName(string path)
{
    return path.substr(0,path.find_last_of("/\\"));
}


/* glmReadMTL: read a wavefront material library file
 *
 * model - properly initialized GLMmodel structure
 * name  - name of the material library
 */
static GLvoid glmReadMTL(GLMmodel* model, string name)
{
    FILE* file;
    string dir;
    string filename;
    char buf[128];
    GLuint nummaterials, i;

    dir = glmDirName(model->pathname);
    filename = dir + "\\" + name;

    file = fopen(filename.c_str(), "r");
    if (!file) {
        cerr<<"glmReadMTL() failed: can't open material file \""<<filename<<"\""<<endl;
        exit(1);
    }


    /* count the number of materials in the file */
    nummaterials = 1;
    while(fscanf(file, "%s", buf) != EOF) {
        switch(buf[0]) {
        case '#':               /* comment */
            /* eat up rest of line */
            fgets(buf, sizeof(buf), file);
            break;
        case 'n':               /* newmtl */
            fgets(buf, sizeof(buf), file);
            nummaterials++;
            sscanf(buf, "%s %s", buf, buf);
            break;
        default:
            /* eat up rest of line */
            fgets(buf, sizeof(buf), file);
            break;
        }
    }

    rewind(file);

	model->materials.clear();
    model->nummaterials = nummaterials;

    /* set the default material */
    for (i = 0; i < nummaterials; i++) {
		GLMmaterial material;
		model->materials.push_back(material);
        model->materials[i].name = "";
        model->materials[i].shininess = 65.0;
        model->materials[i].diffuse[0] = 0.8;
        model->materials[i].diffuse[1] = 0.8;
        model->materials[i].diffuse[2] = 0.8;
        model->materials[i].diffuse[3] = 1.0;
        model->materials[i].ambient[0] = 0.2;
        model->materials[i].ambient[1] = 0.2;
        model->materials[i].ambient[2] = 0.2;
        model->materials[i].ambient[3] = 1.0;
        model->materials[i].specular[0] = 0.0;
        model->materials[i].specular[1] = 0.0;
        model->materials[i].specular[2] = 0.0;
        model->materials[i].specular[3] = 1.0;
    }
    model->materials[0].name = "default";

    /* now, read in the data */
    nummaterials = 0;
    while(fscanf(file, "%s", buf) != EOF) {
        switch(buf[0]) {
        case '#':               /* comment */
            /* eat up rest of line */
            fgets(buf, sizeof(buf), file);
            break;
        case 'n':               /* newmtl */
            fgets(buf, sizeof(buf), file);
            sscanf(buf, "%s %s", buf, buf);
            nummaterials++;
            model->materials[nummaterials].name = string(buf);
            break;
        case 'N':
            fscanf(file, "%f", &model->materials[nummaterials].shininess);
            /* wavefront shininess is from [0, 1000], so scale for OpenGL */
            model->materials[nummaterials].shininess /= 1000.0;
            model->materials[nummaterials].shininess *= 128.0;
            break;
        case 'K':
            switch(buf[1]) {
            case 'd':
                fscanf(file, "%f %f %f",
                    &model->materials[nummaterials].diffuse[0],
                    &model->materials[nummaterials].diffuse[1],
                    &model->materials[nummaterials].diffuse[2]);
                break;
            case 's':
                fscanf(file, "%f %f %f",
                    &model->materials[nummaterials].specular[0],
                    &model->materials[nummaterials].specular[1],
                    &model->materials[nummaterials].specular[2]);
                break;
            case 'a':
                fscanf(file, "%f %f %f",
                    &model->materials[nummaterials].ambient[0],
                    &model->materials[nummaterials].ambient[1],
                    &model->materials[nummaterials].ambient[2]);
                break;
            default:
                /* eat up rest of line */
                fgets(buf, sizeof(buf), file);
                break;
            }
            break;
            default:
                /* eat up rest of line */
                fgets(buf, sizeof(buf), file);
                break;
        }
    }
}

/* glmFirstPass: first pass at a Wavefront OBJ file that gets all the
 * statistics of the model (such as #vertices, #normals, etc)
 *
 * model - properly initialized GLMmodel structure
 * file  - (fopen'd) file descriptor
 */
static GLvoid glmFirstPass(GLMmodel* model, FILE* file)
{
    GLuint numvertices;        /* number of vertices in model */
    GLuint numnormals;         /* number of normals in model */
    GLuint numtexcoords;       /* number of texcoords in model */
    GLuint numtriangles;       /* number of triangles in model */
    GLMgroup* group;           /* current group */
    int v, n, t;
    char buf[128];

    /* make a default group */
    group = glmAddGroup(model, "default");

    numvertices = numnormals = numtexcoords = numtriangles = 0;
    while(fscanf(file, "%s", buf) != EOF) {
        switch(buf[0]) {
        case '#':               /* comment */
            /* eat up rest of line */
            fgets(buf, sizeof(buf), file);
            break;
        case 'v':               /* v, vn, vt */
            switch(buf[1]) {
            case '\0':          /* vertex */
                /* eat up rest of line */
                fgets(buf, sizeof(buf), file);
                numvertices++;
                break;
            case 'n':           /* normal */
                /* eat up rest of line */
                fgets(buf, sizeof(buf), file);
                numnormals++;
                break;
            case 't':           /* texcoord */
                /* eat up rest of line */
                fgets(buf, sizeof(buf), file);
                numtexcoords++;
                break;
            default:
                printf("glmFirstPass(): Unknown token \"%s\".\n", buf);
                exit(1);
                break;
            }
            break;
            case 'm':
                fgets(buf, sizeof(buf), file);
                sscanf(buf, "%s %s", buf, buf);
                model->mtllibname = strdup(buf);
                glmReadMTL(model, buf);
                break;
            case 'u':
                /* eat up rest of line */
                fgets(buf, sizeof(buf), file);
                break;
            case 'g':               /* group */
                /* eat up rest of line */
                fgets(buf, sizeof(buf), file);
#if SINGLE_STRING_GROUP_NAMES
                sscanf(buf, "%s", buf);
#else
                buf[strlen(buf)-1] = '\0';  /* nuke '\n' */
#endif
                group = glmAddGroup(model, buf);
                break;
            case 'f':               /* face */
                v = n = t = 0;
                fscanf(file, "%s", buf);
                /* can be one of %d, %d//%d, %d/%d, %d/%d/%d %d//%d */
                if (strstr(buf, "//")) {
                    /* v//n */
                    sscanf(buf, "%d//%d", &v, &n);
                    fscanf(file, "%d//%d", &v, &n);
                    fscanf(file, "%d//%d", &v, &n);
                    numtriangles++;
                    group->numtriangles++;
                    while(fscanf(file, "%d//%d", &v, &n) > 0) {
                        numtriangles++;
                        group->numtriangles++;
                    }
                } else if (sscanf(buf, "%d/%d/%d", &v, &t, &n) == 3) {
                    /* v/t/n */
                    fscanf(file, "%d/%d/%d", &v, &t, &n);
                    fscanf(file, "%d/%d/%d", &v, &t, &n);
                    numtriangles++;
                    group->numtriangles++;
                    while(fscanf(file, "%d/%d/%d", &v, &t, &n) > 0) {
                        numtriangles++;
                        group->numtriangles++;
                    }
                } else if (sscanf(buf, "%d/%d", &v, &t) == 2) {
                    /* v/t */
                    fscanf(file, "%d/%d", &v, &t);
                    fscanf(file, "%d/%d", &v, &t);
                    numtriangles++;
                    group->numtriangles++;
                    while(fscanf(file, "%d/%d", &v, &t) > 0) {
                        numtriangles++;
                        group->numtriangles++;
                    }
                } else {
                    /* v */
                    fscanf(file, "%d", &v);
                    fscanf(file, "%d", &v);
                    numtriangles++;
                    group->numtriangles++;
                    while(fscanf(file, "%d", &v) > 0) {
                        numtriangles++;
                        group->numtriangles++;
                    }
                }
                break;

            default:
                /* eat up rest of line */
                fgets(buf, sizeof(buf), file);
                break;
        }
  }

  /* set the stats in the model structure */
  model->numvertices  = numvertices;
  model->numnormals   = numnormals;
  model->numtexcoords = numtexcoords;
  model->numtriangles = numtriangles;

  /* allocate memory for the triangles in each group */
  group = model->groups;
  while(group) {
      //group->triangles = new GLuint[group->numtriangles];
      group->numtriangles = 0;
      group = group->next;
  }
}

/* glmSecondPass: second pass at a Wavefront OBJ file that gets all
 * the data.
 *
 * model - properly initialized GLMmodel structure
 * file  - (fopen'd) file descriptor
 */
static GLvoid glmSecondPass(GLMmodel* model, FILE* file)
{
    GLuint numvertices;        /* number of vertices in model */
    GLuint numnormals;         /* number of normals in model */
    GLuint numtexcoords;       /* number of texcoords in model */
    GLuint numtriangles;       /* number of triangles in model */
    GLMgroup* group;           /* current group pointer */
    GLuint material;           /* current material */
    int v, n, t;
    char buf[128];

    /* set the pointer shortcuts */
    group      = model->groups;

    /* on the second pass through the file, read all the data into the
    allocated arrays */
    numvertices = numnormals = numtexcoords = 1;
    numtriangles = 0;
    material = 0;
    while(fscanf(file, "%s", buf) != EOF) {
        float f1, f2, f3;
		switch(buf[0]) {
        case '#':               /* comment */
            /* eat up rest of line */
            fgets(buf, sizeof(buf), file);
            break;
        case 'v':               /* v, vn, vt */
            switch(buf[1]) {
            case '\0':          /* vertex */
                fscanf(file, "%f %f %f", &f1, &f2, &f3);
				model->vertices.push_back(f1);
				model->vertices.push_back(f2);
				model->vertices.push_back(f3);
                numvertices++;
                break;
            case 'n':           /* normal */
                fscanf(file, "%f %f %f", &f1, &f2, &f3);
				model->normals.push_back(f1);
				model->normals.push_back(f2);
				model->normals.push_back(f3);
                numnormals++;
                break;
            case 't':           /* texcoord */
                fscanf(file, "%f %f %f", &f1, &f2, &f3);
				model->texcoords.push_back(f1);
				model->texcoords.push_back(f2);
                numtexcoords++;
                break;
            }
            break;
            case 'u':
                fgets(buf, sizeof(buf), file);
                sscanf(buf, "%s %s", buf, buf);
                group->material = material = glmFindMaterial(model, buf);
                break;
            case 'g':               /* group */
                /* eat up rest of line */
                fgets(buf, sizeof(buf), file);
#if SINGLE_STRING_GROUP_NAMES
                sscanf(buf, "%s", buf);
#else
                buf[strlen(buf)-1] = '\0';  /* nuke '\n' */
#endif
                group = glmFindGroup(model, buf);
                group->material = material;
                break;
            case 'f':               /* face */
                v = n = t = 0;
                fscanf(file, "%s", buf);
                /* can be one of %d, %d//%d, %d/%d, %d/%d/%d %d//%d */
                if (strstr(buf, "//")) {
                    /* v//n */
                    sscanf(buf, "%d//%d", &v, &n);
                    T(numtriangles).vindices[0] = v < 0 ? v + numvertices : v;
                    T(numtriangles).nindices[0] = n < 0 ? n + numnormals : n;
                    fscanf(file, "%d//%d", &v, &n);
                    T(numtriangles).vindices[1] = v < 0 ? v + numvertices : v;
                    T(numtriangles).nindices[1] = n < 0 ? n + numnormals : n;
                    fscanf(file, "%d//%d", &v, &n);
                    T(numtriangles).vindices[2] = v < 0 ? v + numvertices : v;
                    T(numtriangles).nindices[2] = n < 0 ? n + numnormals : n;
					group->triangles.push_back(numtriangles);
					group->numtriangles++;
                    numtriangles++;
                    while(fscanf(file, "%d//%d", &v, &n) > 0) {
                        T(numtriangles).vindices[0] = T(numtriangles-1).vindices[0];
                        T(numtriangles).nindices[0] = T(numtriangles-1).nindices[0];
                        T(numtriangles).vindices[1] = T(numtriangles-1).vindices[2];
                        T(numtriangles).nindices[1] = T(numtriangles-1).nindices[2];
                        T(numtriangles).vindices[2] = v < 0 ? v + numvertices : v;
                        T(numtriangles).nindices[2] = n < 0 ? n + numnormals : n;
						group->triangles.push_back(numtriangles);
						group->numtriangles++;
                        numtriangles++;
                    }
                } else if (sscanf(buf, "%d/%d/%d", &v, &t, &n) == 3) {
                    /* v/t/n */
                    T(numtriangles).vindices[0] = v < 0 ? v + numvertices : v;
                    T(numtriangles).tindices[0] = t < 0 ? t + numtexcoords : t;
                    T(numtriangles).nindices[0] = n < 0 ? n + numnormals : n;
                    fscanf(file, "%d/%d/%d", &v, &t, &n);
                    T(numtriangles).vindices[1] = v < 0 ? v + numvertices : v;
                    T(numtriangles).tindices[1] = t < 0 ? t + numtexcoords : t;
                    T(numtriangles).nindices[1] = n < 0 ? n + numnormals : n;
                    fscanf(file, "%d/%d/%d", &v, &t, &n);
                    T(numtriangles).vindices[2] = v < 0 ? v + numvertices : v;
                    T(numtriangles).tindices[2] = t < 0 ? t + numtexcoords : t;
                    T(numtriangles).nindices[2] = n < 0 ? n + numnormals : n;
					group->triangles.push_back(numtriangles);
					group->numtriangles++;
                    numtriangles++;
                    while(fscanf(file, "%d/%d/%d", &v, &t, &n) > 0) {
                        T(numtriangles).vindices[0] = T(numtriangles-1).vindices[0];
                        T(numtriangles).tindices[0] = T(numtriangles-1).tindices[0];
                        T(numtriangles).nindices[0] = T(numtriangles-1).nindices[0];
                        T(numtriangles).vindices[1] = T(numtriangles-1).vindices[2];
                        T(numtriangles).tindices[1] = T(numtriangles-1).tindices[2];
                        T(numtriangles).nindices[1] = T(numtriangles-1).nindices[2];
                        T(numtriangles).vindices[2] = v < 0 ? v + numvertices : v;
                        T(numtriangles).tindices[2] = t < 0 ? t + numtexcoords : t;
                        T(numtriangles).nindices[2] = n < 0 ? n + numnormals : n;
						group->triangles.push_back(numtriangles);
						group->numtriangles++;
                        numtriangles++;
                    }
                } else if (sscanf(buf, "%d/%d", &v, &t) == 2) {
                    /* v/t */
                    T(numtriangles).vindices[0] = v < 0 ? v + numvertices : v;
                    T(numtriangles).tindices[0] = t < 0 ? t + numtexcoords : t;
                    fscanf(file, "%d/%d", &v, &t);
                    T(numtriangles).vindices[1] = v < 0 ? v + numvertices : v;
                    T(numtriangles).tindices[1] = t < 0 ? t + numtexcoords : t;
                    fscanf(file, "%d/%d", &v, &t);
                    T(numtriangles).vindices[2] = v < 0 ? v + numvertices : v;
                    T(numtriangles).tindices[2] = t < 0 ? t + numtexcoords : t;
					group->triangles.push_back(numtriangles);
					group->numtriangles++;
                    numtriangles++;
                    while(fscanf(file, "%d/%d", &v, &t) > 0) {
                        T(numtriangles).vindices[0] = T(numtriangles-1).vindices[0];
                        T(numtriangles).tindices[0] = T(numtriangles-1).tindices[0];
                        T(numtriangles).vindices[1] = T(numtriangles-1).vindices[2];
                        T(numtriangles).tindices[1] = T(numtriangles-1).tindices[2];
                        T(numtriangles).vindices[2] = v < 0 ? v + numvertices : v;
                        T(numtriangles).tindices[2] = t < 0 ? t + numtexcoords : t;
						group->triangles.push_back(numtriangles);
						group->numtriangles++;
                        numtriangles++;
                    }
                } else {
                    /* v */
                    sscanf(buf, "%d", &v);
                    T(numtriangles).vindices[0] = v < 0 ? v + numvertices : v;
                    fscanf(file, "%d", &v);
                    T(numtriangles).vindices[1] = v < 0 ? v + numvertices : v;
                    fscanf(file, "%d", &v);
                    T(numtriangles).vindices[2] = v < 0 ? v + numvertices : v;
					group->triangles.push_back(numtriangles);
					group->numtriangles++;
                    numtriangles++;
                    while(fscanf(file, "%d", &v) > 0) {
                        T(numtriangles).vindices[0] = T(numtriangles-1).vindices[0];
                        T(numtriangles).vindices[1] = T(numtriangles-1).vindices[2];
                        T(numtriangles).vindices[2] = v < 0 ? v + numvertices : v;
						group->triangles.push_back(numtriangles);
						group->numtriangles++;
                        numtriangles++;
                    }
                }
                break;

            default:
                /* eat up rest of line */
                fgets(buf, sizeof(buf), file);
                break;
    }
  }
}


/* public functions */


/* glmUnitize: "unitize" a model by translating it to the origin and
 * scaling it to fit in a unit cube around the origin.   Returns the
 * scalefactor used.
 *
 * model - properly initialized GLMmodel structure
 */
GLfloat glmUnitize(GLMmodel* model)
{
    GLuint i;
    GLfloat maxx, minx, maxy, miny, maxz, minz;
    GLfloat cx, cy, cz, w, h, d;
    GLfloat scale;

    assert(model);
	assert(!(model->vertices.empty()));

    /* get the max/mins */
    maxx = minx = model->vertices.at(3 + 0);
    maxy = miny = model->vertices.at(3 + 1);
    maxz = minz = model->vertices.at(3 + 2);
    for (i = 1; i <= model->numvertices; i++) {
        if (maxx < model->vertices.at(3 * i + 0))
            maxx = model->vertices.at(3 * i + 0);
        if (minx > model->vertices.at(3 * i + 0))
            minx = model->vertices.at(3 * i + 0);

        if (maxy < model->vertices.at(3 * i + 1))
            maxy = model->vertices.at(3 * i + 1);
        if (miny > model->vertices.at(3 * i + 1))
            miny = model->vertices.at(3 * i + 1);

        if (maxz < model->vertices.at(3 * i + 2))
            maxz = model->vertices.at(3 * i + 2);
        if (minz > model->vertices.at(3 * i + 2))
            minz = model->vertices.at(3 * i + 2);
    }

    /* calculate model width, height, and depth */
    w = glmAbs(maxx) + glmAbs(minx);
    h = glmAbs(maxy) + glmAbs(miny);
    d = glmAbs(maxz) + glmAbs(minz);

    /* calculate center of the model */
    cx = (maxx + minx) / 2.0;
    cy = (maxy + miny) / 2.0;
    cz = (maxz + minz) / 2.0;

    /* calculate unitizing scale factor */
    scale = 2.0 / glmMax(glmMax(w, h), d);

    /* translate around center then scale */
    for (i = 1; i <= model->numvertices; i++) {
        model->vertices.at(3 * i + 0) -= cx;
        model->vertices.at(3 * i + 1) -= cy;
        model->vertices.at(3 * i + 2) -= cz;
        model->vertices.at(3 * i + 0) *= scale;
        model->vertices.at(3 * i + 1) *= scale;
        model->vertices.at(3 * i + 2) *= scale;
    }

    return scale;
}

/* glmFacetNormals: Generates facet normals for a model (by taking the
 * cross product of the two vectors derived from the sides of each
 * triangle).  Assumes a counter-clockwise winding.
 *
 * model - initialized GLMmodel structure
 */
GLvoid glmFacetNormals(GLMmodel* model)
{
    GLuint  i;
    GLfloat u[3];
    GLfloat v[3];

    assert(model);
	assert(!model->vertices.empty());

    

    /* allocate memory for the new facet normals */
    model->numfacetnorms = model->numtriangles;
	if (!model->facetnorms.empty())
		model->facetnorms.clear();
	model->facetnorms.push_back(0);
	model->facetnorms.push_back(0);
	model->facetnorms.push_back(0);
	
    for (i = 0; i < model->numtriangles; i++) {
        model->triangles[i].findex = i+1;

        u[0] = model->vertices.at(3 * T(i).vindices[1] + 0) -
            model->vertices.at(3 * T(i).vindices[0] + 0);
        u[1] = model->vertices.at(3 * T(i).vindices[1] + 1) -
            model->vertices.at(3 * T(i).vindices[0] + 1);
        u[2] = model->vertices.at(3 * T(i).vindices[1] + 2) -
            model->vertices.at(3 * T(i).vindices[0] + 2);

        v[0] = model->vertices.at(3 * T(i).vindices[2] + 0) -
            model->vertices.at(3 * T(i).vindices[0] + 0);
        v[1] = model->vertices.at(3 * T(i).vindices[2] + 1)-
            model->vertices.at(3 * T(i).vindices[0] + 1);
        v[2] = model->vertices.at(3 * T(i).vindices[2] + 2) -
            model->vertices.at(3 * T(i).vindices[0] + 2);

        vector<GLfloat> cross = glmCross(u, v);
		model->facetnorms.push_back(cross[0]);
		model->facetnorms.push_back(cross[1]);
		model->facetnorms.push_back(cross[2]);
        glmNormalize(&model->facetnorms[3 * (i+1)]);
    }
}

/* glmVertexNormals: Generates smooth vertex normals for a model.
 *
 * model - initialized GLMmodel structure
 * angle - maximum angle (in degrees) to smooth across
 */
GLvoid glmVertexNormals(GLMmodel* model, GLfloat angle)
{
    GLMnode* node;
    GLMnode* tail;
    vector<GLMnode*> members;
    GLuint numnormals;
    GLfloat average[3];
    GLfloat dot, cos_angle;
    GLuint i, avg;

    assert(model);
	assert(!model->facetnorms.empty());

    /* calculate the cosine of the angle (in degrees) */
    cos_angle = cos(angle * M_PI / 180.0);

    /* nuke any previous normals */
	if (!model->normals.empty())
		model->normals.clear();

    /* allocate space for new normals */
    model->numnormals = model->numtriangles * 3; /* 3 normals per triangle */
    //model->normals = new GLfloat[3* (model->numnormals+1)];
	for (int i=0;i<=3* (model->numnormals);i++)
		model->normals.push_back(0);

    /* allocate a structure that will hold a linked list of triangle
    indices for each vertex */
    //members = new (GLMnode*)[(model->numvertices + 1)];
    for (i = 1; i <= model->numvertices; i++)
		members.push_back(NULL);
		

    /* for every triangle, create a node for each vertex in it */
    for (i = 0; i < model->numtriangles; i++) {
        node = new GLMnode;
        node->index = i;
		while (T(i).vindices[0]>=members.size()) //make more space missing above!
			members.push_back(NULL);
		node->next  = members[T(i).vindices[0]];
		members[T(i).vindices[0]] = node;
		//members.push_back(node);

        node = new GLMnode;
        node->index = i;
		while (T(i).vindices[1]>=members.size()) //make more space missing above!
			members.push_back(NULL);
        node->next  = members[T(i).vindices[1]];
		members[T(i).vindices[1]] = node;
		
        node = new GLMnode;
        node->index = i;
		while (T(i).vindices[2]>=members.size()) //make more space missing above!
			members.push_back(NULL);
        node->next  = members[T(i).vindices[2]];
		members[T(i).vindices[2]] = node;
    }

    /* calculate the average normal for each vertex */
    numnormals = 1;
    for (i = 1; i <= model->numvertices; i++) {
    /* calculate an average normal for this vertex by averaging the
        facet normal of every triangle this vertex is in */
        node = members[i];
        if (!node)
            fprintf(stderr, "glmVertexNormals(): vertex w/o a triangle\n");
        average[0] = 0.0; average[1] = 0.0; average[2] = 0.0;
        avg = 0;
        while (node) {
        /* only average if the dot product of the angle between the two
        facet normals is greater than the cosine of the threshold
        angle -- or, said another way, the angle between the two
            facet normals is less than (or equal to) the threshold angle */
            dot = glmDot(&model->facetnorms[3 * T(node->index).findex],
                &model->facetnorms[3 * T(members[i]->index).findex]);
            if (dot > cos_angle) {
                node->averaged = GL_TRUE;
                average[0] += model->facetnorms[3 * T(node->index).findex + 0];
                average[1] += model->facetnorms[3 * T(node->index).findex + 1];
                average[2] += model->facetnorms[3 * T(node->index).findex + 2];
                avg = 1;            /* we averaged at least one normal! */
            } else {
                node->averaged = GL_FALSE;
            }
            node = node->next;
        }

        if (avg) {
            /* normalize the averaged normal */
            glmNormalize(average);

            /* add the normal to the vertex normals list */
            model->normals[3 * numnormals + 0] = average[0];
            model->normals[3 * numnormals + 1] = average[1];
            model->normals[3 * numnormals + 2] = average[2];
            avg = numnormals;
            numnormals++;
        }

        /* set the normal of this vertex in each triangle it is in */
        node = members[i];
        while (node) {
            if (node->averaged) {
                /* if this node was averaged, use the average normal */
                if (T(node->index).vindices[0] == i)
                    T(node->index).nindices[0] = avg;
                else if (T(node->index).vindices[1] == i)
                    T(node->index).nindices[1] = avg;
                else if (T(node->index).vindices[2] == i)
                    T(node->index).nindices[2] = avg;
            } else {
                /* if this node wasn't averaged, use the facet normal */
                model->normals[3 * numnormals + 0] =
                    model->facetnorms[3 * T(node->index).findex + 0];
                model->normals[3 * numnormals + 1] =
                    model->facetnorms[3 * T(node->index).findex + 1];
                model->normals[3 * numnormals + 2] =
                    model->facetnorms[3 * T(node->index).findex + 2];
                if (T(node->index).vindices[0] == i)
                    T(node->index).nindices[0] = numnormals;
                else if (T(node->index).vindices[1] == i)
                    T(node->index).nindices[1] = numnormals;
                else if (T(node->index).vindices[2] == i)
                    T(node->index).nindices[2] = numnormals;
                numnormals++;
            }
            node = node->next;
        }
    }

    model->numnormals = numnormals - 1;

    /* free the member information */
    for (i = 1; i <= model->numvertices; i++) {
        node = members[i];
        while (node) {
            tail = node;
            node = node->next;
            delete tail;
        }
    }
    members.clear();

    /* pack the normals array (we previously allocated the maximum
    number of normals that could possibly be created (numtriangles *
    3), so get rid of some of them (usually alot unless none of the
    facet normals were averaged)) */
	model->normals.erase(model->normals.begin() + 3* (model->numnormals+1), model->normals.end());
}

/* glmReadOBJ: Reads a model description from a Wavefront .OBJ file.
 * filename - name of the file containing the Wavefront .OBJ format data.
 */
GLMmodel* glmReadOBJ(string filename)
{
    GLMmodel* model;
    FILE* file;

    /* open the file */
    file = fopen(filename.c_str(), "r");
    if (!file) {
        cerr<<"glmReadOBJ() failed: can't open data file \""<<filename<<"\"."<<endl;
        exit(1);
    }

    /* allocate a new model */
    model = new GLMmodel();
    model->pathname    = filename;
    model->mtllibname    = "";
    model->numvertices   = 0;
	if (!model->vertices.empty())
		model->vertices.clear();
	model->vertices.push_back(0);model->vertices.push_back(0);model->vertices.push_back(0);
    model->numnormals    = 0;
    if (!model->normals.empty())
		model->normals.clear();
	model->normals.push_back(0);model->normals.push_back(0);model->normals.push_back(0);
    
    model->numtexcoords  = 0;
    if (!model->texcoords.empty())
		model->texcoords.clear();
	model->texcoords.push_back(0);model->texcoords.push_back(0);
    model->numfacetnorms = 0;
    if (!model->facetnorms.empty())
		model->facetnorms.clear();
    
    model->numtriangles  = 0;
    model->nummaterials  = 0;
    if (!model->materials.empty())
		model->materials.clear();
    model->numgroups       = 0;
    model->groups      = NULL;
    model->position[0]   = 0.0;
    model->position[1]   = 0.0;
    model->position[2]   = 0.0;

    /* make a first pass through the file to get a count of the number
    of vertices, normals, texcoords & triangles */
    glmFirstPass(model, file);

    if (!model->triangles.empty())
		model->triangles.clear();
	model->triangles = vector<GLMtriangle>(model->numtriangles);


    /* rewind to beginning of file and read in the data this pass */
    rewind(file);

    glmSecondPass(model, file);

    /* close the file */
    fclose(file);

    return model;
}

/* glmDraw: Renders the model to the current OpenGL context using the
 * mode specified.
 *
 * model - initialized GLMmodel structure
 * mode  - a bitwise OR of values describing what is to be rendered.
 *             GLM_NONE     -  render with only vertices
 *             GLM_FLAT     -  render with facet normals
 *             GLM_SMOOTH   -  render with vertex normals
 *             GLM_TEXTURE  -  render with texture coords
 *             GLM_COLOR    -  render with colors (color material)
 *             GLM_MATERIAL -  render with materials
 *             GLM_COLOR and GLM_MATERIAL should not both be specified.
 *             GLM_FLAT and GLM_SMOOTH should not both be specified.
 */
GLvoid glmDraw(GLMmodel* model, GLuint mode)
{
    static GLuint i;
    static GLMgroup* group;
    static GLMtriangle* triangle;
    static GLMmaterial* material;

    assert(model);
	assert(!model->vertices.empty());

    /* do a bit of warning */
	if (mode & GLM_FLAT && model->facetnorms.empty()) {
        cerr<<"glmDraw() warning: flat render mode requested with no facet normals defined.\n";
        mode &= ~GLM_FLAT;
    }
	if (mode & GLM_SMOOTH && model->normals.empty()) {
        cerr<<"glmDraw() warning: smooth render mode requested with no normals defined.\n";
        mode &= ~GLM_SMOOTH;
    }
	if (mode & GLM_TEXTURE && model->texcoords.empty()) {
        cerr<<"glmDraw() warning: texture render mode requested with no texture coordinates defined.\n";
        mode &= ~GLM_TEXTURE;
    }
    if (mode & GLM_FLAT && mode & GLM_SMOOTH) {
        cerr<<"glmDraw() warning: flat render mode requested and smooth render mode requested (using smooth).\n";
        mode &= ~GLM_FLAT;
    }
	if (mode & GLM_COLOR && model->materials.empty()) {
        cerr<<"glmDraw() warning: color render mode requested with no materials defined.\n";
        mode &= ~GLM_COLOR;
    }
	if (mode & GLM_MATERIAL && model->materials.empty()) {
        cerr<<"glmDraw() warning: material render mode requested with no materials defined.\n";
        mode &= ~GLM_MATERIAL;
    }
    if (mode & GLM_COLOR && mode & GLM_MATERIAL) {
        cerr<<"glmDraw() warning: color and material render mode requested using only material mode.\n";
        mode &= ~GLM_COLOR;
    }
    if (mode & GLM_COLOR)
        glEnable(GL_COLOR_MATERIAL);
    else if (mode & GLM_MATERIAL)
        glDisable(GL_COLOR_MATERIAL);

    group = model->groups;
    while (group) {
        if (mode & GLM_MATERIAL) {
            material = &model->materials[group->material];
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material->ambient);
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material->diffuse);
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material->specular);
            glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material->shininess);
        }

        if (mode & GLM_COLOR) {
			if(material)
            glColor3fv(material->diffuse);
        }
		
		if (mode == GLM_NONE)
		{
			glColor3f(1,1,1);
			glDisable(GL_LIGHTING);
			glDisable(GL_LIGHT0);
			glBegin(GL_POINTS);
			for (i = 0; i < group->numtriangles; i++) {
				triangle = &T(group->triangles[i]);
				glVertex3fv(&model->vertices.at(3 * triangle->vindices[0]));
				glVertex3fv(&model->vertices.at(3 * triangle->vindices[1]));
				glVertex3fv(&model->vertices.at(3 * triangle->vindices[2]));
			}
			glEnd();
		}
		else
		{
			if (!(mode & GLM_MATERIAL))
			{
				glColorMaterial (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
				glEnable (GL_COLOR_MATERIAL );
				glColor3f(1,1,1);
				glEnable(GL_LIGHTING);
				glEnable(GL_LIGHT0);
				float pos[] = {0.0,5.0,0.0,1.0};
				glLightfv(GL_LIGHT0, GL_POSITION, pos);
			}
			if (mode == GLM_FLAT)
			{
				glDisable(GL_LIGHTING);
				glDisable(GL_LIGHT0);
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			glBegin(GL_TRIANGLES);
			for (i = 0; i < group->numtriangles; i++) 
			{
				triangle = &T(group->triangles[i]);

				if (mode & GLM_FLAT)
					glNormal3fv(&model->facetnorms[3 * triangle->findex]);

				if (mode & GLM_SMOOTH)
					glNormal3fv(&model->normals[3 * triangle->nindices[0]]);
				if (mode & GLM_TEXTURE)
					glTexCoord2fv(&model->texcoords[2 * triangle->tindices[0]]);
            

				glVertex3fv(&model->vertices.at(3 * triangle->vindices[0]));

				if (mode & GLM_SMOOTH)
					glNormal3fv(&model->normals[3 * triangle->nindices[1]]);
				if (mode & GLM_TEXTURE)
					glTexCoord2fv(&model->texcoords[2 * triangle->tindices[1]]);
				glVertex3fv(&model->vertices.at(3 * triangle->vindices[1]));

				if (mode & GLM_SMOOTH)
					glNormal3fv(&model->normals[3 * triangle->nindices[2]]);
				if (mode & GLM_TEXTURE)
					glTexCoord2fv(&model->texcoords[2 * triangle->tindices[2]]);
				glVertex3fv(&model->vertices.at(3 * triangle->vindices[2]));
			}
			glEnd();
		}
        group = group->next;
    }
}

struct info
{
	GLMmaterial* material;
	GLMtriangle* triangle;
};