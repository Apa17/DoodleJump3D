#ifndef MESH_H
#define MESH_H
#include <SDL_opengl.h>
#include <vector>
#include <GL/glu.h>

struct Vertex {
	GLfloat xv;
	GLfloat yv;
	GLfloat zv;
	GLfloat xn;
	GLfloat yn;
	GLfloat zn;
	GLfloat xt; // New texture coordinates
	GLfloat yt; // New texture coordinates
};

struct Face {
	int* indices;
	int numIndices;
};

struct Mesh {
	std::vector<Face> faces;
	Vertex* vertices;
	GLuint textureID; // New texture ID field
};

struct Objeto3d {
	std::vector<Mesh> meshes;
	void load(const char* filename);
	void draw(const float& xobject, const float& yobject, const float& zobject, const double& r, const double& g, const double& b, GLfloat colorLuz[4]); //render
};

Objeto3d* cargarObjetos3d();

#endif