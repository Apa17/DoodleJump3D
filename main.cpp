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
#include <queue>

using namespace std;
using std::chrono::system_clock;




system_clock::time_point starting_time, current_time, previous_time, current_pausa_time;

// variables de estado
//std::chrono::duration<double> delta_pausa_time, deltatime;
double total_time;
int score = 0;
bool fin;
bool velocidad_rapida = false;
bool textOn = true;
bool wireframe = false;
bool facetado = false;
int camara_mode = 0;
enum Mode {
	MAIN_MENU,
	IN_GAME,
	NIVELES_MODE,
	SETTINGS_MODE,
	PAUSA
};

enum Selected_MAIN_MENU {
	NIVELES,
	SETTINGS,
};

enum Selected_NIVELES {
	UNO,
	DOS,
};

enum Selected_SETTINGS{
	VELOCIDAD,
	TEXTURAS,
	WIREFRAME,
	FACETADO
};

enum Selected_MENU_PAUSA {
	REANUDAR,
	SETTINGS_PAUSA
};

Mode mode = MAIN_MENU;
Mode prev_mode;
Selected_MAIN_MENU selected_MAIN_MENU = NIVELES;
Selected_NIVELES selected_NIVELES = UNO;
Selected_SETTINGS selected_SETTINGS = VELOCIDAD;
Selected_MENU_PAUSA selected_MENU_PAUSA = REANUDAR;

SDL_Event evento;


float x, y, z;
float posx = 0, posy = -1.4;
float posyWorld = 0;
float posyWorld_delta = 0;
GLuint* texturas;
GLuint* texturas_digitos;
GLuint* texturas_menu;

GLfloat luz_posicion[4] = { 0, 0, -7, 1 };
GLfloat luz_posicion1[4] = { -5, -5, -5, 1 };
GLfloat colorLuz[4] = { 1, 1, 1, 1 };
GLfloat colorLuzDifusa[4] = { 1.8, 1.8, 1.8, 1.8 };
GLfloat colorLuzSpecular[4] = { 1, 1, 1, 1 };
GLfloat direccion_luz[3] = { 0.0, 0.0, 0.0 };

const double velocidadInicialX = 5;
const double velocidadInicialY = 3;
bool jumping = false;
bool falling = false;
double velocidadX = velocidadInicialX;
double velocidadY = velocidadInicialY;
bool strifingRight = false;
bool strifingLeft = false;

float angulo_x = 180;
float angulo_y = -13;
float radio = -6;

const double aceleracionG = 10;
const double FPS = 60;
const int w = 640;
const int h = 480;
Objeto3d* objetos3d;

struct Plataforma {
	int x;
	float y;
	float tama�o;
};
queue<Plataforma> plataformas;

const int max_plataformas = 7;

void dibujar_doodle() {
	glDisable(GL_TEXTURE_2D);
	glColor3f(1, 1, 0);
	glPushMatrix();
	glRotatef(90, 0.0, 1.0, 0.0); //lo pongo de frente
	glScalef(0.2, 0.2, 0.2); //lo achico
	objetos3d->draw(posx, posy, 0, 1.0, 1.0, 1.0, colorLuz);
	glPopMatrix();
	glEnable(GL_TEXTURE_2D);
}

void re_inicicializacion() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(x, y, z, 0, 0, 0, 0, 1, 0);

	////PRENDO LA LUZ (SIEMPRE DESPUES DEL gluLookAt)
	//glEnable(GL_LIGHT0); // habilita la luz 0
	//glLightfv(GL_LIGHT0, GL_POSITION, luz_posicion);
	//glLightfv(GL_LIGHT0, GL_DIFFUSE, colorLuz);
	GLfloat ambiente[] = { 2.0f,2.0f,2.0f,2.0f };

	glEnable(GL_LIGHT1); // habilita la luz 1
	glLightfv(GL_LIGHT1, GL_POSITION, luz_posicion1);
	//glLightfv(GL_LIGHT1, GL_AMBIENT, ambiente);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, colorLuzDifusa);
	glLightfv(GL_LIGHT1, GL_SPECULAR, colorLuzSpecular);
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, direccion_luz);
	

	//glEnable(GL_LIGHT2); // habilita la luz 1
	//glLightfv(GL_LIGHT2, GL_POSITION, luz_posicion1);
	//glLightfv(GL_LIGHT2, GL_DIFFUSE, colorLuz);
}

void inicializar_plataformas() {
	// Inicializa plataformas y piso
	Plataforma piso;
	piso.x = 0.0f;
	piso.y = -2.0f;
	piso.tama�o = 6.5;
	plataformas.push(piso);
	int x_plat;
	for (int i = 1; i < 7; i++) {
		x_plat = (rand() % 5) - 2;
		Plataforma plataforma1;
		plataforma1.x = x_plat;
		plataforma1.y = i*1.5 - 3;
		plataforma1.tama�o = 1.2f;
		plataformas.push(plataforma1);
	}
}

void mover_camara() {
	x = sin(angulo_x * M_PI / 180) * cos(angulo_y * M_PI / 180) * radio;
	y = sin(angulo_y * M_PI / 180) * radio;
	z = cos(angulo_x * M_PI / 180) * cos(angulo_y * M_PI / 180) * radio;
}

void manejoEventos() {
	while (SDL_PollEvent(&evento)) {
		switch (evento.type) {
			case SDL_MOUSEWHEEL:
				if (camara_mode == 2) {
					if (evento.wheel.y > 0) // scroll up
					{
						if (radio < 0)
							radio += .5;
						mover_camara();
					}
					else if (evento.wheel.y < 0) // scroll down
					{
						radio -= .5;
						mover_camara();
					}
				}
				break;
			case SDL_MOUSEMOTION:
				if (camara_mode == 2) {
					if (evento.motion.yrel < 0 && angulo_y < 80)
						angulo_y -= evento.motion.yrel * 0.4;
					else if (evento.motion.yrel >= 0 && angulo_y > -80)
						angulo_y -= evento.motion.yrel * 0.4;
					angulo_x += evento.motion.xrel * 0.4;
					mover_camara();
				}
				break;
			case SDL_QUIT:
				fin = true;
				break;
			case SDL_KEYDOWN:
				switch (evento.key.keysym.sym) {
					case SDLK_RIGHT:
						strifingRight = true;
						break;
					case SDLK_LEFT:
						strifingLeft = true;
						break;
				}
				break;
			case SDL_KEYUP:
				switch (evento.key.keysym.sym) {
					case SDLK_RIGHT:
						strifingRight = false;
						break;
					case SDLK_LEFT:
						strifingLeft = false;
						break;
					case SDLK_v:
						camara_mode++;
						camara_mode = camara_mode % 3;
						if (camara_mode == 0) {
							x = 0;
							y = 0;
							z = 7;
						}
						if (camara_mode == 1) {
							x = 5;
							y = 5;
							z = 5;
						}
						break;
					case SDLK_ESCAPE:
						if (mode == SETTINGS_MODE) {
							if (prev_mode == MAIN_MENU){
								mode = MAIN_MENU;
								prev_mode = SETTINGS_MODE;
							}
							else {
								mode = PAUSA;
								prev_mode = SETTINGS_MODE;
							}
						}
						else if (mode == IN_GAME){
							prev_mode = IN_GAME;
							mode = PAUSA;
							//pausa();
						}
						else if (mode == PAUSA) {
							prev_mode = PAUSA;
							mode = IN_GAME;
						}
						break;
					case SDLK_l:
						textOn = !textOn;
						break;
					case SDLK_q:
						fin = true;
						break;
					case SDLK_p:
						mode = PAUSA;
						//pausa();
						break;
					case SDLK_RETURN:
						if (mode == MAIN_MENU) {
							if (selected_MAIN_MENU == NIVELES) {
								inicializar_plataformas();
								prev_mode = MAIN_MENU;
								mode = IN_GAME;
							}
							else if (selected_MAIN_MENU == SETTINGS) {
								prev_mode = MAIN_MENU;
								mode = SETTINGS_MODE;
								selected_SETTINGS = VELOCIDAD;
							}
						}
						else if (mode == SETTINGS_MODE) {
							if (selected_SETTINGS == VELOCIDAD) {
								velocidad_rapida = !velocidad_rapida;
							}
							if (selected_SETTINGS == TEXTURAS) {
								textOn = !textOn;
								wireframe = (!textOn) && (wireframe);
							}
							if (selected_SETTINGS == WIREFRAME) {
								wireframe = !wireframe;
								textOn = (!wireframe) && (textOn);
							}
							if (selected_SETTINGS == FACETADO) {
								facetado = !facetado;
							}
						}
						else if (mode == PAUSA) {
							if (selected_MENU_PAUSA == REANUDAR) {
								prev_mode = PAUSA;
								mode = IN_GAME;
							}
							if (selected_MENU_PAUSA == SETTINGS_PAUSA) {
								prev_mode = PAUSA;
								mode = SETTINGS_MODE;
							}
						}
						break;
					case SDLK_DOWN:
						if (mode == MAIN_MENU) {
							selected_MAIN_MENU = SETTINGS;
						}
						else if (mode == IN_GAME) {
							falling = !falling;
						}
						else if (mode == SETTINGS_MODE) {
							if (selected_SETTINGS == VELOCIDAD) {
								selected_SETTINGS = TEXTURAS;
							}
							else if (selected_SETTINGS == TEXTURAS) {
								selected_SETTINGS = WIREFRAME;
							}
							else if (selected_SETTINGS == WIREFRAME) {
								selected_SETTINGS = FACETADO;
							}
						}
						else if (mode == PAUSA) {
							selected_MENU_PAUSA = SETTINGS_PAUSA;
						}
						break;
					case SDLK_UP:
						if (mode == MAIN_MENU) {
							selected_MAIN_MENU = NIVELES;
						}
						else if(mode== IN_GAME){
							jumping = !jumping;
						}
						else if (mode == SETTINGS_MODE) {
							if (selected_SETTINGS == TEXTURAS) {
								selected_SETTINGS = VELOCIDAD;
							}
							else if (selected_SETTINGS == WIREFRAME) {
								selected_SETTINGS = TEXTURAS;
							}
							else if (selected_SETTINGS == FACETADO) {
								selected_SETTINGS = WIREFRAME;
							}
						}
						else if (mode == PAUSA) {
							selected_MENU_PAUSA = REANUDAR;
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
	texturas = new GLuint[6];
	glGenTextures(6, texturas);
	cout << texturas << endl;

	texturas_digitos = new GLuint[11];
	glGenTextures(11, texturas_digitos);

	texturas_menu = new GLuint[18];
	glGenTextures(18, texturas_menu);

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

	char j[] = "../rojo.png";
	cargarTextura(j, 5, texturas);


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

	char check[] = "../check.png";
	cargarTextura(check, 12, texturas_menu);

	char nocheck[] = "../nocheck.png";
	cargarTextura(nocheck, 13, texturas_menu);

	char reanudar[] = "../reanudar.png";
	cargarTextura(reanudar, 14, texturas_menu);

	char reanudar_s[] = "../reanudar_selected.png";
	cargarTextura(reanudar_s, 15, texturas_menu);

	char f[] = "../f.png";
	cargarTextura(f, 16, texturas_menu);

	char i[] = "../i.png";
	cargarTextura(i, 17, texturas_menu);

	
	//FIN CARGAR IMAGEN

	
	
}

void draw_background() {

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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
	if (wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
}

void draw_menu(Mode menu) {

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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

	if (mode == MAIN_MENU)
	{
		//Dibujamos opciones
		int text = 0;
		if (selected_MAIN_MENU == NIVELES) {
			text = 2;
		}
		glBindTexture(GL_TEXTURE_2D, texturas_menu[text]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(220, 250);
		glTexCoord2f(0, 1);
		glVertex2i(220, 330);
		glTexCoord2f(1, 1);
		glVertex2i(420, 330);
		glTexCoord2f(1, 0);
		glVertex2i(420, 250);
		glEnd();

		text = 1;
		if (selected_MAIN_MENU == SETTINGS) {
			text = 3;
		}
		glBindTexture(GL_TEXTURE_2D, texturas_menu[text]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(220, 150);
		glTexCoord2f(0, 1);
		glVertex2i(220, 230);
		glTexCoord2f(1, 1);
		glVertex2i(420, 230);
		glTexCoord2f(1, 0);
		glVertex2i(420, 150);
		glEnd();
	}
	if (mode == PAUSA)
	{
		//Dibujamos opciones
		int text = 14;
		if (selected_MENU_PAUSA == REANUDAR) {
			text = 15;
		}
		glBindTexture(GL_TEXTURE_2D, texturas_menu[text]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(220, 250);
		glTexCoord2f(0, 1);
		glVertex2i(220, 330);
		glTexCoord2f(1, 1);
		glVertex2i(420, 330);
		glTexCoord2f(1, 0);
		glVertex2i(420, 250);
		glEnd();

		text = 1;
		if (selected_MENU_PAUSA == SETTINGS_PAUSA) {
			text = 3;
		}
		glBindTexture(GL_TEXTURE_2D, texturas_menu[text]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(220, 150);
		glTexCoord2f(0, 1);
		glVertex2i(220, 230);
		glTexCoord2f(1, 1);
		glVertex2i(420, 230);
		glTexCoord2f(1, 0);
		glVertex2i(420, 150);
		glEnd();
	}
	else if(mode == SETTINGS_MODE) {

		//Dibujamos opciones
		int text = 4;
		if (selected_SETTINGS == VELOCIDAD) {
			text = 8;
		}
		glBindTexture(GL_TEXTURE_2D, texturas_menu[text]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(220, 380);
		glTexCoord2f(0, 1);
		glVertex2i(220, 460);
		glTexCoord2f(1, 1);
		glVertex2i(420, 460);
		glTexCoord2f(1, 0);
		glVertex2i(420, 380);
		glEnd();

		if (velocidad_rapida) {
			text = 12;
		}
		else {
			text = 13;
		}
		glBindTexture(GL_TEXTURE_2D, texturas_menu[text]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(470, 390);
		glTexCoord2f(0, 1);
		glVertex2i(470, 450);
		glTexCoord2f(1, 1);
		glVertex2i(530, 450);
		glTexCoord2f(1, 0);
		glVertex2i(530, 390);
		glEnd();


		text = 5;
		if (selected_SETTINGS == TEXTURAS) {
			text = 9;
		}
		glBindTexture(GL_TEXTURE_2D, texturas_menu[text]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(220, 280);
		glTexCoord2f(0, 1);
		glVertex2i(220, 360);
		glTexCoord2f(1, 1);
		glVertex2i(420, 360);
		glTexCoord2f(1, 0);
		glVertex2i(420, 280);
		glEnd();

		if (textOn) {
			text = 12;
		}
		else {
			text = 13;
		}
		glBindTexture(GL_TEXTURE_2D, texturas_menu[text]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(470, 290);
		glTexCoord2f(0, 1);
		glVertex2i(470, 350);
		glTexCoord2f(1, 1);
		glVertex2i(530, 350);
		glTexCoord2f(1, 0);
		glVertex2i(530, 290);
		glEnd();


		text = 6;
		if (selected_SETTINGS == WIREFRAME) {
			text = 10;
		}
		glBindTexture(GL_TEXTURE_2D, texturas_menu[text]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(220, 180);
		glTexCoord2f(0, 1);
		glVertex2i(220, 260);
		glTexCoord2f(1, 1);
		glVertex2i(420, 260);
		glTexCoord2f(1, 0);
		glVertex2i(420, 180);
		glEnd();

		if (wireframe) {
			text = 12;
		}
		else {
			text = 13;
		}
		glBindTexture(GL_TEXTURE_2D, texturas_menu[text]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(470, 190);
		glTexCoord2f(0, 1);
		glVertex2i(470, 250);
		glTexCoord2f(1, 1);
		glVertex2i(530, 250);
		glTexCoord2f(1, 0);
		glVertex2i(530, 190);
		glEnd();

		text = 7;
		if (selected_SETTINGS == FACETADO) {
			text = 11;
		}
		glBindTexture(GL_TEXTURE_2D, texturas_menu[text]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(220, 80);
		glTexCoord2f(0, 1);
		glVertex2i(220, 160);
		glTexCoord2f(1, 1);
		glVertex2i(420, 160);
		glTexCoord2f(1, 0);
		glVertex2i(420, 80);
		glEnd();

		if (facetado) {
			text = 16;
		}
		else {
			text = 17;
		}
		glBindTexture(GL_TEXTURE_2D, texturas_menu[text]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(470, 90);
		glTexCoord2f(0, 1);
		glVertex2i(470, 150);
		glTexCoord2f(1, 1);
		glVertex2i(530, 150);
		glTexCoord2f(1, 0);
		glVertex2i(530, 90);
		glEnd();
	}


	//Recuperamos contexto
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	if (wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
}

void dibujar_plataforma(GLfloat x, GLfloat y, int tama�o) {

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texturas[4]);
	glBegin(GL_QUAD_STRIP);
	glTexCoord2f(0, 0);
	glVertex3f(x + 1.0f * tama�o/2, y - 0.15f, 0.f);
	glTexCoord2f(0, 1);
	glVertex3f(x + 1.0f * tama�o / 2, y - 0.15f, 0.4f);
	glTexCoord2f(1, 1);
	glVertex3f(x + 1.0f * tama�o / 2, y + 0.15f, 0.4f);
	glTexCoord2f(1, 0);
	glVertex3f(x + 1.0f * tama�o / 2, y + 0.15f, 0.f);
	glTexCoord2f(1, 0);
	glVertex3f(x - 1.0f * tama�o / 2, y + 0.15f, 0.f);
	glTexCoord2f(1, 1);
	glVertex3f(x - 1.0f * tama�o / 2, y + 0.15f, 0.4f);
	glTexCoord2f(0, 1);
	glVertex3f(x - 1.0f * tama�o / 2, y - 0.15f, 0.4f);
	glTexCoord2f(0, 0);
	glVertex3f(x - 1.0f * tama�o / 2, y - 0.15f, 0.f);
	glTexCoord2f(1, 1);
	glVertex3f(x + 1.0f * tama�o / 2, y - 0.15f, 0.4f);
	glTexCoord2f(1, 0);
	glVertex3f(x + 1.0f * tama�o / 2, y - 0.15f, 0.f);
	glTexCoord2f(0, 1);
	glVertex3f(x - 1.0f * tama�o / 2, y - 0.15f, 0.4f);
	glTexCoord2f(0, 0);
	glVertex3f(x - 1.0f * tama�o / 2, y - 0.15f, 0.f);
	glTexCoord2f(1, 1);
	glVertex3f(x + 1.0f * tama�o / 2, y + 0.15f, 0.4f);
	glTexCoord2f(1, 0);
	glVertex3f(x + 1.0f * tama�o / 2, y + 0.15f, 0.f);
	glTexCoord2f(0, 1);
	glVertex3f(x - 1.0f * tama�o / 2, y + 0.15f, 0.4f);
	glTexCoord2f(0, 0);
	glVertex3f(x - 1.0f * tama�o / 2, y + 0.15f, 0.f);
	glTexCoord2f(1, 1);
	glVertex3f(x + 1.0f * tama�o / 2, y + 0.15f, 0.f);
	glTexCoord2f(1,0);
	glVertex3f(x + 1.0f * tama�o / 2, y - 0.15f, 0.f);
	glTexCoord2f(0, 0);
	glVertex3f(x - 1.0f * tama�o / 2, y - 0.15f, 0.f);
	glTexCoord2f(0, 1);
	glVertex3f(x - 1.0f * tama�o / 2, y + 0.15f, 0.f);
	glTexCoord2f(1, 1);
	glVertex3f(x + 1.0f * tama�o / 2, y + 0.15f, 0.4f);
	glTexCoord2f(1, 0);
	glVertex3f(x + 1.0f * tama�o / 2, y - 0.15f, 0.4f);
	glTexCoord2f(0, 0);
	glVertex3f(x - 1.0f * tama�o / 2, y - 0.15f, 0.4f);
	glTexCoord2f(0, 1);
	glVertex3f(x - 1.0f * tama�o / 2, y + 0.15f, 0.4f);
	glEnd();

}

void dibujar_plataformas() {
	int size = plataformas.size();
	for (int i = 0; i < size; i++) {
		Plataforma plat = plataformas.front();
		plataformas.pop(); 
		dibujar_plataforma(plat.x, plat.y, plat.tama�o); //y - floor(posyWorld)
		plataformas.push(plat);
	}
}

void actualizo_plataformas() {
	Plataforma plat= plataformas.front();
	if (plat.y < -posyWorld - 5) {
		plataformas.pop();
		Plataforma plat;
		plat.x = (rand() % 5) - 2;
		plat.y = -posyWorld + 4;
		plat.tama�o = 1.2f;
		plataformas.push(plat);
	}
}

void dibujarObjetos() {
	glEnable(GL_LIGHTING);
	glPushMatrix();
		glTranslatef(0, posyWorld, 0.0);
		dibujar_plataformas();
	glPopMatrix();
	glPushMatrix();
		glTranslatef(posx, posy, 0.0);
		dibujar_doodle();
	glPopMatrix();
	
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
		if (!((i == 0) && (n == 0))) {
			drawNum(true, i, n);
			i++;
		}
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
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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

	drawNumeros(score, (int)floor(total_time) / 60, (int)floor(total_time) % 60);


	// Termina de dibujar
	
	//carga las matrices previas
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	if (wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
}

bool check_colision(Plataforma p) {
	return false;
}

void controlar_movimiento(std::chrono::duration<double> deltatime) {
	if (jumping) {
		velocidadY = velocidadY - aceleracionG * deltatime.count();
		double incremento = (velocidadInicialY + velocidadY) * deltatime.count() - (1 / 2) * aceleracionG * deltatime.count() * deltatime.count();
		posy += incremento;
		//cout << incremento << endl;
		if (velocidadY < 0) {
			falling = true;
		}
		else {
			falling = false;
		}
		if (falling) {
			//check colisiones
			for (int i = 0; i < plataformas.size(); i++) {
				Plataforma plat = plataformas.front();
				plataformas.pop();
				bool xizq = plat.x - 1.0f * plat.tama�o / 2 < posx;
				bool xder = posx < plat.x + 1.0f * plat.tama�o / 2;
				bool yizq = posyWorld + plat.y + 0.05f < posy;
				bool yder = posy < posyWorld + plat.y + 0.3f;
				bool res = (xizq && xder) && (yizq && yder);
				
				if (res) {
					/*cout << "i=" << i << endl;
					cout << "posx=" << posx << endl;
					cout << "posy=" << posy << endl;
					cout << "plat.x=" << plat.x << endl;
					cout << "plat.y=" << plat.y << endl;
					cout << "plat.tama�o=" << plat.tama�o << endl;
					cout << "posyWorld=" << posyWorld << endl;
					cout << "plat.x - 1.0f * plat.tama�o / 2=" << plat.x - 1.0f * plat.tama�o / 2 << endl;
					cout << "plat.x + 1.0f * plat.tama�o / 2=" << plat.x + 1.0f * plat.tama�o / 2 << endl;
					cout << endl << endl << endl;*/
					jumping = true;
					velocidadY = velocidadInicialY;
					falling = false;
				}
				plataformas.push(plat);
			}
		}/* else {
			posyWorld -= deltatime.count() * 3;
			posyWorld_delta += deltatime.count() * 1.5;
		}*/
		posyWorld -= deltatime.count() * 2;
	}
	else {
		velocidadY = velocidadInicialY;
		velocidadX = velocidadInicialX;
	}
	if (strifingLeft) {
		posx -= velocidadX * deltatime.count();
	}
	if (strifingRight) {
		posx += velocidadX * deltatime.count();
	}
	if (-7 > posy){
		for(int i = 0 ; i<1000; i++)
			cout << "PERDISTE " << "PERDISTE " << "PERDISTE " << "PERDISTE " << "PERDISTE " << "PERDISTE " << "PERDISTE " << "PERDISTE " << "PERDISTE " << "PERDISTE " << endl;
		fin = true;//cambiar por end_game
	}
}

int main(int argc, char *argv[]) {
	srand(time(0));
	//INICIALIZACION
	if (SDL_Init(SDL_INIT_VIDEO)<0) {
		cerr << "No se pudo iniciar SDL: " << SDL_GetError() << endl;
		exit(1);
	}

	SDL_Window* win = SDL_CreateWindow("Doodle Jump 3D",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	SDL_GLContext context = SDL_GL_CreateContext(win);

	x = 0;
	y = 0;
	z = 7;
	textOn = true;
	fin = false;
	current_time = system_clock::now();
	std::chrono::duration<double> delta_pausa_time, deltatime, deltatime_aux;
	total_time = 0;

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
	do {
		score = max(score, ((int)floor(posy)));

		previous_time = current_time;
		current_time = system_clock::now();
		deltatime = (current_time - previous_time);
		if (mode == IN_GAME) {
			deltatime_aux = (current_time - previous_time);
		}
		else
		{
			deltatime_aux = (current_time - current_time);
		}
		total_time += deltatime_aux.count();
		
		
		//DIBUJAR 
		// Supuistamente son necesarias estas dos lineas para transparencia pero por ahora nada
		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		if ((mode == MAIN_MENU) || (mode == SETTINGS_MODE) || (mode == PAUSA) || (mode == NIVELES_MODE)) {
			draw_background();
			draw_menu(mode);
		}

		if (mode == IN_GAME)
		{	
			if (wireframe) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			else {
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
			
			re_inicicializacion();
			actualizo_plataformas();
			draw_background();
			dibujarObjetos();
			controlar_movimiento(deltatime);
			drawHud();
		}
		
		//MANEJO DE EVENTOS
		manejoEventos();
		if ((1000.0 / FPS) > deltatime.count())
			SDL_Delay((1000.0 / FPS) - deltatime.count());
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