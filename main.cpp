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
float velocidad_multiplicador = 1;
bool textOn = true;
bool textOnAux;
bool wireframe = false;
bool facetado = false;
int camara_mode = 0;
enum Mode {
	MAIN_MENU,
	IN_GAME,
	NIVELES_MODE,
	SETTINGS_MODE,
	PAUSA,
	GAME_OVER,
	LUCES_MODE
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
	FACETADO,
	LUCES
};

enum Selected_MENU_PAUSA {
	REANUDAR,
	SETTINGS_PAUSA
};

enum MENU_LUCES_SELECTED {
	LUZ1_POS1,
	LUZ1_POS2,
	LUZ1_POS3,
	LUZ2_POS1,
	LUZ2_POS2,
	LUZ2_POS3,
	LUZ1_COLR,
	LUZ1_COLA,
	LUZ1_COLG,
	LUZ1_COLB,
	LUZ2_COLR,
	LUZ2_COLA,
	LUZ2_COLG,
	LUZ2_COLB,
};

Mode mode = MAIN_MENU;
Mode prev_mode;
Selected_MAIN_MENU selected_MAIN_MENU = NIVELES;
Selected_NIVELES selected_NIVELES = UNO;
Selected_SETTINGS selected_SETTINGS = VELOCIDAD;
Selected_MENU_PAUSA selected_MENU_PAUSA = REANUDAR;
MENU_LUCES_SELECTED selected_LUCES = LUZ1_POS1;

SDL_Event evento;


float x, y, z;
float posx = 0, posy = -1.4;
float posyWorld = 0;
float posyWorld_delta = 0;
GLuint* texturas;
GLuint* texturas_digitos;
GLuint* texturas_menu;

GLfloat pos1[3] = {0.0, 1.0, 8.0};
GLfloat pos2[3] = {1.0, 0.0, 8.0};
GLfloat pos3[3] = {0.0, 0.0, 0.0};
GLfloat luz_posicion[3] = { 0.0, 1.0, 8.0 };
GLfloat luz_posicion1[3] = { 1.0, 0.0, 8.0 };
GLfloat colorLuz[4] = { 1, 1, 1, 0.1 };
GLfloat colorLuz1[4] = { 1, 1, 1, 0.1 };
GLfloat blanco[4] = { 1, 1, 1, 0.1 };
GLfloat verde[4] = { 0, 0.1, 0, 0.1 };
GLfloat azul[4] = { 0, 0, 0.1, 0.1 };
GLfloat rojo[4] = { 0.1, 0, 0, 0.1 };
GLfloat ambiente[4] = {0.1, 0.1, 0.1, 1};
GLfloat specular[4] = {0.5, 0.5, 0.5, 1};
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
	float tamaño;
};
queue<Plataforma> plataformas;

bool equals_homogeneas(GLfloat a1[], GLfloat a2[]) {
	int i = 0;
	while ((i < 4) && (a1[i] == a2[i])) {
		i++;
	}
	return i == 4;
}

void dibujar_doodle() {
	glDisable(GL_TEXTURE_2D);
	glPushMatrix();
	glScalef(0.2, 0.2, 0.2);
	objetos3d->draw(posx, posy, 0, 1, 1.0, 1, colorLuz, true);
	glPopMatrix();
	glEnable(GL_TEXTURE_2D);
}

void re_inicicializacion() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(x, y, z, 0, 0, 0, 0, 1, 0);

	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

	////PRENDO LA LUZ (SIEMPRE DESPUES DEL gluLookAt)
	glEnable(GL_LIGHT0); // habilita la luz 1
	glLightfv(GL_LIGHT0, GL_POSITION, luz_posicion);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambiente);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, colorLuz);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, direccion_luz);

	glEnable(GL_LIGHT1); // habilita la luz 1
	glLightfv(GL_LIGHT1, GL_POSITION, luz_posicion1);
	glLightfv(GL_LIGHT1, GL_AMBIENT, ambiente);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, colorLuz1);
	glLightfv(GL_LIGHT1, GL_SPECULAR, specular);
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, direccion_luz);
	

}

void inicializar_plataformas() {
	// Inicializa plataformas y piso
	Plataforma piso;
	piso.x = 0.0f;
	piso.y = -2.0f;
	piso.tamaño = 6.5;
	plataformas.push(piso);
	int x_plat;
	for (int i = 1; i < 15; i++) {
		x_plat = (rand() % 5) - 2;
		Plataforma plataforma1;
		plataforma1.x = x_plat;
		plataforma1.y = i*1.5 - 2;
		plataforma1.tamaño = 1.2f;
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
						if (mode == LUCES_MODE) {
							switch (selected_LUCES) {
							case LUZ1_POS1:
								selected_LUCES = LUZ1_POS2;
								break;
							case LUZ2_POS1:
								selected_LUCES = LUZ2_POS2;
								break;
							case LUZ1_POS2:
								selected_LUCES = LUZ1_POS3;
								break;
							case LUZ2_POS2:
								selected_LUCES = LUZ2_POS3;
								break;
							case LUZ1_POS3:
								break;
							case LUZ2_POS3:
								break;
							case LUZ1_COLB:
								selected_LUCES = LUZ1_COLG;
								break;
							case LUZ1_COLG:
								selected_LUCES = LUZ1_COLA;
								break;
							case LUZ1_COLA:
								selected_LUCES = LUZ1_COLR;
								break;
							case LUZ1_COLR:
								break;
							case LUZ2_COLB:
								selected_LUCES = LUZ2_COLG;
								break;
							case LUZ2_COLG:
								selected_LUCES = LUZ2_COLA;
								break;
							case LUZ2_COLA:
								selected_LUCES = LUZ2_COLR;
								break;
							case LUZ2_COLR:
								break;
							}
						}
						break;
					case SDLK_LEFT:
						strifingLeft = false;
						if (mode == LUCES_MODE) {
							switch (selected_LUCES) {
							case LUZ1_POS1:
								break;
							case LUZ2_POS1:
								break;
							case LUZ1_POS2:
								selected_LUCES = LUZ1_POS1;
								break;
							case LUZ2_POS2:
								selected_LUCES = LUZ2_POS1;
								break;
							case LUZ1_POS3:
								selected_LUCES = LUZ1_POS3;
								break;
							case LUZ2_POS3:
								selected_LUCES = LUZ2_POS2;
								break;
							case LUZ1_COLB:
								break;
							case LUZ1_COLG:
								selected_LUCES = LUZ1_COLB;
								break;
							case LUZ1_COLA:
								selected_LUCES = LUZ1_COLG;
								break;
							case LUZ1_COLR:
								selected_LUCES = LUZ1_COLA;
								break;
							case LUZ2_COLB:
								break;
							case LUZ2_COLG:
								selected_LUCES = LUZ2_COLB;
								break;
							case LUZ2_COLA:
								selected_LUCES = LUZ2_COLG;
								break;
							case LUZ2_COLR:
								selected_LUCES = LUZ2_COLA;
								break;
							}
						}
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
							if (prev_mode != PAUSA){
								mode = MAIN_MENU;
							}
							else {
								mode = PAUSA;
							}
						}
						else if (mode == IN_GAME){
							mode = PAUSA;
						}
						else if (mode == PAUSA) {
							prev_mode = PAUSA;
							mode = IN_GAME;
						}
						else if (mode == GAME_OVER) {
							mode = MAIN_MENU;
						}
						else if (mode == LUCES_MODE) {
							mode = SETTINGS_MODE;
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
								if (velocidad_multiplicador == 1) {
									velocidad_multiplicador = 1.5;
								}
								else {
									velocidad_multiplicador = 1;
								}
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
							if (selected_SETTINGS == LUCES) {
								mode = LUCES_MODE;
								break;
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
						else if (mode == GAME_OVER) {
							mode = MAIN_MENU;
						}
						else if (mode == LUCES_MODE) {
							switch (selected_LUCES) {
							case LUZ1_POS1:
								for (int i = 0; i < 4; i++) {
									luz_posicion[i] = pos1[i];
								}
								break;
							case LUZ2_POS1:
								for (int i = 0; i < 4; i++) {
									luz_posicion1[i] = pos1[i];
								}
								break;
							case LUZ1_POS2:
								for (int i = 0; i < 4; i++) {
									luz_posicion[i] = pos2[i];
								}
								break;
							case LUZ2_POS2:
								for (int i = 0; i < 4; i++) {
									luz_posicion1[i] = pos2[i];
								}
								break;
							case LUZ1_POS3:
								for (int i = 0; i < 4; i++) {
									luz_posicion[i] = pos3[i];
								}
								break;
							case LUZ2_POS3:
								for (int i = 0; i < 4; i++) {
									luz_posicion1[i] = pos3[i];
								}
								break;
							case LUZ1_COLB:
								for (int i = 0; i < 4; i++) {
									colorLuz[i] = blanco[i];
								}
								break;
							case LUZ1_COLG:
								for (int i = 0; i < 4; i++) {
									colorLuz[i] = verde[i];
								}
								break;
							case LUZ1_COLA:
								for (int i = 0; i < 4; i++) {
									colorLuz[i] = azul[i];
								}
								break;
							case LUZ1_COLR:
								for (int i = 0; i < 4; i++) {
									colorLuz[i] = rojo[i];
								}
								break;
							case LUZ2_COLB:
								for (int i = 0; i < 4; i++) {
									colorLuz1[i] = blanco[i];
								}
								break;
							case LUZ2_COLG:
								for (int i = 0; i < 4; i++) {
									colorLuz1[i] = verde[i];
								}
								break;
							case LUZ2_COLA:
								for (int i = 0; i < 4; i++) {
									colorLuz1[i] = azul[i];
								}
								break;
							case LUZ2_COLR:
								for (int i = 0; i < 4; i++) {
									colorLuz1[i] = rojo[i];
								}
								break;
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
							else if (selected_SETTINGS == FACETADO) {
								selected_SETTINGS = LUCES;
							}
						}
						else if (mode == PAUSA) {
							selected_MENU_PAUSA = SETTINGS_PAUSA;
						}
						else if (mode == LUCES_MODE) {
							switch (selected_LUCES) {
							case LUZ1_POS1:
								selected_LUCES = LUZ2_POS1;
								break;
							case LUZ2_POS1:
								selected_LUCES = LUZ1_COLB;
								break;
							case LUZ1_POS2:
								selected_LUCES = LUZ2_POS2;
								break;
							case LUZ2_POS2:
								selected_LUCES = LUZ1_COLG;
								break;
							case LUZ1_POS3:
								selected_LUCES = LUZ2_POS3;
								break;
							case LUZ2_POS3:
								selected_LUCES = LUZ1_COLA;
								break;
							case LUZ1_COLB:
								selected_LUCES = LUZ2_COLB;
								break;
							case LUZ1_COLG:
								selected_LUCES = LUZ2_COLG;
								break;
							case LUZ1_COLA:
								selected_LUCES = LUZ2_COLA;
								break;
							case LUZ1_COLR:
								selected_LUCES = LUZ2_COLR;
								break;
							case LUZ2_COLB:
								break;
							case LUZ2_COLG:
								break;
							case LUZ2_COLA:
								break;
							case LUZ2_COLR:
								break;
							}
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
							else if (selected_SETTINGS == LUCES) {
								selected_SETTINGS = FACETADO;
							}
						}
						else if (mode == PAUSA) {
							selected_MENU_PAUSA = REANUDAR;
						}
						else if (mode == LUCES_MODE) {
							switch (selected_LUCES) {
							case LUZ1_POS1:
								break;
							case LUZ2_POS1:
								selected_LUCES = LUZ1_POS1;
								break;
							case LUZ1_POS2:
								break;
							case LUZ2_POS2:
								selected_LUCES = LUZ1_POS2;
								break;
							case LUZ1_POS3:
								break;
							case LUZ2_POS3:
								selected_LUCES = LUZ1_POS3;
								break;
							case LUZ1_COLB:
								selected_LUCES = LUZ2_POS1;
								break;
							case LUZ1_COLG:
								selected_LUCES = LUZ2_POS2;
								break;
							case LUZ1_COLA:
								selected_LUCES = LUZ2_POS3;
								break;
							case LUZ1_COLR:
								selected_LUCES = LUZ2_POS3;
								break;
							case LUZ2_COLB:
								selected_LUCES = LUZ1_COLB;
								break;
							case LUZ2_COLG:
								selected_LUCES = LUZ1_COLG;
								break;
							case LUZ2_COLA:
								selected_LUCES = LUZ1_COLA;
								break;
							case LUZ2_COLR:
								selected_LUCES = LUZ1_COLR;
								break;
							}
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, datos);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

void cargarTexturas() {
	texturas = new GLuint[6];
	glGenTextures(6, texturas);
	cout << texturas << endl;

	texturas_digitos = new GLuint[11];
	glGenTextures(11, texturas_digitos);

	texturas_menu = new GLuint[30];
	glGenTextures(30, texturas_menu);

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

	char gameover[] = "../gameover.png";
	cargarTextura(gameover, 18, texturas_menu);

	char iralmenu[] = "../iralmenu.png";
	cargarTextura(iralmenu, 19, texturas_menu);

	char luces[] = "../luces.png";
	cargarTextura(luces, 20, texturas_menu);

	char luces_s[] = "../luces_selected.png";
	cargarTextura(luces_s, 21, texturas_menu);

	char posicion[] = "../posicion.png";
	cargarTextura(posicion, 22, texturas_menu);

	char color[] = "../color.png";
	cargarTextura(color, 23, texturas_menu);

	char luz1[] = "../luz1.png";
	cargarTextura(luz1, 24, texturas_menu);

	char luz2[] = "../luz2.png";
	cargarTextura(luz2, 25, texturas_menu);

	char pos1t[] = "../pos1.png";
	cargarTextura(pos1t, 26, texturas_menu);

	char pos2t[] = "../pos2.png";
	cargarTextura(pos2t, 27, texturas_menu);

	char pos3t[] = "../pos3.png";
	cargarTextura(pos3t, 28, texturas_menu);

	char flecha[] = "../flecha.png";
	cargarTextura(flecha, 29, texturas_menu);

	
	//FIN CARGAR IMAGEN

	
	
}

void draw_background() {

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, w, 0, h, -1, 1);


	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	if (textOn) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texturas[3]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2f(-w, -h);
		glTexCoord2f(1, 0);
		glVertex2f(w, -h);
		glTexCoord2f(1, 1);
		glVertex2f(w, h);
		glTexCoord2f(0, 1);
		glVertex2f(-w, h);
		glEnd();
	}
	else {
		glColor3f(1, 1, 1);
		glBegin(GL_QUADS);
		glVertex2f(-w, -h);
		glVertex2f(w, -h);
		glVertex2f(w, h);
		glVertex2f(-w, h);
		glEnd();
	}


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
		glVertex2i(220, 390);
		glTexCoord2f(0, 1);
		glVertex2i(220, 460);
		glTexCoord2f(1, 1);
		glVertex2i(420, 460);
		glTexCoord2f(1, 0);
		glVertex2i(420, 390);
		glEnd();

		if (velocidad_multiplicador == 1.5) {
			text = 12;
		}
		else {
			text = 13;
		}
		glBindTexture(GL_TEXTURE_2D, texturas_menu[text]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(470, 395);
		glTexCoord2f(0, 1);
		glVertex2i(470, 455);
		glTexCoord2f(1, 1);
		glVertex2i(530, 455);
		glTexCoord2f(1, 0);
		glVertex2i(530, 395);
		glEnd();


		text = 5;
		if (selected_SETTINGS == TEXTURAS) {
			text = 9;
		}
		glBindTexture(GL_TEXTURE_2D, texturas_menu[text]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(220, 300);
		glTexCoord2f(0, 1);
		glVertex2i(220, 370);
		glTexCoord2f(1, 1);
		glVertex2i(420, 370);
		glTexCoord2f(1, 0);
		glVertex2i(420, 300);
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
		glVertex2i(470, 305);
		glTexCoord2f(0, 1);
		glVertex2i(470, 365);
		glTexCoord2f(1, 1);
		glVertex2i(530, 365);
		glTexCoord2f(1, 0);
		glVertex2i(530, 305);
		glEnd();


		text = 6;
		if (selected_SETTINGS == WIREFRAME) {
			text = 10;
		}
		glBindTexture(GL_TEXTURE_2D, texturas_menu[text]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(220, 210);
		glTexCoord2f(0, 1);
		glVertex2i(220, 280);
		glTexCoord2f(1, 1);
		glVertex2i(420, 280);
		glTexCoord2f(1, 0);
		glVertex2i(420, 210);
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
		glVertex2i(470, 215);
		glTexCoord2f(0, 1);
		glVertex2i(470, 275);
		glTexCoord2f(1, 1);
		glVertex2i(530, 275);
		glTexCoord2f(1, 0);
		glVertex2i(530, 215);
		glEnd();

		text = 7;
		if (selected_SETTINGS == FACETADO) {
			text = 11;
		}
		glBindTexture(GL_TEXTURE_2D, texturas_menu[text]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(220, 120);
		glTexCoord2f(0, 1);
		glVertex2i(220, 190);
		glTexCoord2f(1, 1);
		glVertex2i(420, 190);
		glTexCoord2f(1, 0);
		glVertex2i(420, 120);
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
		glVertex2i(470, 125);
		glTexCoord2f(0, 1);
		glVertex2i(470, 185);
		glTexCoord2f(1, 1);
		glVertex2i(530, 185);
		glTexCoord2f(1, 0);
		glVertex2i(530, 125);
		glEnd();

		text = 20;
		if (selected_SETTINGS == LUCES) {
			text = 21;
		}
		glBindTexture(GL_TEXTURE_2D, texturas_menu[text]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(220, 30);
		glTexCoord2f(0, 1);
		glVertex2i(220, 100);
		glTexCoord2f(1, 1);
		glVertex2i(420, 100);
		glTexCoord2f(1, 0);
		glVertex2i(420, 30);
		glEnd();
	}
	else if (mode == LUCES_MODE) {
		// POSICION
		glBindTexture(GL_TEXTURE_2D, texturas_menu[22]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(60, 380);
		glTexCoord2f(0, 1);
		glVertex2i(60, 460);
		glTexCoord2f(1, 1);
		glVertex2i(180, 460);
		glTexCoord2f(1, 0);
		glVertex2i(180, 380);
		glEnd();

		//POS 1
		glBindTexture(GL_TEXTURE_2D, texturas_menu[26]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(220, 380);
		glTexCoord2f(0, 1);
		glVertex2i(220, 460);
		glTexCoord2f(1, 1);
		glVertex2i(320, 460);
		glTexCoord2f(1, 0);
		glVertex2i(320, 380);
		glEnd();

		//POS 2
		glBindTexture(GL_TEXTURE_2D, texturas_menu[27]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(360, 380);
		glTexCoord2f(0, 1);
		glVertex2i(360, 460);
		glTexCoord2f(1, 1);
		glVertex2i(460, 460);
		glTexCoord2f(1, 0);
		glVertex2i(460, 380);
		glEnd();

		//POS 3
		glBindTexture(GL_TEXTURE_2D, texturas_menu[28]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(500, 380);
		glTexCoord2f(0, 1);
		glVertex2i(500, 460);
		glTexCoord2f(1, 1);
		glVertex2i(600, 460);
		glTexCoord2f(1, 0);
		glVertex2i(600, 380);
		glEnd();

		// LUZ 1
		glBindTexture(GL_TEXTURE_2D, texturas_menu[24]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(60, 300);
		glTexCoord2f(0, 1);
		glVertex2i(60, 360);
		glTexCoord2f(1, 1);
		glVertex2i(180, 360);
		glTexCoord2f(1, 0);
		glVertex2i(180, 300);
		glEnd();

		if (selected_LUCES == LUZ1_POS1) {
			// FLECHA
			glBindTexture(GL_TEXTURE_2D, texturas_menu[29]);
			glBegin(GL_QUADS);
			glTexCoord2f(0, 0);
			glVertex2i(220, 300);
			glTexCoord2f(0, 1);
			glVertex2i(220, 360);
			glTexCoord2f(1, 1);
			glVertex2i(270, 360);
			glTexCoord2f(1, 0);
			glVertex2i(270, 300);
			glEnd();
		}

		int text = 13;
		if (equals_homogeneas(luz_posicion, pos1)) {
			text = 12;
		}
		// CHECK
		glBindTexture(GL_TEXTURE_2D, texturas_menu[text]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(270, 300);
		glTexCoord2f(0, 1);
		glVertex2i(270, 360);
		glTexCoord2f(1, 1);
		glVertex2i(320, 360);
		glTexCoord2f(1, 0);
		glVertex2i(320, 300);
		glEnd();


		if (selected_LUCES == LUZ1_POS2) {
			// FLECHA
			glBindTexture(GL_TEXTURE_2D, texturas_menu[29]);
			glBegin(GL_QUADS);
			glTexCoord2f(0, 0);
			glVertex2i(360, 300);
			glTexCoord2f(0, 1);
			glVertex2i(360, 360);
			glTexCoord2f(1, 1);
			glVertex2i(410, 360);
			glTexCoord2f(1, 0);
			glVertex2i(410, 300);
			glEnd();
		}

		text = 13;
		if (equals_homogeneas(luz_posicion, pos2)) {
			text = 12;
		}
		// CHECK
		glBindTexture(GL_TEXTURE_2D, texturas_menu[text]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(410, 300);
		glTexCoord2f(0, 1);
		glVertex2i(410, 360);
		glTexCoord2f(1, 1);
		glVertex2i(460, 360);
		glTexCoord2f(1, 0);
		glVertex2i(460, 300);
		glEnd();

		if (selected_LUCES == LUZ1_POS3) {
			// FLECHA
			glBindTexture(GL_TEXTURE_2D, texturas_menu[29]);
			glBegin(GL_QUADS);
			glTexCoord2f(0, 0);
			glVertex2i(500, 300);
			glTexCoord2f(0, 1);
			glVertex2i(500, 360);
			glTexCoord2f(1, 1);
			glVertex2i(550, 360);
			glTexCoord2f(1, 0);
			glVertex2i(550, 300);
			glEnd();
		}

		text = 13;
		if (equals_homogeneas(luz_posicion, pos3)) {
			text = 12;
		}
		// CHECK
		glBindTexture(GL_TEXTURE_2D, texturas_menu[text]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(550, 300);
		glTexCoord2f(0, 1);
		glVertex2i(550, 360);
		glTexCoord2f(1, 1);
		glVertex2i(600, 360);
		glTexCoord2f(1, 0);
		glVertex2i(600, 300);
		glEnd();




		// LUZ 2
		glBindTexture(GL_TEXTURE_2D, texturas_menu[25]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(60, 220);
		glTexCoord2f(0, 1);
		glVertex2i(60, 280);
		glTexCoord2f(1, 1);
		glVertex2i(180, 280);
		glTexCoord2f(1, 0);
		glVertex2i(180, 220);
		glEnd();

		if (selected_LUCES == LUZ2_POS1) {
			// FLECHA
			glBindTexture(GL_TEXTURE_2D, texturas_menu[29]);
			glBegin(GL_QUADS);
			glTexCoord2f(0, 0);
			glVertex2i(220, 220);
			glTexCoord2f(0, 1);
			glVertex2i(220, 280);
			glTexCoord2f(1, 1);
			glVertex2i(270, 280);
			glTexCoord2f(1, 0);
			glVertex2i(270, 220);
			glEnd();
		}

		text = 13;
		if (equals_homogeneas(luz_posicion1, pos1)) {
			text = 12;
		}
		// CHECK
		glBindTexture(GL_TEXTURE_2D, texturas_menu[text]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(270, 220);
		glTexCoord2f(0, 1);
		glVertex2i(270, 280);
		glTexCoord2f(1, 1);
		glVertex2i(320, 280);
		glTexCoord2f(1, 0);
		glVertex2i(320, 220);
		glEnd();


		if (selected_LUCES == LUZ2_POS2) {
			// FLECHA
			glBindTexture(GL_TEXTURE_2D, texturas_menu[29]);
			glBegin(GL_QUADS);
			glTexCoord2f(0, 0);
			glVertex2i(360, 220);
			glTexCoord2f(0, 1);
			glVertex2i(360, 280);
			glTexCoord2f(1, 1);
			glVertex2i(410, 280);
			glTexCoord2f(1, 0);
			glVertex2i(410, 220);
			glEnd();
		}

		text = 13;
		if (equals_homogeneas(luz_posicion1, pos2)) {
			text = 12;
		}
		// CHECK
		glBindTexture(GL_TEXTURE_2D, texturas_menu[text]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(410, 220);
		glTexCoord2f(0, 1);
		glVertex2i(410, 280);
		glTexCoord2f(1, 1);
		glVertex2i(460, 280);
		glTexCoord2f(1, 0);
		glVertex2i(460, 220);
		glEnd();

		if (selected_LUCES == LUZ2_POS3) {
			// FLECHA
			glBindTexture(GL_TEXTURE_2D, texturas_menu[29]);
			glBegin(GL_QUADS);
			glTexCoord2f(0, 0);
			glVertex2i(500, 220);
			glTexCoord2f(0, 1);
			glVertex2i(500, 280);
			glTexCoord2f(1, 1);
			glVertex2i(550, 280);
			glTexCoord2f(1, 0);
			glVertex2i(550, 220);
			glEnd();
		}

		text = 13;
		if (equals_homogeneas(luz_posicion1, pos3)) {
			text = 12;
		}
		// CHECK
		glBindTexture(GL_TEXTURE_2D, texturas_menu[text]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(550, 220);
		glTexCoord2f(0, 1);
		glVertex2i(550, 280);
		glTexCoord2f(1, 1);
		glVertex2i(600, 280);
		glTexCoord2f(1, 0);
		glVertex2i(600, 220);
		glEnd();








		// COLOR
		glBindTexture(GL_TEXTURE_2D, texturas_menu[23]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(60, 140);
		glTexCoord2f(0, 1);
		glVertex2i(60, 200);
		glTexCoord2f(1, 1);
		glVertex2i(180, 200);
		glTexCoord2f(1, 0);
		glVertex2i(180, 140);
		glEnd();

		// BLANCO
		glDisable(GL_TEXTURE_2D);
		glColor3f(1, 1, 1);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(220, 140);
		glTexCoord2f(0, 1);
		glVertex2i(220, 200);
		glTexCoord2f(1, 1);
		glVertex2i(280, 200);
		glTexCoord2f(1, 0);
		glVertex2i(280, 140);
		glEnd();

		// VERDE
		glColor3f(0, 1, 0);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(320, 140);
		glTexCoord2f(0, 1);
		glVertex2i(320, 200);
		glTexCoord2f(1, 1);
		glVertex2i(380, 200);
		glTexCoord2f(1, 0);
		glVertex2i(380, 140);
		glEnd();

		// AZUL
		glColor3f(0, 0, 1);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(420, 140);
		glTexCoord2f(0, 1);
		glVertex2i(420, 200);
		glTexCoord2f(1, 1);
		glVertex2i(480, 200);
		glTexCoord2f(1, 0);
		glVertex2i(480, 140);
		glEnd();

		// ROJO
		glColor3f(1, 0, 0);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(520, 140);
		glTexCoord2f(0, 1);
		glVertex2i(520, 200);
		glTexCoord2f(1, 1);
		glVertex2i(580, 200);
		glTexCoord2f(1, 0);
		glVertex2i(580, 140);
		glEnd();
		glEnable(GL_TEXTURE_2D);
		glColor3f(1, 1, 1);

		// LUZ 1
		glBindTexture(GL_TEXTURE_2D, texturas_menu[24]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(60, 80);
		glTexCoord2f(0, 1);
		glVertex2i(60, 120);
		glTexCoord2f(1, 1);
		glVertex2i(180, 120);
		glTexCoord2f(1, 0);
		glVertex2i(180, 80);
		glEnd();

		if (selected_LUCES == LUZ1_COLB) {
			// FLECHA
			glBindTexture(GL_TEXTURE_2D, texturas_menu[29]);
			glBegin(GL_QUADS);
			glTexCoord2f(0, 0);
			glVertex2i(220, 80);
			glTexCoord2f(0, 1);
			glVertex2i(220, 120);
			glTexCoord2f(1, 1);
			glVertex2i(250, 120);
			glTexCoord2f(1, 0);
			glVertex2i(250, 80);
			glEnd();
		}

		text = 13;
		if (equals_homogeneas(colorLuz, blanco)) {
			text = 12;
		}
		// CHECK
		glBindTexture(GL_TEXTURE_2D, texturas_menu[text]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(250, 80);
		glTexCoord2f(0, 1);
		glVertex2i(250, 120);
		glTexCoord2f(1, 1);
		glVertex2i(280, 120);
		glTexCoord2f(1, 0);
		glVertex2i(280, 80);
		glEnd();


		if (selected_LUCES == LUZ1_COLG) {
			// FLECHA
			glBindTexture(GL_TEXTURE_2D, texturas_menu[29]);
			glBegin(GL_QUADS);
			glTexCoord2f(0, 0);
			glVertex2i(320, 80);
			glTexCoord2f(0, 1);
			glVertex2i(320, 120);
			glTexCoord2f(1, 1);
			glVertex2i(350, 120);
			glTexCoord2f(1, 0);
			glVertex2i(350, 80);
			glEnd();
		}

		text = 13;
		if (equals_homogeneas(colorLuz, verde)) {
			text = 12;
		}
		// CHECK
		glBindTexture(GL_TEXTURE_2D, texturas_menu[text]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(350, 80);
		glTexCoord2f(0, 1);
		glVertex2i(350, 120);
		glTexCoord2f(1, 1);
		glVertex2i(380, 120);
		glTexCoord2f(1, 0);
		glVertex2i(380, 80);
		glEnd();

		if (selected_LUCES == LUZ1_COLA) {
			// FLECHA
			glBindTexture(GL_TEXTURE_2D, texturas_menu[29]);
			glBegin(GL_QUADS);
			glTexCoord2f(0, 0);
			glVertex2i(420, 80);
			glTexCoord2f(0, 1);
			glVertex2i(420, 120);
			glTexCoord2f(1, 1);
			glVertex2i(450, 120);
			glTexCoord2f(1, 0);
			glVertex2i(450, 80);
			glEnd();
		}

		text = 13;
		if (equals_homogeneas(colorLuz, azul)) {
			text = 12;
		}
		// CHECK
		glBindTexture(GL_TEXTURE_2D, texturas_menu[text]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(450, 80);
		glTexCoord2f(0, 1);
		glVertex2i(450, 120);
		glTexCoord2f(1, 1);
		glVertex2i(480, 120);
		glTexCoord2f(1, 0);
		glVertex2i(480, 80);
		glEnd();

		if (selected_LUCES == LUZ1_COLR) {
			// FLECHA
			glBindTexture(GL_TEXTURE_2D, texturas_menu[29]);
			glBegin(GL_QUADS);
			glTexCoord2f(0, 0);
			glVertex2i(520, 80);
			glTexCoord2f(0, 1);
			glVertex2i(520, 120);
			glTexCoord2f(1, 1);
			glVertex2i(550, 120);
			glTexCoord2f(1, 0);
			glVertex2i(550, 80);
			glEnd();
		}

		text = 13;
		if (equals_homogeneas(colorLuz, rojo)) {
			text = 12;
		}
		// CHECK
		glBindTexture(GL_TEXTURE_2D, texturas_menu[text]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(550, 80);
		glTexCoord2f(0, 1);
		glVertex2i(550, 120);
		glTexCoord2f(1, 1);
		glVertex2i(580, 120);
		glTexCoord2f(1, 0);
		glVertex2i(580, 80);
		glEnd();




		// LUZ 2
		glBindTexture(GL_TEXTURE_2D, texturas_menu[25]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(60, 20);
		glTexCoord2f(0, 1);
		glVertex2i(60, 60);
		glTexCoord2f(1, 1);
		glVertex2i(180, 60);
		glTexCoord2f(1, 0);
		glVertex2i(180, 20);
		glEnd();

		if (selected_LUCES == LUZ2_COLB) {
			// FLECHA
			glBindTexture(GL_TEXTURE_2D, texturas_menu[29]);
			glBegin(GL_QUADS);
			glTexCoord2f(0, 0);
			glVertex2i(220, 20);
			glTexCoord2f(0, 1);
			glVertex2i(220, 60);
			glTexCoord2f(1, 1);
			glVertex2i(250, 60);
			glTexCoord2f(1, 0);
			glVertex2i(250, 20);
			glEnd();
		}

		text = 13;
		if (equals_homogeneas(colorLuz1, blanco)) {
			text = 12;
		}
		// CHECK
		glBindTexture(GL_TEXTURE_2D, texturas_menu[text]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(250, 20);
		glTexCoord2f(0, 1);
		glVertex2i(250, 60);
		glTexCoord2f(1, 1);
		glVertex2i(280, 60);
		glTexCoord2f(1, 0);
		glVertex2i(280, 20);
		glEnd();


		if (selected_LUCES == LUZ2_COLG) {
			// FLECHA
			glBindTexture(GL_TEXTURE_2D, texturas_menu[29]);
			glBegin(GL_QUADS);
			glTexCoord2f(0, 0);
			glVertex2i(320, 20);
			glTexCoord2f(0, 1);
			glVertex2i(320, 60);
			glTexCoord2f(1, 1);
			glVertex2i(350, 60);
			glTexCoord2f(1, 0);
			glVertex2i(350, 20);
			glEnd();
		}

		text = 13;
		if (equals_homogeneas(colorLuz1, verde)) {
			text = 12;
		}
		// CHECK
		glBindTexture(GL_TEXTURE_2D, texturas_menu[text]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(350, 20);
		glTexCoord2f(0, 1);
		glVertex2i(350, 60);
		glTexCoord2f(1, 1);
		glVertex2i(380, 60);
		glTexCoord2f(1, 0);
		glVertex2i(380, 20);
		glEnd();

		if (selected_LUCES == LUZ2_COLA) {
			// FLECHA
			glBindTexture(GL_TEXTURE_2D, texturas_menu[29]);
			glBegin(GL_QUADS);
			glTexCoord2f(0, 0);
			glVertex2i(420, 20);
			glTexCoord2f(0, 1);
			glVertex2i(420, 60);
			glTexCoord2f(1, 1);
			glVertex2i(450, 60);
			glTexCoord2f(1, 0);
			glVertex2i(450, 20);
			glEnd();
		}

		text = 13;
		if (equals_homogeneas(colorLuz1, azul)) {
			text = 12;
		}
		// CHECK
		glBindTexture(GL_TEXTURE_2D, texturas_menu[text]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(450, 20);
		glTexCoord2f(0, 1);
		glVertex2i(450, 60);
		glTexCoord2f(1, 1);
		glVertex2i(480, 60);
		glTexCoord2f(1, 0);
		glVertex2i(480, 20);
		glEnd();

		if (selected_LUCES == LUZ2_COLR) {
			// FLECHA
			glBindTexture(GL_TEXTURE_2D, texturas_menu[29]);
			glBegin(GL_QUADS);
			glTexCoord2f(0, 0);
			glVertex2i(520, 20);
			glTexCoord2f(0, 1);
			glVertex2i(520, 60);
			glTexCoord2f(1, 1);
			glVertex2i(550, 60);
			glTexCoord2f(1, 0);
			glVertex2i(550, 20);
			glEnd();
		}

		text = 13;
		if (equals_homogeneas(colorLuz1, rojo)) {
			text = 12;
		}
		// CHECK
		glBindTexture(GL_TEXTURE_2D, texturas_menu[text]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(550, 20);
		glTexCoord2f(0, 1);
		glVertex2i(550, 60);
		glTexCoord2f(1, 1);
		glVertex2i(580, 60);
		glTexCoord2f(1, 0);
		glVertex2i(580, 20);
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

void dibujar_plataforma(GLfloat x, GLfloat y, int tamaño) {
	if (textOn) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texturas[4]);

	}
	else {
		glDisable(GL_TEXTURE_2D);
	}
	glBegin(GL_QUADS);

	// Cara de derecha

	glTexCoord2f(0, 0);
	glNormal3f(1/3, -1/3, -1/3);
	glVertex3f(x + 1.0f * tamaño/2, y - 0.15f, 0.f); //Derecha abajo atras
	glTexCoord2f(0, 1);
	glNormal3f(1 / 3, -1 / 3, 1 / 3);
	glVertex3f(x + 1.0f * tamaño / 2, y - 0.15f, 0.4f); //Derecha abajo adelante
	glTexCoord2f(1, 1);
	glNormal3f(1 / 3, 1 / 3, 1 / 3); //Derecha arriba adelante 
	glVertex3f(x + 1.0f * tamaño / 2, y + 0.15f, 0.4f);
	glTexCoord2f(1, 0);
	glNormal3f(1 / 3, 1 / 3, -1 / 3);
	glVertex3f(x + 1.0f * tamaño / 2, y + 0.15f, 0.f); // Deerecha arriba atras

	// Cara de izquierda

	glTexCoord2f(1, 0);
	glNormal3f(-1, 1, -1);
	glVertex3f(x - 1.0f * tamaño / 2, y + 0.15f, 0.f); // Izquierda arriba atras
	glTexCoord2f(1, 1);
	glNormal3f(-1, 1, 1);
	glVertex3f(x - 1.0f * tamaño / 2, y + 0.15f, 0.4f); // Izquierda arriba adelante
	glTexCoord2f(0, 1);
	glNormal3f(-1, -1, 1);
	glVertex3f(x - 1.0f * tamaño / 2, y - 0.15f, 0.4f); // Izquierda abajo adelante
	glTexCoord2f(0, 0);
	glNormal3f(-1, -1, -1);
	glVertex3f(x - 1.0f * tamaño / 2, y - 0.15f, 0.f); // Izquierda abajo atras

	// Cara de abajo

	glTexCoord2f(1, 1);
	glNormal3f(1, -1, 1);
	glVertex3f(x + 1.0f * tamaño / 2, y - 0.15f, 0.4f); // Derecha abajo adelante
	glTexCoord2f(1, 0);
	glNormal3f(1, -1, -1);
	glVertex3f(x + 1.0f * tamaño / 2, y - 0.15f, 0.f); // Derecha abajo atras
	glTexCoord2f(0, 0);
	glNormal3f(-1, -1, -1);
	glVertex3f(x - 1.0f * tamaño / 2, y - 0.15f, 0.f); // izquierda abajo atras
	glTexCoord2f(0, 1);
	glNormal3f(-1, -1, 1);
	glVertex3f(x - 1.0f * tamaño / 2, y - 0.15f, 0.4f); // izquierda abajo adelante
	
	// Cara de arriba
	
	glTexCoord2f(1, 1);
	glNormal3f(1, 1, 1);
	glVertex3f(x + 1.0f * tamaño / 2, y + 0.15f, 0.4f); // deerecha arriba adelante
	glTexCoord2f(1, 0);
	glNormal3f(1, 1, -1);
	glVertex3f(x + 1.0f * tamaño / 2, y + 0.15f, 0.f); // derecha arriba atras
	glTexCoord2f(0, 0);
	glNormal3f(-1, 1, -1);
	glVertex3f(x - 1.0f * tamaño / 2, y + 0.15f, 0.f); // izquierda arriba atras
	glTexCoord2f(0, 1);
	glNormal3f(-1, 1, 1);
	glVertex3f(x - 1.0f * tamaño / 2, y + 0.15f, 0.4f); // izquierda arriba adelante

	// Cara de atras

	glTexCoord2f(1, 1);
	glNormal3f(1, 1, -1);
	glVertex3f(x + 1.0f * tamaño / 2, y + 0.15f, 0.f); // derecha arriba atras
	glTexCoord2f(1,0);
	glNormal3f(1, -1, -1);
	glVertex3f(x + 1.0f * tamaño / 2, y - 0.15f, 0.f); //derecha abajo atras
	glTexCoord2f(0, 0);
	glNormal3f(-1, -1, -1);
	glVertex3f(x - 1.0f * tamaño / 2, y - 0.15f, 0.f); //izquierda abajo atras
	glTexCoord2f(0, 1);
	glNormal3f(-1, 1, -1);
	glVertex3f(x - 1.0f * tamaño / 2, y + 0.15f, 0.f); //izquierda arriba atras

	// Cara de adelante

	glTexCoord2f(1, 1);
	glNormal3f(1, 1, 1);
	glVertex3f(x + 1.0f * tamaño / 2, y + 0.15f, 0.4f); //derecha arriba adelante
	glTexCoord2f(1, 0);
	glNormal3f(1, -1, 1);
	glVertex3f(x + 1.0f * tamaño / 2, y - 0.15f, 0.4f); //derecha abajo adelante
	glTexCoord2f(0, 0);
	glNormal3f(-1, -1, 1);
	glVertex3f(x - 1.0f * tamaño / 2, y - 0.15f, 0.4f); // izquierda abajo adelante
	glTexCoord2f(0, 1);
	glNormal3f(-1, 1, 1);
	glVertex3f(x - 1.0f * tamaño / 2, y + 0.15f, 0.4f); // izquierda arriba adelante
	glEnd();

}

void dibujar_plataformas() {
	int size = plataformas.size();
	for (int i = 0; i < size; i++) {
		Plataforma plat = plataformas.front();
		plataformas.pop(); 
		dibujar_plataforma(plat.x, plat.y, plat.tamaño); //y - floor(posyWorld)
		plataformas.push(plat);
	}
}

void actualizo_plataformas() {
	Plataforma plat_vieja= plataformas.front();
	if (plat_vieja.y < -posyWorld - 5) {
		plataformas.pop();
		Plataforma plat;
		plat.x = (rand() % 5) - 2;
		plat.y = plat_vieja.y + (1.5*15);
		plat.tamaño = 1.2f;
		plataformas.push(plat);
	}
}

void dibujarObjetos() {
	glEnable(GL_LIGHTING);
	glPushMatrix();
		glTranslatef(0, posyWorld, -0.2);
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
				bool xizq = plat.x - 1.0f * plat.tamaño / 2 < posx;
				bool xder = posx < plat.x + 1.0f * plat.tamaño / 2;
				bool yizq = posyWorld + plat.y + 0.05f < posy;
				bool yder = posy < posyWorld + plat.y + 0.3f;
				bool res = (xizq && xder) && (yizq && yder);
				
				if (res) {
					/*cout << "i=" << i << endl;
					cout << "posx=" << posx << endl;
					cout << "posy=" << posy << endl;
					cout << "plat.x=" << plat.x << endl;
					cout << "plat.y=" << plat.y << endl;
					cout << "plat.tamaño=" << plat.tamaño << endl;
					cout << "posyWorld=" << posyWorld << endl;
					cout << "plat.x - 1.0f * plat.tamaño / 2=" << plat.x - 1.0f * plat.tamaño / 2 << endl;
					cout << "plat.x + 1.0f * plat.tamaño / 2=" << plat.x + 1.0f * plat.tamaño / 2 << endl;
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
		posyWorld -= (deltatime.count() * (2 + ((posy + 7) / 10))) * velocidad_multiplicador;
	}
	else {
		velocidadY = velocidadInicialY*velocidad_multiplicador;
		velocidadX = velocidadInicialX;
	}
	if (strifingLeft) {
		posx -= velocidadX * deltatime.count();
	}
	if (strifingRight) {
		posx += velocidadX * deltatime.count();
	}
	if (-7 > posy){
		mode = GAME_OVER;
	}
}

void end_game() {
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

	//dibujo

	glBindTexture(GL_TEXTURE_2D, texturas_menu[18]);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex2i(190, 110);
	glTexCoord2f(0, 1);
	glVertex2i(190, 370);
	glTexCoord2f(1, 1);
	glVertex2i(450, 370);
	glTexCoord2f(1, 0);
	glVertex2i(450, 110);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, texturas_menu[19]);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex2i(220, 60);
	glTexCoord2f(0, 1);
	glVertex2i(220, 140);
	glTexCoord2f(1, 1);
	glVertex2i(420, 140);
	glTexCoord2f(1, 0);
	glVertex2i(420, 60);
	glEnd();

	//recupero contexto

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	if (wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	// Re inicializo valores para otra jugada

	posx = 0, posy = -1.4;
	posyWorld = 0;
	posyWorld_delta = 0;
	jumping = false;
	falling = false;
	velocidadX = velocidadInicialX;
	velocidadY = velocidadInicialY;
	strifingRight = false;
	strifingLeft = false;
	plataformas = {};
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
		if (facetado) {
			glShadeModel(GL_FLAT);
		}
		else{
			glShadeModel(GL_SMOOTH);
		}
		score = max(score, ((int)floor(-posyWorld)));

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

		if ((mode == MAIN_MENU) || (mode == SETTINGS_MODE) || (mode == PAUSA) || (mode == NIVELES_MODE) || (mode == LUCES_MODE)) {
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
		else if (mode == GAME_OVER) {
			textOnAux = textOn;
			textOn = false;
			draw_background();
			textOn = textOnAux;
			end_game();
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