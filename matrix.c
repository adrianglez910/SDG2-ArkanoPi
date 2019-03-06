/*
 * Matrix.c
 *
 *  Created on: 12 may. 2017
 *      Author: Javi
 */
#include "matrix.h"

/* Concatenates ints
 * int x, int y
 * Returns x concatenated with y
 */
unsigned concatenate(unsigned x, unsigned y) {
    unsigned pow = 10;
    while(y >= pow)
        pow *= 10;
    return x * pow + y;
}

/* Converts the row of the matrix into the row of the screen
 * int row, row of the matrix
 * Returns the equivalent row of the screen
 */
int convertRow(int row){
	switch(row) {
	case 0:
		return 0;
	case 1:
		return 2;
	case 2:
		return 4;
	case 3:
		return 6;
	case 4:
		return 8;
	case 5:
		return 10;
	case 6:
		return 12;
	case 7:
		return 14;

	}return 0;
}

/* Converts a binary number into a decimal one.
 * int binary, binary number
 * Returns decimal number
 */
int convertDec(int binary){
	 int remainder;
	 int e = 1;
	 long decimal = 0;
	    while(binary != 0) {
	        remainder = binary%10;
	        binary = binary/10;
	        decimal = decimal + (remainder*e);
	        e=2*e;
	    }
	    return decimal;
	}

/* Initialises the led matrix
 * Oscillator, display and minimum brightness *
 */
void onMatrix(void){
	int matrix = wiringPiI2CSetup(0x70);
	wiringPiI2CWriteReg8(matrix, 0x21, 0x00);
	wiringPiI2CWriteReg8(matrix, 0x81, 0x00);
	wiringPiI2CWriteReg8(matrix, 0xe0, 0x00);
}

/* Turns off the led display
 * Oscillator and display *
 */
void offMatrix(void){
	int matrix = wiringPiI2CSetup(0x70);
	wiringPiI2CWriteReg8(matrix, 0x20, 0x00);
	wiringPiI2CWriteReg8(matrix, 0x80, 0x00);
}

/* Illuminates the screen
 * int screen[8][8], matrix with the motif
 */
void lightMatrix(int screen[8][8]){
	int matrix = wiringPiI2CSetup(0x70);
	int i,j;
	int data, row;

	for(j=0;j<8;j++){								// Rows
		data = screen[j][0]; 						// First led
		for(i=7;i>0;i--){							// Cols
			data = concatenate(data,screen[j][i]);	// All cols are concatenated into a single binary int
		}
		row = convertRow(j);						// Conversion of the matrix row into the screen row
	    data = convertDec(data);					// Conversion binary to decimal
		wiringPiI2CWriteReg8(matrix, row, data);
	}
}

/* Makes the screen blink
 * int valor, 0 no blink, 1 very fast, 3 slow
 */
void parpadeaMatrix(int valor){
	int matrix = wiringPiI2CSetup(0x70);
	switch (valor) {
	case 0:
		wiringPiI2CWriteReg8(matrix, 0x81, 0x00);
		break;
	case 1:
		wiringPiI2CWriteReg8(matrix, 0x83, 0x00);
		break;
	case 2:
		wiringPiI2CWriteReg8(matrix, 0x85, 0x00);
		break;
	case 3:
		wiringPiI2CWriteReg8(matrix, 0x87, 0x00);
		break;
	}
}

