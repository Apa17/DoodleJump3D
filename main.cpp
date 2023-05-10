#include "SDL.h"
#include "SDL_opengl.h"
#include <iostream>
#include "FreeImage.h"
#include <stdio.h>
#include <conio.h>
#include <GL/glu.h>
#include "game_hud.cpp"
#include "mesh.h"

using namespace std;
using std::chrono::system_clock;

system_clock::time_point starting_time, current_time, previous_time, current_pausa_time;
//std::chrono::duration<double> delta_pausa_time, deltatime;
GLfloat luz_posicion[4] = { 0, 0, 1, 1 };
GLfloat luz_posicion1[4] = { 0, 0, -1, 1 };
GLfloat colorLuz[4] = { 1, 1, 1, 1 };
bool textOn;
bool fin;
bool rotation;

SDL_Event evento;

float x, y, z;
GLuint* texturas;

float degrees = 0;

int w = 640;
int h = 480;
Objeto3d* objetos3d;


void manejoEventos() {
	while (SDL_PollEvent(&evento)) {
		switch (evento.type) {
		case SDL_MOUSEBUTTONDOWN:
			rotation = true;
			break;
		case SDL_MOUSEBUTTONUP:
			rotation = false;
			break;
		case SDL_QUIT:
			fin = true;
			break;
		case SDL_KEYUP:
			switch (evento.key.keysym.sym) {
			case SDLK_ESCAPE:
				fin = true;
				break;
			case SDLK_l:
				textOn = !textOn;
				break;
			case SDLK_RIGHT:
				break;
			}
		}
	}
}


void cargarTextura(char* archivo, int i) {
	FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(archivo);
	FIBITMAP* bitmap = FreeImage_Load(fif, archivo);
	cout << archivo << bitmap << endl;
	bitmap = FreeImage_ConvertTo24Bits(bitmap);
	int w = FreeImage_GetWidth(bitmap);
	int h = FreeImage_GetHeight(bitmap);
	void* datos= FreeImage_GetBits(bitmap);
	glBindTexture(GL_TEXTURE_2D, texturas[i]);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, datos);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

void cargarTexturas() {
	texturas = new GLuint[2];
	glGenTextures(2, texturas);
	cout << texturas << endl;

	//archivos
	char* archivo = new char[20];
	archivo = "../canon.png";
	cargarTextura(archivo,0);

	char* archivo2 = new char[20];
	archivo2 = "../numeros.jpg";
	cargarTextura(archivo2,1);
	
	//FIN CARGAR IMAGEN

	
	
}

void dibujarObjetos() {
	//DIBUJO TRIANGULO CON COLOR
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glBegin(GL_TRIANGLES);
	glColor3f(1.0, 0.0, 0.0);
	glVertex3f(1., -1., 0.);
	glVertex3f(-1., -1., 0.);
	glVertex3f(0., 1., 0.);
	glEnd();
	glPopMatrix();
	glDisable(GL_LIGHTING);

	//DIBUJO TRIANGULO CON TEXTURA
	if (textOn) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texturas[0]);
	}
	glBegin(GL_TRIANGLES);
	glColor3f(1.0, 1.0, 1.0);
	glTexCoord2f(0, 0);
	glVertex3f(3., -1., 0.);
	glTexCoord2f(0, 1);
	glVertex3f(1., -1., 0.);
	glTexCoord2f(1, 0);
	glVertex3f(2., 1., 0.);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	//DIBUJO TRIANGULO CON LUZ
	glEnable(GL_LIGHTING);
	glBegin(GL_TRIANGLES);
	glNormal3f(0, 0, 1);
	glVertex3f(-1., -1., 0.);
	glVertex3f(-3., -1., 0.);
	glVertex3f(-2., 1., 0.);
	glEnd();	
	objetos3d[0].draw(0, 0, -5, 1, 1, 0, colorLuz);
	glDisable(GL_LIGHTING);
}


void drawHud() {
	//save state
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, w, 0, h, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	//enable textures if textOn
	if (textOn) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texturas[1]);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}
	else {
		glDisable(GL_TEXTURE_2D);
		glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
	}
	// Dibuja el HUD
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2i(40, 40);
	glTexCoord2f(1.0f, 0.0f);
	glVertex2i(150, 40);
	glTexCoord2f(1.0f, 1.0f);
	glVertex2i(150, 100);
	glTexCoord2f(0.0f, 1.0f);
	glVertex2i(40, 100);
	glEnd();
	// Termina de dibujar
	
	//carga las matrices previas
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}

int main(int argc, char *argv[]) {
	//INICIALIZACION
	if (SDL_Init(SDL_INIT_VIDEO)<0) {
		cerr << "No se pudo iniciar SDL: " << SDL_GetError() << endl;
		exit(1);
	}

	SDL_Window* win = SDL_CreateWindow("Doodle Jump 3d",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	SDL_GLContext context = SDL_GL_CreateContext(win);

	x = 0;
	y = 0;
	z = 7;
	textOn = true;
	fin = false;
	rotation = false;
	degrees = 0;
	current_time = system_clock::now();

	glMatrixMode(GL_PROJECTION);

	float color = 0;
	glClearColor(color, color, color, 1);

	gluPerspective(45, w / h, 0.1, 100);
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_MODELVIEW);

	objetos3d = cargarObjetos3d();
	cargarTexturas();
	//FIN INICIALIZACION 
	
	//LOOP PRINCIPAL
	do{
		previous_time = current_time;
		current_time = system_clock::now();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();
		gluLookAt(x, y, z, 0, 0, 0, 0, 1, 0);

		//PRENDO LA LUZ (SIEMPRE DESPUES DEL gluLookAt)
		glEnable(GL_LIGHT0); // habilita la luz 0
		glLightfv(GL_LIGHT0, GL_POSITION, luz_posicion);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, colorLuz);
		
		glEnable(GL_LIGHT1); // habilita la luz 1
		glLightfv(GL_LIGHT1, GL_POSITION, luz_posicion1);
		glLightfv(GL_LIGHT1, GL_DIFFUSE, colorLuz);

		glPushMatrix();
		//TRANSFORMACIONES LINEALES
		if (rotation){
			degrees = degrees + 1;
		}
		glRotatef(degrees, 0.0, 1.0, 0.0);

		//DIBUJAR 
		dibujarObjetos();
		drawHud();
		

		

		//MANEJO DE EVENTOS
		manejoEventos();
		//FIN MANEJO DE EVENTOS
		SDL_GL_SwapWindow(win);
	} while (!fin);
	//FIN LOOP PRINCIPAL
	// LIMPIEZA
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(win);
	SDL_Quit();
	return 0;
}
