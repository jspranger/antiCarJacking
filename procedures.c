/* ------------------------------------------------------------------------- */
/* File: procedures.c ------------------------------------------------------ */
/* Author: João Spranger - a2716@alunos.ipca.pt ---------------------------- */
/* Course: Informática - Branch: Industrial -------------------------------- */
/* Class: Projecto - 2009/2010 --------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* This file is a source-code file, and contains: -------------------------- */
/* @ Startup and routine procedures source-code ---------------------------- */
/* ------------------------------------------------------------------------- */

// Include Project Libraries
#include "definitions.h"
#include "procedures.h"
#include "usart/usart.h"
#include "gsm/gsm.h"
#include "gps/gps.h"

// Program memory variables
const unsigned char cmtistr[] PROGMEM = "\r\n+CMTI:";
const unsigned char beginofsms[] PROGMEM = ".outputGPS.";
const unsigned char location[] PROGMEM = "2716carLocation";
const unsigned char locationfailure[] PROGMEM =  "Unavailable";
const unsigned char shutdown[] PROGMEM = "2716carShutdown";
const unsigned char shutdownsuccess[] PROGMEM = "Shutdown Success";
const unsigned char shutdownfailure[] PROGMEM = "Shutdown Failure";
const unsigned char wrong_code[] PROGMEM = "Wrong Code";

PGM_P proceduresstaticdata[] PROGMEM =
{
	cmtistr,			// 0
	beginofsms,			// 1
	location,			// 2
	locationfailure,	// 3
	shutdown,			// 4
	shutdownsuccess,	// 5
	shutdownfailure,	// 6
	wrong_code			// 7
};

/* ------------------------------- PROTOTYPES ------------------------------ */
void toggleReady(void);
void toggleBusy(void);
/* ------------------------------------------------------------------------- */

/* ---------------------------- PUBLIC METHODS ----------------------------- */
void startup()
{
	// Disable Watchdog
	wdt_disable();

	// Set sleep mode (idle)
	set_sleep_mode(SLEEP_MODE_IDLE);

	// Setup PORTA
	DDRC = 0x03;	// Set first 2 pins of PORTA to output
	PORTC = 0x00;	// Set all pins of PORTA as low
	// Setup PORTB
	DDRB = 0x03;	// Set first 2 pins of PORTB to output
	PORTB = 0x00;	// Set all pins of PORTB as low

	// Test Leds for 1 second
	toggleReady(); toggleBusy();
	_delay_ms(1000);

	toggleReady(); toggleBusy();
	_delay_ms(1000);

	// Turn on System Busy Led
	toggleBusy();
	_delay_ms(100);

	// Initialize USART
	uint16_t baudrate = 12; // UBRR value for Baud = 4800 @ 1 MHz
	initUSART(baudrate);
	// Initialize GSM
	initGSM();

	// Enable RXCIE Interrupt
	UCSR0B ^= 0x80;

	// Turn on System Ready Led && off System Busy Led
	toggleReady(); toggleBusy();
}

void processRoutine()
{
	// Turn on System Busy Led
	toggleBusy();

	// Common message = \r\n+CMTI: "SM",(%d+)\r\n (minimum size = 17)
	unsigned char *cmti = retrieveUSART(25, 2);
	unsigned char *teststring = returnSubStr(0, 7, cmti);

	// Compare start of received string to "\r\n+CMTI:"
	if (strcmp_P(teststring, (PGM_P)pgm_read_word(&(proceduresstaticdata[0]))) == 0)
	{
		SMS *receive = retrieveSMS(cmti);

		// Maximum size = 11 + 21 = 32 -> ".outputGPS." + gps_coords
		unsigned char *send = calloc(40, sizeof(unsigned char));
		strcpy_P(send, (PGM_P)pgm_read_word(&(proceduresstaticdata[1])));

		unsigned char *gps_coords = calloc(21, sizeof(unsigned char));

		uint8_t code = 0;
		// If received content == 2716carLocation
		if (strcmp_P(receive -> content, (PGM_P)pgm_read_word(&(proceduresstaticdata[2]))) == 0) code = 1;
		// If received content == 2716carShutdown
		if (strcmp_P(receive -> content, (PGM_P)pgm_read_word(&(proceduresstaticdata[4]))) == 0) code = 2;

		switch (code)
		{
			case 0:
				strcat_P(send, (PGM_P)pgm_read_word(&(proceduresstaticdata[7])));
				break;
			case 1:
				; // <- C fix
				unsigned char *gps_coords_temp = retrieveGPS();
				strcpy(gps_coords, gps_coords_temp);
				free(gps_coords_temp);

				if ((gps_coords[0] == 'N') || (gps_coords[0] == 'S')) strcat(send, gps_coords);
				else strcat_P(send, (PGM_P)pgm_read_word(&(proceduresstaticdata[3])));
				break;
			case 2:
				PORTB = 0x02;
				if (PORTB == 0x02) strcat_P(send, (PGM_P)pgm_read_word(&(proceduresstaticdata[5])));
				else strcat_P(send, (PGM_P)pgm_read_word(&(proceduresstaticdata[6])));
				break;
		}
		// SEND SMS
		sendSMS(receive -> address_length, receive -> address, send);

		free(send);
		free(gps_coords);
		free(receive -> content);
		free(receive -> address_length);
		free(receive -> address);
		free(receive);

	}
	free(teststring);
	free(cmti);
	// Turn on System Ready Led
	toggleBusy();
}

// Método para retornar array de char composto pelos índices seleccionados
unsigned char *returnSubStr(int istart, int istop, unsigned char *input)
{
	unsigned char *substr = calloc(((istop - istart) + 1) + 1, sizeof(unsigned char));

	uint8_t counter = 0;
	uint8_t i;
	for (i = istart; i <= istop; i++)
	{
		substr[counter] = input[i];
		counter++;
	}
	substr[counter] = '\0';
	return substr;
}
/* ------------------------------------------------------------------------- */

/* ---------------------------- PRIVATE METHODS ---------------------------- */
void toggleReady(void) { PORTC ^= 0x01; }

void toggleBusy(void) { PORTC ^= 0x02; }
/* ------------------------------------------------------------------------- */
