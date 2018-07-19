/* ------------------------------------------------------------------------- */
/* File: usart.c ----------------------------------------------------------- */
/* Author: João Spranger - a2716@alunos.ipca.pt ---------------------------- */
/* Course: Informática - Branch: Industrial -------------------------------- */
/* Class: Projecto - 2009/2010 --------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* This file is a source-code file, and contains: -------------------------- */
/* @ Usart configuration and interaction methods source-code --------------- */
/* ------------------------------------------------------------------------- */

// Include Project Libraries
#include "../definitions.h"
#include "usart.h"

/* ------------------------------- PROTOTYPES ------------------------------ */
unsigned char getChar(void);
/* ------------------------------------------------------------------------- */

/* ---------------------------- PUBLIC METHODS ----------------------------- */
// Method to set Baud Rate
void initUSART(int baudrate)
{
	// Set baud rate
	UBRR0H = (uint8_t)(baudrate>>8);
	UBRR0L = (uint8_t)baudrate;

	// Enable receiver & transmitter
	UCSR0B |= (1<<RXEN0)|(1<<TXEN0);
}

// Method to send string (char *) through transmitter
void sendUSART(unsigned char *input)
{
	int i;
	for (i = 0; i < strlen(input); i++) sendChar(input[i]);
}


// Method to retrieve string (char *) from receiver
unsigned char *retrieveUSART(int size, int minimum)
{
	unsigned char *string = calloc(size, sizeof(unsigned char));

	unsigned char test = FALSE;
	int i = 0;
	while (test == FALSE)
	{
		string[i] = getChar();
		if ((string[i] == '\n') & ((i + 1) > minimum))
		{
			test = TRUE;
			string[i + 1] = '\0';
		}
		i++;
	}
	return string;
}

unsigned char *retrieveUSARTGPS()
{
	unsigned char *string = calloc(150, sizeof(unsigned char));

	unsigned char test = FALSE;
	int i = 0;

	while (test == FALSE)
	{
		string[i] = getChar();
		if (string[i] == '\n') test = TRUE;
		i++;
	}
	string[i] = '\0';
	return string;
}

// Method to send char through USART
void sendChar(unsigned char c)
{
    while (!(UCSR0A & (1<<UDRE0))); // While UDR not empty, do nothing
	UDR0 = c; // Send char
}

/* ------------------------------------------------------------------------- */

/* ---------------------------- PRIVATE METHODS ---------------------------- */
// Method to retrieve char through USART
unsigned char getChar()
{
	while (!(UCSR0A & (1<<RXC0))); // While RXC == 0, do nothing
    return UDR0; // Get char
}
