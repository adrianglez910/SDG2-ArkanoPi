#ifndef _ARKANOPI_H_
#define _ARKANOPI_H_

#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include "kbhit.h"
//#include <conio.h>
#include "fsm.h"
#include "tmr.h"
#include <wiringPi.h>
#include <softPwm.h>
#include "matrix.h"

#define CLK_MS 10 // PERIODO DE ACTUALIZACION DE LA MAQUINA ESTADOS
#define TIMEOUT 1
//#define TIMEOUT2 500
#define TIMEOUT3 50
#define TIMEOUT4 200
//Flags del sistema
#define FLAG_RAQUETA_IZQUIERDA		0x01
#define FLAG_RAQUETA_DERECHA		0x02
#define FLAG_PELOTA					0x04
#define FLAG_TECLA					0x08
#define FLAG_FINAL_JUEGO			0x10
#define FLAG_JOYSTICK				0x20
#define FLAG_TIMER					0x40
#define FLAG_PAUSA					0x80

#define	FLAGS_KEY	1
#define	STD_IO_BUFFER_KEY	2
#define boton_izquierdo 16;
#define boton_derecho 19;
#define DEBOUNCE_TIME 200;

#include "arkanoPiLib.h"

//#define WAIT_START 0
//#define WAIT_PUSH 1
//#define WAIT_END 2


typedef struct {
	tipo_arkanoPi arkanoPi;
	tmr_t* temporizador1; // pantalla
	tmr_t* temporizador2; // pelota
	tmr_t* temporizador3; // pausa
	tmr_t* temporizador4; // sonido
	char teclaPulsada;
} tipo_juego;

//Funciones de entrada
int CompruebaBotonIzquierdo(fsm_t* this);
int CompruebaBotonDerecha(fsm_t* this);
int CompruebaTeclaPelota(fsm_t* this);
int CompruebaTeclaPulsada(fsm_t* this);
int CompruebaFinalJuego(fsm_t* this);
int CompruebaJoystick(fsm_t* this);
int CompruebaPausa(fsm_t* this);



//------------------------------------------------------
// FUNCIONES DE ACCION
//------------------------------------------------------

void InicializaJuego (fsm_t*);
void MueveRaquetaIzquierda (fsm_t*);
void MueveRaquetaDerecha (fsm_t*);
void MovimientoPelota (fsm_t*);
void FinalJuego (fsm_t*);
void ReseteaJuego (fsm_t*);
void PausaJuego (fsm_t*);


//------------------------------------------------------
// FUNCIONES DE INICIALIZACION
//------------------------------------------------------
//int systemSetup (void);

// Thread  de gestion de teclado
PI_THREAD (thread_explora_teclado);
void AtualizaColumna(void);
void Suena (int freq, int dc);

#endif /* ARKANOPI_H_ */
