/* ------------------------------------------------------------------------- */
/* File: gps.c ------------------------------------------------------------- */
/* Author: João Spranger - a2716@alunos.ipca.pt ---------------------------- */
/* Course: Informática - Branch: Industrial -------------------------------- */
/* Class: Projecto - 2009/2010 --------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* This file is a source-code file, and contains: -------------------------- */
/* @ Gps interaction methods source-code ----------------------------------- */
/* ------------------------------------------------------------------------- */

// Include Project Libraries
#include "../definitions.h"
#include "../procedures.h"
#include "../usart/usart.h"
#include "gps.h"

// Global variables
const unsigned char gpgga[] PROGMEM = "$GPGGA";

PGM_P gpsstaticdata[] PROGMEM =
{
	gpgga,		// 0
};

/* ------------------------------- PROTOTYPES ------------------------------ */
void toggleInput(void);
/* ------------------------------------------------------------------------- */

/* ---------------------------- PUBLIC METHODS ----------------------------- */
unsigned char *retrieveGPS()
{
	unsigned char *gps_string = calloc(21, sizeof(unsigned char));

	uint8_t test = FALSE;

	while (test == FALSE)
	{
		toggleInput();
		unsigned char *gps_string_temp = retrieveUSARTGPS();
		toggleInput();

 		// DOR error prevention, read char if any left in buffer and discard
		if ((UCSR0A & (1<<RXC0)) != 0) { unsigned char c = UDR0; }

		if (strlen(gps_string_temp) > 38)
		{
			unsigned char *gps_test = returnSubStr(0, 5, gps_string_temp);

			if (strcmp_P(gps_test, (PGM_P)pgm_read_word(&(gpsstaticdata[0]))) == 0)
			{
				unsigned char *gps_temp = calloc(21, sizeof(unsigned char));

				gps_temp[0] = gps_string_temp[28];
				gps_temp[1] = '-';
				gps_temp[2] = gps_string_temp[18];
				gps_temp[3] = gps_string_temp[19];
				gps_temp[4] = ',';
				gps_temp[5] = gps_string_temp[20];
				gps_temp[6] = gps_string_temp[21];
				gps_temp[7] = gps_string_temp[23];
				gps_temp[8] = gps_string_temp[24];
				gps_temp[9] = '.';
				gps_temp[10] = gps_string_temp[41];
				gps_temp[11] = '-';
				gps_temp[12] = gps_string_temp[31];
				gps_temp[13] = gps_string_temp[32];
				gps_temp[14] = ',';
				gps_temp[15] = gps_string_temp[33];
				gps_temp[16] = gps_string_temp[34];
				gps_temp[17] = gps_string_temp[36];
				gps_temp[18] = gps_string_temp[37];
				gps_temp[19] = '.';
				gps_temp[20] = '\0';

				strcpy(gps_string, gps_temp);
				free(gps_temp);

				test = TRUE;
			}
			free(gps_test);
		}
		free(gps_string_temp);
	}
	return gps_string;
}
/* ------------------------------------------------------------------------- */

void toggleInput() { PORTB ^= 0x01; }
