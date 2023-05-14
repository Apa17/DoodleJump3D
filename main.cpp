#include "SDL.h"
#include "SDL_opengl.h"
#include <iostream>
#include "FreeImage.h"
#include <stdio.h>
#include <conio.h>
#include <GL/glu.h>
#include "game_hud.cpp"
#include "mesh.h"
#include <time.h>

using namespace std;
using std::chrono::system_clock;

system_clock::time_point starting_time, current_time, previous_time, current_pausa_time;

// variables de estado
//std::chrono::duration<double> delta_pausa_time, deltatime;
GLfloat luz_posicion[4] = { 0, 0, 1, 1 };
GLfloat luz_posicion1[4] = { 0, 0, -1, 1 };
GLfloat colorLuz[4] = { 1, 1, 1, 1 };
bool textOn;
bool fin;
bool rotation;
string mode = "MAIN MENU";
string selected = "NIVELES";

SDL_Event evento;

float x, y, z;
GLuint* texturas;
GLuint* texturas_digitos;
GLuint* texturas_menu;

float degrees = 0;

int w = 640;
int h = 480;
Objeto3d* objetos3d;



struct Plataforma {
	int x;
	float tamaño;
};

vector<Plataforma> plataformas;

const int max_plataformas = 7;

void manejoEventos() {
	while (SDL_PollEvent(&evento)) {
		switch (evento.type) {
		case SDL_SCANCODE_DOWN:
			break;
		case SDL_KEYUP:
			switch (evento.key.keysym.sym) {
			case SDLK_RETURN:
				if (mode == "MAIN MENU") {
					if (selected == "NIVELES") {
						mode = "IN GAME";
					}
					if (selected == "SETTINGS") {
						mode = "SETTINGS";
						selected = "VELOCIDAD";
					}
				}
				break;
			case SDLK_UP:
				if (mode == "MAIN MENU") {
					selected = "NIVELES";
				}
				break;
			case SDLK_DOWN:
				if (mode == "MAIN MENU") {
					selected = "SETTINGS";
				}
				break;
			}
		}
	}
}


void cargarTextura(char archivo[], int i, GLuint* texturas_array) {
	FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(archivo);
	FIBITMAP* bitmap = FreeImage_Load(fif, archivo);
	cout << archivo << bitmap << endl;
	bitmap = FreeImage_ConvertTo24Bits(bitmap);
	int w = FreeImage_GetWidth(bitmap);
	int h = FreeImage_GetHeight(bitmap);
	void* datos= FreeImage_GetBits(bitmap);
	glBindTexture(GL_TEXTURE_2D, texturas_array[i]);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, datos);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

void cargarTexturas() {
	texturas = new GLuint[5];
	glGenTextures(5, texturas);
	cout << texturas << endl;

	texturas_digitos = new GLuint[11];
	glGenTextures(11, texturas_digitos);

	texturas_menu = new GLuint[14];
	glGenTextures(4, texturas_menu);

	//archivos
	char archivo[] = "../canon.png";
	cargarTextura(archivo,0, texturas);

	char archivo2[] = "../score.png";
	cargarTextura(archivo2,1, texturas);

	char archivo3[] = "../time.png";
	cargarTextura(archivo3, 2, texturas);

	char archivo4[] = "../fondo.jpg";
	cargarTextura(archivo4, 3, texturas);

	char archivo5[] = "../marron.jpg";
	cargarTextura(archivo5, 4, texturas);


	// Cargo texturas de numeros
	char archivo_temp[] = "../0.png";
	cargarTextura(archivo_temp, 0, texturas_digitos);

	archivo_temp[3] = '1';
	cargarTextura(archivo_temp, 1, texturas_digitos);

	archivo_temp[3] = '2';
	cargarTextura(archivo_temp, 2, texturas_digitos);

	archivo_temp[3] = '3';
	cargarTextura(archivo_temp, 3, texturas_digitos);

	archivo_temp[3] = '4';
	cargarTextura(archivo_temp, 4, texturas_digitos);

	archivo_temp[3] = '5';
	cargarTextura(archivo_temp, 5, texturas_digitos);

	archivo_temp[3] = '6';
	cargarTextura(archivo_temp, 6, texturas_digitos);

	archivo_temp[3] = '7';
	cargarTextura(archivo_temp, 7, texturas_digitos);

	archivo_temp[3] = '8';
	cargarTextura(archivo_temp, 8, texturas_digitos);

	archivo_temp[3] = '9';
	cargarTextura(archivo_temp, 9, texturas_digitos);


	char archivo_punto[] = "../punto.png";
	cargarTextura(archivo_punto, 10, texturas_digitos);


	// Cargar texturas menus
	char niveles[] = "../niveles.png";
	cargarTextura(niveles, 0, texturas_menu);

	char settings[] = "../settings.png";
	cargarTextura(settings, 1, texturas_menu);

	// Cargar texturas menus
	char niveles_s[] = "../niveles_selected.png";
	cargarTextura(niveles_s, 2, texturas_menu);

	char settings_s[] = "../settings_selected.png";
	cargarTextura(settings_s, 3, texturas_menu);

	char velocidad[] = "../velocidad.png";
	cargarTextura(velocidad, 4, texturas_menu);

	char textures[] = "../texturas.png";
	cargarTextura(textures, 5, texturas_menu);

	char wireframe[] = "../wireframe.png";
	cargarTextura(wireframe, 6, texturas_menu);

	char facetado[] = "../facetado.png";
	cargarTextura(facetado, 7, texturas_menu);

	char velocidad_s[] = "../velocidad_selected.png";
	cargarTextura(velocidad_s, 8, texturas_menu);

	char textures_s[] = "../texturas_selected.png";
	cargarTextura(textures_s, 9, texturas_menu);

	char wireframe_s[] = "../wireframe_selected.png";
	cargarTextura(wireframe_s, 10, texturas_menu);

	char facetado_s[] = "../facetado_selected.png";
	cargarTextura(facetado_s, 11, texturas_menu);

	
	//FIN CARGAR IMAGEN

	
	
}

void draw_background() {

	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texturas[3]);
	glDisable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, w, 0, h, -1, 1);


	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glBegin(GL_QUADS);

	glColor3f(1, 1, 1);
	glTexCoord2f(0, 0);
	glVertex2f(-w, -h);
	glTexCoord2f(1, 0);
	glVertex2f(w, -h);
	glTexCoord2f(1, 1);
	glVertex2f(w, h);
	glTexCoord2f(0, 1);
	glVertex2f(-w, h);

	glEnd();

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
}

void draw_menu(string menu) {
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);

	//Guardamos contexto
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, w, 0, h, -1, 1);


	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	if (mode == "MAIN MENU")
	{
		//Dibujamos opciones
		int text = 0;
		if (selected == "NIVELES") {
			text = 2;
		}
		glBindTexture(GL_TEXTURE_2D, texturas_menu[text]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(250, 250);
		glTexCoord2f(0, 1);
		glVertex2i(250, 330);
		glTexCoord2f(1, 1);
		glVertex2i(390, 330);
		glTexCoord2f(1, 0);
		glVertex2i(390, 250);
		glEnd();

		text = 1;
		if (selected == "SETTINGS") {
			text = 3;
		}
		glBindTexture(GL_TEXTURE_2D, texturas_menu[text]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(250, 150);
		glTexCoord2f(0, 1);
		glVertex2i(250, 230);
		glTexCoord2f(1, 1);
		glVertex2i(390, 230);
		glTexCoord2f(1, 0);
		glVertex2i(390, 150);
		glEnd();
	}
	else if(mode == "SETTINGS") {

		//Dibujamos opciones
		int text = 4;
		if (selected == "VELOCIDAD") {
			text = 8;
		}
		glBindTexture(GL_TEXTURE_2D, texturas_menu[text]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(250, 380);
		glTexCoord2f(0, 1);
		glVertex2i(250, 460);
		glTexCoord2f(1, 1);
		glVertex2i(390, 460);
		glTexCoord2f(1, 0);
		glVertex2i(390, 380);
		glEnd();

		int text = 5;
		if (selected == "TEXTURAS") {
			text = 9;
		}
		glBindTexture(GL_TEXTURE_2D, texturas_menu[text]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(250, 280);
		glTexCoord2f(0, 1);
		glVertex2i(250, 360);
		glTexCoord2f(1, 1);
		glVertex2i(390, 360);
		glTexCoord2f(1, 0);
		glVertex2i(390, 280);
		glEnd();

		int text = 6;
		if (selected == "WIREFRAME") {
			text = 10;
		}
		glBindTexture(GL_TEXTURE_2D, texturas_menu[text]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(250, 180);
		glTexCoord2f(0, 1);
		glVertex2i(250, 260);
		glTexCoord2f(1, 1);
		glVertex2i(390, 260);
		glTexCoord2f(1, 0);
		glVertex2i(390, 180);
		glEnd();

		int text = 7;
		if (selected == "VELOCIDAD") {
			text = 11;
		}
		glBindTexture(GL_TEXTURE_2D, texturas_menu[text]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(250, 380);
		glTexCoord2f(0, 1);
		glVertex2i(250, 460);
		glTexCoord2f(1, 1);
		glVertex2i(390, 460);
		glTexCoord2f(1, 0);
		glVertex2i(390, 380);
		glEnd();
	}


	//Recuperamos contexto
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
}

void dibujar_plataforma(GLfloat x, GLfloat y, int tamaño) {

	glBindTexture(GL_TEXTURE_2D, texturas[4]);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUAD_STRIP);
	glTexCoord2f(0, 0);
	glVertex3f(x + 1.0f * tamaño/2, y - 0.15f, 0.f);
	glTexCoord2f(0, 1);
	glVertex3f(x + 1.0f * tamaño / 2, y - 0.15f, 0.4f);
	glTexCoord2f(1, 1);
	glVertex3f(x + 1.0f * tamaño / 2, y + 0.15f, 0.4f);
	glTexCoord2f(1, 0);
	glVertex3f(x + 1.0f * tamaño / 2, y + 0.15f, 0.f);
	glTexCoord2f(1, 0);
	glVertex3f(x - 1.0f * tamaño / 2, y + 0.15f, 0.f);
	glTexCoord2f(1, 1);
	glVertex3f(x - 1.0f * tamaño / 2, y + 0.15f, 0.4f);
	glTexCoord2f(0, 1);
	glVertex3f(x - 1.0f * tamaño / 2, y - 0.15f, 0.4f);
	glTexCoord2f(0, 0);
	glVertex3f(x - 1.0f * tamaño / 2, y - 0.15f, 0.f);
	glTexCoord2f(1, 1);
	glVertex3f(x + 1.0f * tamaño / 2, y - 0.15f, 0.4f);
	glTexCoord2f(1, 0);
	glVertex3f(x + 1.0f * tamaño / 2, y - 0.15f, 0.f);
	glTexCoord2f(0, 1);
	glVertex3f(x - 1.0f * tamaño / 2, y - 0.15f, 0.4f);
	glTexCoord2f(0, 0);
	glVertex3f(x - 1.0f * tamaño / 2, y - 0.15f, 0.f);
	glTexCoord2f(1, 1);
	glVertex3f(x + 1.0f * tamaño / 2, y + 0.15f, 0.4f);
	glTexCoord2f(1, 0);
	glVertex3f(x + 1.0f * tamaño / 2, y + 0.15f, 0.f);
	glTexCoord2f(0, 1);
	glVertex3f(x - 1.0f * tamaño / 2, y + 0.15f, 0.4f);
	glTexCoord2f(0, 0);
	glVertex3f(x - 1.0f * tamaño / 2, y + 0.15f, 0.f);
	glTexCoord2f(1, 1);
	glVertex3f(x + 1.0f * tamaño / 2, y + 0.15f, 0.f);
	glTexCoord2f(1,0);
	glVertex3f(x + 1.0f * tamaño / 2, y - 0.15f, 0.f);
	glTexCoord2f(0, 0);
	glVertex3f(x - 1.0f * tamaño / 2, y - 0.15f, 0.f);
	glTexCoord2f(0, 1);
	glVertex3f(x - 1.0f * tamaño / 2, y + 0.15f, 0.f);
	glTexCoord2f(1, 1);
	glVertex3f(x + 1.0f * tamaño / 2, y + 0.15f, 0.4f);
	glTexCoord2f(1, 0);
	glVertex3f(x + 1.0f * tamaño / 2, y - 0.15f, 0.4f);
	glTexCoord2f(0, 0);
	glVertex3f(x - 1.0f * tamaño / 2, y - 0.15f, 0.4f);
	glTexCoord2f(0, 1);
	glVertex3f(x - 1.0f * tamaño / 2, y + 0.15f, 0.4f);
	glEnd();

}

void dibujarObjetos() {
	// Dibujar plataformas
	int i = 0;
	for (auto& element : plataformas) {
		dibujar_plataforma(element.x, -3 + i, element.tamaño);
		i++;
		if (i == 7) {
			break;
		}
	}
}



// Funcion que usamos para pasar numeros a lista de chars y dibujarlos luego.
void collect_digits(std::vector<int>& digits, unsigned long num) {
	if (num > 9) {
		collect_digits(digits, num / 10);
	}
	digits.push_back(num % 10);
}

void drawNum(bool time, unsigned int pos, unsigned int num) {
	int ymin;
	int ymax;
	if (time) {
		ymin = 55;
		ymax = 70;
	}
	else {
		ymin = 25;
		ymax = 45;
	}
	int xmin = 80 + (pos * 15);
	int xmax = xmin + 15;

	// Dibuja numero
	glBindTexture(GL_TEXTURE_2D, texturas_digitos[num]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2i(xmin, ymin);
	glTexCoord2f(1.0f, 0.0f);
	glVertex2i(xmax, ymin);
	glTexCoord2f(1.0f, 1.0f);
	glVertex2i(xmax, ymax);
	glTexCoord2f(0.0f, 1.0f);
	glVertex2i(xmin, ymax);
	glEnd();
}

void drawNumeros(int score, int min, int sec) {
	std::vector<int> digitos_score;
	collect_digits(digitos_score, score);
	std::vector<int> digitos_min;
	collect_digits(digitos_min, min);
	std::vector<int> digitos_sec;
	collect_digits(digitos_sec, sec);

	int i = 0;
	for (int n : digitos_score) {
		drawNum(false, i, n);
		i++;
	}

	i = 0;
	for (int n : digitos_min) {
		drawNum(true, i, n);
		i++;
	}

	if (i > 0) {
		drawNum(true, i, 10);
		i++;
	}

	for (int n : digitos_sec) {
		drawNum(true, i, n);
		i++;
	}
}

void drawHud() {
	// save state
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, w, 0, h, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	// Dibuja el HUD
	glBegin(GL_QUADS);
	glColor3f(1, 1, 1);
	glVertex2i(20, 20);
	glVertex2i(180, 20);
	glVertex2i(180, 80);
	glVertex2i(20, 80);
	glEnd();


	//enable textures if textOn
	if (textOn) {
		glEnable(GL_TEXTURE_2D);

		// Dibuja score
		glBindTexture(GL_TEXTURE_2D, texturas[1]);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2i(30, 20);
		glTexCoord2f(1.0f, 0.0f);
		glVertex2i(80, 20);
		glTexCoord2f(1.0f, 1.0f);
		glVertex2i(80, 50);
		glTexCoord2f(0.0f, 1.0f);
		glVertex2i(30, 50);
		glEnd();

		// Dibuja time
		glBindTexture(GL_TEXTURE_2D, texturas[2]);

		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2i(30, 50);
		glTexCoord2f(1.0f, 0.0f);
		glVertex2i(80, 50);
		glTexCoord2f(1.0f, 1.0f);
		glVertex2i(80, 76);
		glTexCoord2f(0.0f, 1.0f);
		glVertex2i(30, 76);
		glEnd();

		drawNumeros(10, 2, 55);
	}
	else {
		glDisable(GL_TEXTURE_2D);
		glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
	}


	// Termina de dibujar
	
	//carga las matrices previas
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}

int main(int argc, char *argv[]) {

	srand(time(0));
	// Inicializa plataformas y piso
	Plataforma piso;
	piso.x = 0.0f;
	piso.tamaño = 6.5;
	plataformas.push_back(piso);
	int x_plat;
	for (int i = 1; i < 7; i++) {
		x_plat = (rand() % 5) - 2;
		Plataforma plataforma1;
		plataforma1.x = x_plat;
		plataforma1.tamaño = 1.2f;
		plataformas.push_back(plataforma1);
	}

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
		draw_background();
		if ((mode == "MAIN MENU") || (mode == "SETTINGS")) {
			draw_menu(mode);
		}

		if (mode == "IN GAME")
		{
			//TRANSFORMACIONES LINEALES
			if (rotation) {
				degrees = degrees + 1;
			}
			glRotatef(degrees, 0.0, 1.0, 0.0);
			glEnable(GL_LIGHTING);
			dibujarObjetos();

			drawHud();
		}
		

		
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
