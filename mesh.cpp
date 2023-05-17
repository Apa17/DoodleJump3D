#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include <assimp/Importer.hpp>      // C++ importer interface
#include "mesh.h"
#include <iostream>
#include <windows.h>

using namespace std;


vector<Mesh> processModel(const aiScene* scene) {
	vector<Mesh> newObject3d;
	for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
		Mesh newMesh;
		aiMesh* mesh = scene->mMeshes[i];
		vector<Face> faces;
		for (unsigned int j = 0; j < mesh->mNumFaces; j++) {//por cada cara
			Face newFace;
			aiFace face = mesh->mFaces[j];
			newFace.numIndices = face.mNumIndices;
			int* indices = new int[face.mNumIndices];
			for (int k = 0; k < face.mNumIndices; k++) {//por cada indice de vertice
				indices[k] = face.mIndices[k];
			}
			newFace.indices = indices;
			faces.push_back(newFace);
		}
		newMesh.faces = faces;
		Vertex* vertices = new Vertex[mesh->mNumVertices];
		for (unsigned int l = 0; l < mesh->mNumVertices; l++) {
			Vertex v;
			v.xv = mesh->mVertices[l].x;
			v.yv = mesh->mVertices[l].y;
			v.zv = mesh->mVertices[l].z;
			v.xn = mesh->mNormals[l].x;
			v.yn = mesh->mNormals[l].y;
			v.zn = mesh->mNormals[l].z;
			vertices[l] = v;
		}
		newMesh.vertices = vertices;
		newObject3d.push_back(newMesh);
	}
	return newObject3d;
}

void Objeto3d::draw(const float& xobject, const float& yobject, const float& zobject, const double& r, const double& g, const double& b, GLfloat colorLuz[4]) {
	glBegin(GL_TRIANGLES);
	for (std::vector<Mesh>::iterator itm = meshes.begin(); itm != meshes.end(); ++itm) {
		Mesh mesh = *itm;
		for (std::vector<Face>::iterator itf = mesh.faces.begin(); itf != mesh.faces.end(); ++itf) {//por cada cara
			Face face = *itf;
			//glColor3f(0, 55, 0);	
			for (int k = 0; k < face.numIndices; k++) {//por cada indice de vertice
				int indicedevertice = face.indices[k];
				//gltextcoord en indicevertice
				GLfloat xlocal, ylocal, zlocal;
				xlocal = mesh.vertices[indicedevertice].xv;
				ylocal = mesh.vertices[indicedevertice].yv;
				zlocal = mesh.vertices[indicedevertice].zv;
				//glColor3f(0 + i * 10, 55+i*10+j*5+k*15, 0);
				glEnable(GL_COLOR_MATERIAL);
				glColor3f(r * colorLuz[0], g * colorLuz[1], b * colorLuz[2]);
				glNormal3f(mesh.vertices[indicedevertice].xn, mesh.vertices[indicedevertice].yn, mesh.vertices[indicedevertice].zn);
				glVertex3f(xlocal + xobject, ylocal + yobject, zlocal + zobject);
			}
		}
		//dibujar mVertice[indice]
	}
	glColor3f(1.0, 1.0, 1.0); //Resetear el color
	glEnd();

}

void Objeto3d::load(const char* pFile) { // eq loadmodel
	// Create an instance of the Importer class

	Assimp::Importer importer;
	const aiScene* scene; 

	// And have it read the given file with some example postprocessing
	// Usually - if speed is not the most important aspect for you - you'll
	// probably to request more postprocessing than we do in this example.
	scene = importer.ReadFile(pFile,
		aiProcess_Triangulate | aiProcess_GenNormals);

	// If the import failed, report it
	if (nullptr == scene) {
		cout << importer.GetErrorString();
		cout << &pFile;
		throw runtime_error("Error: se ha producido un error al cargar un modelo");
	}

	meshes = processModel(scene);
}

Objeto3d* cargarObjetos3d()
{
	Objeto3d* objetos3d = new Objeto3d[1];	
	Objeto3d a = Objeto3d();
	char buffer[MAX_PATH];  // Buffer to store the path
	if (GetCurrentDirectoryA(MAX_PATH, buffer) != 0) {
		std::string currentDir(buffer);
		std::string filePath = currentDir + "\\doodler.obj";
		const char* filePathChar = filePath.c_str();
		a.load(filePath.c_str());
		objetos3d[0] = a;
		return objetos3d;
	}
	else {
		throw runtime_error("Error: se ha producido un error al conseguir el pwd");
	}}
