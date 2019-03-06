 
#include "arkanoPi_1.h"
#include "tmr.h"
int matrizFilas[7]={0, 1, 5, 6, 4, 7, 23};
int boton_izq = boton_izquierdo;
int boton_der = boton_derecho;

static tmr_t* timer_buzzer;


enum fsm_state {
  WAIT_START=1,
  WAIT_PUSH=0,
  WAIT_END=2,
  WAIT_PAUSE =3,
  WAIT_LIVES = 4,
  WAIT_SPEED = 5,
  };

int columna = 0;
int nVidas = 0;
int timeout2 = 0;
static volatile tipo_juego juego;
volatile int flags = 0;
volatile int izq = 0;
volatile int der = 0;

int difcultad[8][8]= {
		{1,1,1,0,0,1,1,0},
		{1,0,0,0,0,1,0,1},
		{1,1,1,0,0,1,0,1},
		{1,0,0,0,0,1,0,1},
		{1,0,0,0,0,1,1,0},
		{0,0,0,0,0,0,0,0},
		{0,1,0,0,0,0,1,0},
		{1,1,0,0,0,0,1,1},
	};

int tresVidas[8][8]= {
			{0,0,0,0,0,0,0,0},
			{0,0,1,1,1,1,0,0},
			{0,0,0,0,0,1,1,0},
			{0,0,0,1,1,1,0,0},
			{0,0,0,0,0,1,1,0},
			{0,0,0,0,0,1,1,0},
			{0,0,1,1,1,1,0,0},
			{0,0,0,0,0,0,0,0},
		};

int dosVidas[8][8]= {
			{0,0,0,0,0,0,0,0},
			{0,0,1,1,1,1,0,0},
			{0,1,1,0,0,1,1,0},
			{0,0,0,0,0,1,1,0},
			{0,0,1,1,1,1,0,0},
			{0,1,1,0,0,0,0,0},
			{0,1,1,1,1,1,1,0},
			{0,0,0,0,0,0,0,0},
		};

int unaVida[8][8]= {
			{0,0,0,0,0,0,0,0},
			{0,0,0,1,1,0,0,0},
			{0,0,1,1,1,0,0,0},
			{0,0,0,1,1,0,0,0},
			{0,0,0,1,1,0,0,0},
			{0,0,0,1,1,0,0,0},
			{0,0,1,1,1,1,0,0},
			{0,0,0,0,0,0,0,0},
		};

int pausa[8][8]= {
		{0,0,0,0,0,0,0,0},
		{0,1,1,0,0,1,1,0},
		{0,1,1,0,0,1,1,0},
		{0,1,1,0,0,1,1,0},
		{0,1,1,0,0,1,1,0},
		{0,1,1,0,0,1,1,0},
		{0,1,1,0,0,1,1,0},
		{0,0,0,0,0,0,0,0},
	};

// espera hasta la próxima activación del reloj
void delay_until (unsigned int next) {
	unsigned int now = millis();

	if (next > now) {
		delay (next - now);
    }
}

// Funciones de entrada
int CompruebaBotonIzquierdo(fsm_t* this) {
	int result;

	piLock (FLAGS_KEY);
	result = (flags & FLAG_RAQUETA_IZQUIERDA);
	piUnlock (FLAGS_KEY);
	return result;
}
int CompruebaBotonDerecha(fsm_t* this) {
	int result;

	piLock (FLAGS_KEY);
	result = (flags & FLAG_RAQUETA_DERECHA);
	piUnlock (FLAGS_KEY);
	return result;
}
int CompruebaTeclaPelota(fsm_t* this) {
	int result;

	piLock (FLAGS_KEY);
	result = (flags & FLAG_PELOTA);
	piUnlock (FLAGS_KEY);
	return result;
}
int CompruebaTeclaPulsada(fsm_t* this) {
	int result;

	piLock (FLAGS_KEY);
	result = (flags & FLAG_TECLA);
	piUnlock (FLAGS_KEY);
	return result;
}
int CompruebaTeclaFinalJuego(fsm_t* this) {
	int result;

	piLock (FLAGS_KEY);
	result = (flags & FLAG_FINAL_JUEGO);
	piUnlock (FLAGS_KEY);
	return result;
}
int CompruebaJoystick(fsm_t* this) {
	int result;

	piLock (FLAGS_KEY);
	result = (flags & FLAG_JOYSTICK);
	piUnlock (FLAGS_KEY);
	return result;
}

int CompruebaFinalJuego(fsm_t* this) {
	int result;
	piLock (FLAGS_KEY);
	result = (flags & FLAG_FINAL_JUEGO);
	piUnlock (FLAGS_KEY);
	return result;
}
int CompruebaPausa(fsm_t* this){
	int result;
	piLock (FLAGS_KEY);
	result = (flags & FLAG_PAUSA);
	piUnlock (FLAGS_KEY);
	return result;

}
//------------------------------------------------------
// FUNCIONES DE ACCION
//------------------------------------------------------

// void InicializaJuego (void): funcion encargada de llevar a cabo
// la oportuna inicializaciÃ³n de toda variable o estructura de datos
// que resulte necesaria para el desarrollo del juego.
void InicializaJuego (fsm_t* fsm) {
	piLock (FLAGS_KEY);

	flags &= ~FLAG_TECLA;

	piUnlock (FLAGS_KEY);

	piLock (STD_IO_BUFFER_KEY);
	printf("Inicilizamos");
	InicializaArkanoPi((tipo_arkanoPi*)(&(juego.arkanoPi)));
	piUnlock (STD_IO_BUFFER_KEY);

}

// void MueveRaquetaIzquierda (void): funcion encargada de ejecutar
// el movimiento hacia la izquierda contemplado para la raqueta.
// Debe garantizar la viabilidad del mismo mediante la comprobaciÃ³n
// de que la nueva posiciÃ³n correspondiente a la raqueta no suponga
// que Ã©sta rebase o exceda los lÃ­mites definidos para el Ã¡rea de juego
// (i.e. al menos uno de los leds que componen la raqueta debe permanecer
// visible durante todo el transcurso de la partida).
void boton_izq_isr (void){
	int debounceTime = 0;
	piLock (FLAGS_KEY);
	flags |= FLAG_RAQUETA_IZQUIERDA;
	piUnlock (FLAGS_KEY);
	piLock (FLAGS_KEY);
	flags |= FLAG_TECLA;
	piUnlock (FLAGS_KEY);

	if (millis () < debounceTime) {
	debounceTime = millis () + DEBOUNCE_TIME ;
	return;
	}
	while (digitalRead (boton_der) == HIGH) {
	delay (1) ;
	}
	debounceTime = millis () + DEBOUNCE_TIME ;


}

void boton_der_isr (void){
	int debounceTime = 0;
	piLock (FLAGS_KEY);
	flags |= FLAG_RAQUETA_DERECHA;
	piUnlock (FLAGS_KEY);
	piLock (FLAGS_KEY);
	flags |= FLAG_TECLA;
	piUnlock (FLAGS_KEY);

	if (millis () < debounceTime) {
	debounceTime = millis () + DEBOUNCE_TIME ;
	return;
	}
	while (digitalRead (boton_der) == HIGH) {
	delay (1) ;
	}
	debounceTime = millis () + DEBOUNCE_TIME ;

}

void MueveRaquetaIzquierda (fsm_t* fsm) {
	// A completar por el alumno...


	piLock (FLAGS_KEY);
	flags &= ~FLAG_RAQUETA_IZQUIERDA;
	piUnlock (FLAGS_KEY);

	piLock (STD_IO_BUFFER_KEY);

	if (juego.arkanoPi.raqueta.x > -2){
	juego.arkanoPi.raqueta.x -= 1;
	ActualizaPantalla((tipo_arkanoPi*)(&(juego.arkanoPi)));
	}
	piUnlock (STD_IO_BUFFER_KEY);
	izq = 1;

}

// void MueveRaquetaDerecha (void): funciÃ³n similar a la anterior
// encargada del movimiento hacia la derecha.
void MueveRaquetaDerecha (fsm_t* fsm) {
	// A completar por el alumno...

	piLock (FLAGS_KEY);
	flags &= ~FLAG_RAQUETA_DERECHA;
	piUnlock (FLAGS_KEY);

	piLock (STD_IO_BUFFER_KEY);
	if (juego.arkanoPi.raqueta.x < 9){
	juego.arkanoPi.raqueta.x += 1;
	ActualizaPantalla((tipo_arkanoPi*)(&(juego.arkanoPi)));}
	piUnlock (STD_IO_BUFFER_KEY);

	der = 1;

}

// void MovimientoPelota (void): funciÃ³n encargada de actualizar la
// posiciÃ³n de la pelota conforme a la trayectoria definida para Ã©sta.
// Para ello deberÃ¡ identificar los posibles rebotes de la pelota para,
// en ese caso, modificar su correspondiente trayectoria (los rebotes
// detectados contra alguno de los ladrillos implicarÃ¡n adicionalmente
// la eliminaciÃ³n del ladrillo). Del mismo modo, deberÃ¡ tambiÃ©n
// identificar las situaciones en las que se dÃ© por finalizada la partida:
// bien porque el jugador no consiga devolver la pelota, y por tanto Ã©sta
// rebase el lÃ­mite inferior del Ã¡rea de juego, bien porque se agoten
// los ladrillos visibles en el Ã¡rea de juego.
void MovimientoPelota (fsm_t* fsm) {

	piLock (FLAGS_KEY);

	flags &= ~FLAG_PELOTA;

	piUnlock (FLAGS_KEY);

piLock (STD_IO_BUFFER_KEY);
// Pared izquierda
if ((juego.arkanoPi.pelota.x == MATRIZ_ANCHO-1) && (juego.arkanoPi.pelota.xv == 1)){
	juego.arkanoPi.pelota.xv = -1;
	Suena(12,50);
}
// Pared derecha
if ((juego.arkanoPi.pelota.x == 0) && (juego.arkanoPi.pelota.xv == -1)){
	juego.arkanoPi.pelota.xv = 1;
	Suena(12,50);
}
// Techo
if ((juego.arkanoPi.pelota.y == 0) && (juego.arkanoPi.pelota.yv == -1)){
	juego.arkanoPi.pelota.yv = 1;
	Suena(12,50);
}
// Raqueta
if ((juego.arkanoPi.pelota.y == (juego.arkanoPi.raqueta.y)-1)&& (juego.arkanoPi.pelota.yv == 1)){
	// Caso 1
	if ((juego.arkanoPi.pelota.x == (juego.arkanoPi.raqueta.x)+3 )&& (juego.arkanoPi.pelota.xv == -1)){
		juego.arkanoPi.pelota.xv = 1;
		juego.arkanoPi.pelota.yv = -1;
	}
	// Caso 3
	if ((juego.arkanoPi.pelota.x == (juego.arkanoPi.raqueta.x) +1)&& (juego.arkanoPi.pelota.xv == -1)){
			juego.arkanoPi.pelota.xv = -1;
			juego.arkanoPi.pelota.yv = -1;
		}
	// Caso 4
	if ((juego.arkanoPi.pelota.x == (juego.arkanoPi.raqueta.x) +2 )&& (juego.arkanoPi.pelota.xv == 0)){
			juego.arkanoPi.pelota.xv = 1;
			juego.arkanoPi.pelota.yv = -1;
		}
	// Caso 5
	if ((juego.arkanoPi.pelota.x == (juego.arkanoPi.raqueta.x)  +1)&& (juego.arkanoPi.pelota.xv == 0)){
			juego.arkanoPi.pelota.xv = 0;
			juego.arkanoPi.pelota.yv = -1;
		}
	// Caso 6
	if ((juego.arkanoPi.pelota.x == (juego.arkanoPi.raqueta.x)  )&& (juego.arkanoPi.pelota.xv == 0)){
			juego.arkanoPi.pelota.xv = -1;
			juego.arkanoPi.pelota.yv = -1;
		}
	// Caso 7
	if ((juego.arkanoPi.pelota.x == (juego.arkanoPi.raqueta.x) +1 )&& (juego.arkanoPi.pelota.xv == 1)){
			juego.arkanoPi.pelota.yv = -1;
		}
	// Caso 8
	if ((juego.arkanoPi.pelota.x == (juego.arkanoPi.raqueta.x)  )&& (juego.arkanoPi.pelota.xv == 1)){
			juego.arkanoPi.pelota.xv = 0;
			juego.arkanoPi.pelota.yv = -1;
		}
	// Caso 9
	if ((juego.arkanoPi.pelota.x == (juego.arkanoPi.raqueta.x) -1 )&& (juego.arkanoPi.pelota.xv == 1)){
			juego.arkanoPi.pelota.xv = -1;
			juego.arkanoPi.pelota.yv = -1;
		}
	// Caso 2
	if ((juego.arkanoPi.pelota.x == (juego.arkanoPi.raqueta.x)+2)&& (juego.arkanoPi.pelota.xv == -1)){
			juego.arkanoPi.pelota.xv = 0;
			juego.arkanoPi.pelota.yv = -1;
		}
	Suena(90,50);
}
// Si hay ladrillos en la proxima posicion de la pelota
if (juego.arkanoPi.ladrillos.matriz[juego.arkanoPi.pelota.x+juego.arkanoPi.pelota.xv][juego.arkanoPi.pelota.y+juego.arkanoPi.pelota.yv] == 1){
	int i;
	int j;
	for( i=0;i<MATRIZ_ANCHO;i++) {
			for( j=0;j<MATRIZ_ALTO;j++) {
				if	(juego.arkanoPi.ladrillos.matriz[i][j] == 1) {
			// Caso 1 (abajo)
			if	((juego.arkanoPi.pelota.y == j+1) && (juego.arkanoPi.pelota.x == i) && (juego.arkanoPi.pelota.yv == -1) && (juego.arkanoPi.pelota.xv == 0)){
				juego.arkanoPi.ladrillos.matriz[i][j] = 0;
				juego.arkanoPi.pelota.yv = +1;
			}
			}

			// Caso 2 (golpea abajo-derecha)
			if ((juego.arkanoPi.pelota.y == j+1) && (juego.arkanoPi.pelota.x == i-1)&& (juego.arkanoPi.pelota.yv == -1) && (juego.arkanoPi.pelota.xv == 1)) {
				juego.arkanoPi.ladrillos.matriz[i][j] = 0;
				juego.arkanoPi.pelota.yv = 1;

			}
			// Caso 3 (golpea abajo-izquierda)
			if ((juego.arkanoPi.pelota.y == j+1) && (juego.arkanoPi.pelota.x == i+1)&& (juego.arkanoPi.pelota.yv == -1) && (juego.arkanoPi.pelota.xv == -1)) {
				juego.arkanoPi.ladrillos.matriz[i][j] = 0;
				juego.arkanoPi.pelota.yv = 1;
			}
			// Caso 4 Arriba izquierda
			if ((juego.arkanoPi.pelota.y == j-1) && (juego.arkanoPi.pelota.x == i+1)&& (juego.arkanoPi.pelota.yv == -1) && (juego.arkanoPi.pelota.xv == -1)) {
				juego.arkanoPi.ladrillos.matriz[i][j] = 0;
				juego.arkanoPi.pelota.yv = 1;
			}
			// Caso 4 Arriba derecha
			if ((juego.arkanoPi.pelota.y == j-1) && (juego.arkanoPi.pelota.x == i-1)&& (juego.arkanoPi.pelota.yv == -1) && (juego.arkanoPi.pelota.xv == -1)) {
				juego.arkanoPi.ladrillos.matriz[i][j] = 0;
				juego.arkanoPi.pelota.yv = 1;
			}
		}
	}
	Suena(12,50);
}
	// Asignamos la nueva posicion de la pelota
		juego.arkanoPi.pelota.x += juego.arkanoPi.pelota.xv;
		juego.arkanoPi.pelota.y += juego.arkanoPi.pelota.yv;

		ActualizaPantalla((tipo_arkanoPi*)(&(juego.arkanoPi)));

		switch(nVidas){
		case 2:
			lightMatrix(tresVidas);
			parpadeaMatrix(0);
			break;
		case 1:
			lightMatrix(dosVidas);
			parpadeaMatrix(0);
			break;
		case 0:
			lightMatrix(unaVida);
			parpadeaMatrix(1);
		}

	if (juego.arkanoPi.pelota.y == 6){
        
        if (nVidas==0){
		piLock (FLAGS_KEY);
		flags |= FLAG_FINAL_JUEGO;
		piUnlock (FLAGS_KEY);
        }
        juego.arkanoPi.pelota.x = (MATRIZ_ANCHO/2) - 1;
        juego.arkanoPi.pelota.y = 2;
        juego.arkanoPi.pelota.xv = 0;
        juego.arkanoPi.pelota.yv = 1;
        nVidas--;
	}
piUnlock (STD_IO_BUFFER_KEY);

}
void PausaJuego (fsm_t* fsm){
	piLock (FLAGS_KEY);
	flags &= ~FLAG_PAUSA;
	piUnlock (FLAGS_KEY);

	piLock (FLAGS_KEY);
	tmr_startms(juego.temporizador2, 0);
	lightMatrix(pausa);
	parpadeaMatrix(2);
	piUnlock (FLAGS_KEY);

}

void ReanudaIzq (fsm_t* fsm){

	piLock (FLAGS_KEY);
	flags &= ~FLAG_RAQUETA_IZQUIERDA;
	piUnlock (FLAGS_KEY);

	piLock (STD_IO_BUFFER_KEY);
	izq = 1;
	piUnlock (STD_IO_BUFFER_KEY);
	}

void ReanudaDer (fsm_t* fsm){

	piLock (FLAGS_KEY);
	flags &= ~FLAG_RAQUETA_DERECHA;
	piUnlock (FLAGS_KEY);

	piLock (STD_IO_BUFFER_KEY);
	der = 1;
	piUnlock (STD_IO_BUFFER_KEY);
	}

void ReanudaJuego (fsm_t* fsm){
	piLock (FLAGS_KEY);
	flags &= ~FLAG_PAUSA;
	piUnlock (FLAGS_KEY);

	piLock (STD_IO_BUFFER_KEY);
	tmr_startms(juego.temporizador2, timeout2);
	piUnlock (STD_IO_BUFFER_KEY);
}

// void FinalJuego (void): funciÃ³n encargada de mostrar en la ventana de
// terminal los mensajes necesarios para informar acerca del resultado del juego.
void FinalJuego (fsm_t* fsm) {
	// A completar por el alumno...
	piLock (FLAGS_KEY);
	flags &= ~FLAG_FINAL_JUEGO;
	piUnlock (FLAGS_KEY);

	piLock (STD_IO_BUFFER_KEY);
	printf("Has perdido");
	lightMatrix(difcultad);
	parpadeaMatrix(0);
	piUnlock (STD_IO_BUFFER_KEY);
}

//void ReseteaJuego (void): funciÃ³n encargada de llevar a cabo la
// reinicializaciÃ³n de cuantas variables o estructuras resulten
// necesarias para dar comienzo a una nueva partida.
void ReseteaJuego (fsm_t* fsm) {
	// A completar por el alumno...
	piLock (FLAGS_KEY);

	flags &= ~FLAG_TECLA;

	piUnlock (FLAGS_KEY);
}

void VelocidadRapida(fsm_t* fsm){
    timeout2 = 300;
	tmr_startms(juego.temporizador2, timeout2);
    InicializaJuego(fsm);

}

void VelocidadLenta(fsm_t* fsm){
    timeout2 = 500;
	tmr_startms(juego.temporizador2, timeout2);
    InicializaJuego(fsm);
}

void UnaVida(fsm_t* fsm){
    nVidas=0;
}

void TresVidas(fsm_t* fsm){
    nVidas=2;
}

void Suena (int freq, int dc) {
	softPwmCreate(25, 0, 50);
	softPwmWrite(25, freq);
	tmr_startms(timer_buzzer, 200);
}

void ActualizaColumna(void){


 switch(columna){
 case 0:
	 digitalWrite (14,LOW);
	 digitalWrite (17,LOW);
	 digitalWrite (8,LOW);
	 digitalWrite (22,LOW);

	 break;
 case 1:
	 digitalWrite (14,HIGH);
	 digitalWrite (17,LOW);
	 digitalWrite (8,LOW);
	 digitalWrite (22,LOW);
	 break;
 case 2:
 	 digitalWrite (14,LOW);
 	 digitalWrite (17,HIGH);
 	 digitalWrite (8,LOW);
 	 digitalWrite (22,LOW);
 	 break;
 case 3:
 	 digitalWrite (14,HIGH);
 	 digitalWrite (17,HIGH);
 	 digitalWrite (8,LOW);
 	 digitalWrite (22,LOW);
 	 break;
 case 4:
 	 digitalWrite (14,LOW);
 	 digitalWrite (17,LOW);
 	 digitalWrite (8,HIGH);
 	 digitalWrite (22,LOW);
 	 break;
 case 5:
 	 digitalWrite (14,HIGH);
 	 digitalWrite (17,LOW);
 	 digitalWrite (8,HIGH);
 	 digitalWrite (22,LOW);
 	 break;
 case 6:
 	 digitalWrite (14,LOW);
 	 digitalWrite (17,HIGH);
 	 digitalWrite (8,HIGH);
 	 digitalWrite (22,LOW);
 	 break;
 case 7:
 	 digitalWrite (14,HIGH);
 	 digitalWrite (17,HIGH);
 	 digitalWrite (8,HIGH);
 	 digitalWrite (22,LOW);
 	 break;
 case 8:
 	 digitalWrite (14,LOW);
 	 digitalWrite (17,LOW);
 	 digitalWrite (8,LOW);
 	 digitalWrite (22,HIGH);
 	 break;
 case 9:
 	 digitalWrite (14,HIGH);
 	 digitalWrite (17,LOW);
 	 digitalWrite (18,LOW);
 	 digitalWrite (22,HIGH);
 	 break;
 }
}

void timer_pausa (union sigval value) {
	if ((der == 1) && (izq == 1)){
		piLock (FLAGS_KEY);
		flags |= FLAG_PAUSA;
		piUnlock (FLAGS_KEY);
		}
	izq = 0;
	der = 0;
	tmr_startms(juego.temporizador3, TIMEOUT3);
}

void timer_isr1 (union sigval value) {
	piLock (FLAGS_KEY);
	flags |= FLAG_PELOTA;
	piUnlock (FLAGS_KEY);
	//Empieza la cuenta de nuevo.
	tmr_startms(juego.temporizador2, timeout2);
}

void timer_sonido (union sigval value) {
	softPwmStop(25);
	tmr_startms(timer_buzzer, 0);
}

void timer_isr (union sigval value) {

	int fila;
	ActualizaColumna();
	for(fila=0;fila<7;fila++) {
		if(juego.arkanoPi.pantalla.matriz[columna][fila]==1){
			digitalWrite (matrizFilas[fila], 0);}
		else {
			digitalWrite (matrizFilas[fila], 1);
		}
	}
	//Actualizar la columna a iluminar
	if(columna==9){
		columna=0;
	}
	else {
		columna++;
	}
	//Empieza la cuenta de nuevo.
	tmr_startms(juego.temporizador1, TIMEOUT);

}

//------------------------------------------------------
// FUNCIONES DE INICIALIZACION
//------------------------------------------------------

// int systemSetup (void): procedimiento de configuracion del sistema.
// RealizarÃ¡, entra otras, todas las operaciones necesarias para:
// configurar el uso de posibles librerÃ­as (e.g. Wiring Pi),
// configurar las interrupciones externas asociadas a los pines GPIO,
// configurar las interrupciones periÃ³dicas y sus correspondientes temporizadores,
// crear, si fuese necesario, los threads adicionales que pueda requerir el sistema
int system_setup (void) {
	int x = 0;

	piLock (STD_IO_BUFFER_KEY);

	// sets up the wiringPi library
	if (wiringPiSetupGpio () < 0) {
		printf ("Unable to setup wiringPi\n");
		piUnlock (STD_IO_BUFFER_KEY);
		return -1;
    }

	// Lanzamos thread para exploracion del teclado convencional del PC
	x = piThreadCreate (thread_explora_teclado);

	if (x != 0) {
		printf ("it didn't start!!!\n");
		piUnlock (STD_IO_BUFFER_KEY);
		return -1;
    }

	piUnlock (STD_IO_BUFFER_KEY);

	return 1;
}

void fsm_setup(fsm_t* juego_fsm) {
	piLock (FLAGS_KEY);
	flags = 0;
	piUnlock (FLAGS_KEY);

	InicializaJuego(juego_fsm);
	piLock (STD_IO_BUFFER_KEY);
	printf("\nPulse un boton para comenzar\n");
	piUnlock (STD_IO_BUFFER_KEY);
}

PI_THREAD (thread_explora_teclado) {
// int teclaPulsada;

while (1) {
	delay(10);
	piLock (STD_IO_BUFFER_KEY);
		piUnlock (STD_IO_BUFFER_KEY);
	}
}
int main () {
	unsigned int next;
	// Creamos el temporizador

	juego.temporizador1= tmr_new (timer_isr);
	juego.temporizador2= tmr_new (timer_isr1);
	juego.temporizador3= tmr_new (timer_pausa);
	timer_buzzer = tmr_new (timer_sonido);


	wiringPiSetupGpio();
	pinMode (0, OUTPUT) ;
	pinMode (1, OUTPUT) ;
	pinMode (5, OUTPUT) ;
	pinMode (6, OUTPUT) ;
	pinMode (4, OUTPUT) ;
	pinMode (7, OUTPUT) ;
	pinMode (23, OUTPUT) ;
	pinMode (14, OUTPUT) ;
	pinMode (17, OUTPUT) ;
	pinMode (8, OUTPUT) ;
	pinMode (22, OUTPUT) ;

	pinMode (25, OUTPUT);

	pinMode (16, INPUT);
	pinMode (19, INPUT);

	onMatrix();
	lightMatrix(difcultad);

	wiringPiISR (16, INT_EDGE_FALLING, boton_izq_isr);
	wiringPiISR (19, INT_EDGE_FALLING, boton_der_isr);
	pullUpDnControl(16, PUD_DOWN);
	pullUpDnControl(19, PUD_DOWN);


	// declaro pin 0 como salida
		// Maquina de estados: lista de transiciones
		// {EstadoOrigen,FunciónDeEntrada,EstadoDestino,FunciónDeSalida}
		fsm_trans_t juego_tabla[] = {
				{ WAIT_LIVES, CompruebaBotonIzquierdo,  WAIT_SPEED, TresVidas },
                { WAIT_LIVES, CompruebaBotonDerecha, WAIT_SPEED,   UnaVida },
                { WAIT_SPEED, CompruebaBotonIzquierdo, WAIT_PUSH,  VelocidadLenta  },
                { WAIT_SPEED, CompruebaBotonDerecha, WAIT_PUSH,  VelocidadRapida  },
				{ WAIT_PUSH, CompruebaBotonIzquierdo, WAIT_PUSH,   MueveRaquetaIzquierda },
				{ WAIT_PUSH, CompruebaBotonDerecha, WAIT_PUSH,   MueveRaquetaDerecha },
				{ WAIT_PUSH, CompruebaTeclaPelota, WAIT_PUSH,   MovimientoPelota },
				{ WAIT_PUSH, CompruebaFinalJuego, WAIT_END,   FinalJuego },
				{ WAIT_END, CompruebaTeclaPulsada, WAIT_LIVES,   ReseteaJuego },
				{ WAIT_PUSH, CompruebaPausa, WAIT_PAUSE, PausaJuego},
				{ WAIT_PAUSE, CompruebaBotonIzquierdo, WAIT_PAUSE, ReanudaIzq},
				{ WAIT_PAUSE, CompruebaBotonDerecha, WAIT_PAUSE, ReanudaDer},
				{ WAIT_PAUSE, CompruebaPausa, WAIT_PUSH, ReanudaJuego},
				{ -1, NULL, -1, NULL },
		};
		fsm_t* juego_fsm = fsm_new (WAIT_LIVES, juego_tabla, NULL);
		// Configuracion e inicializacion del sistema
			system_setup();
			fsm_setup (juego_fsm);
			tmr_startms(juego.temporizador1, TIMEOUT);
			tmr_startms(juego.temporizador3, TIMEOUT3);

			PintaMensajeInicialPantalla((tipo_pantalla*) (&(juego.arkanoPi.pantalla)));


			next = millis();
			while (1) {
				fflush(stdout);
				fsm_fire (juego_fsm);
				next += CLK_MS;
				delay_until (next);
			}
			tmr_destroy (juego.temporizador1);
			tmr_destroy (juego.temporizador2);
			tmr_destroy (juego.temporizador3);
			fsm_destroy (juego_fsm);
}

