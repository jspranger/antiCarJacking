/* ------------------------------------------------------------------------- */
/* File: anticarjacking.c -------------------------------------------------- */
/* Author: João Spranger - a2716@alunos.ipca.pt ---------------------------- */
/* Course: Informática - Branch: Industrial -------------------------------- */
/* Class: Projecto - 2009/2010 --------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* This file is a source-code file, and contains: -------------------------- */
/* @ Main program source-code ---------------------------------------------- */
/* @ Usarx Rx interruption handler source-code ----------------------------- */
/* ------------------------------------------------------------------------- */

/* Project Defaults -------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* MicroController - AVR Atmega88 | 8 bit --------------------------------- */
/* Oscillator Frequency - 1 MHz | Baud - 4800 ------------------------------ */
/* ------------------------------------------------------------------------- */
/* PortC,bit0 - Green Led | System Ready - 0x01 ---------------------------- */
/* PortC,bit1 - Red Led | System Busy - 0x02 ------------------------------- */
/* ------------------------------------------------------------------------- */
/* PortB,bit0 - Multiplexer: 0 - Gsm Modem | 1 - Gps Antenna --------------- */
/* PortB,bit1 - Relay: 0 - Turned off | 1 - Turned on (IGNITION IS INVERTED) */
/* ------------------------------------------------------------------------- */
/* Carriage Return + Line Feed - "\r\n" ------------------------------------ */
/* End of String - '\0' ---------------------------------------------------- */
/* ------------------------------------------------------------------------- */

// Include Project Libraries
#include "definitions.h"
#include "procedures.h"

// Volatile global variable interchageable between Main Program & ISR
// (1 if data waiting to be read at USART - Rx, 0 if not)
volatile uint8_t rxflag = 0;

/* --------------------------------- MAIN ---------------------------------- */
int main(void)
{
	// Perform startup procedures
	startup();

	__enable_interrupt(); // Enable Global Interrupts

	// Loop forever
	for (;;)
	{
		// If data received through USART - Rx
		if (rxflag == 1)
		{
			// Disable interrupts
			__disable_interrupt();

			// Manage procedures
			processRoutine();

			// Update rxflag variable
			rxflag = 0;

			// Enable RXCIE interruption
			UCSR0B ^= 0x80;

			// Enable interrupts
			__enable_interrupt();
		}
		else
		{
			// Go to sleep
			sleep_mode();

			// From now on, program will be oriented to interrupt @ RXC Flag bit
		}
	}
	return 0;
}
/* ------------------------------------------------------------------------- */

/* --------------------------- INTERRUPT VECTOR ---------------------------- */
ISR(USART_RX_vect)
{
	// Disable Global Interrupts
	__disable_interrupt();

	// Update rxflag variable
	rxflag = 1;

	// Disable RXCIE interruption
	UCSR0B ^= 0x80;

	// Enable Global Interrupts
	__enable_interrupt();
}
/* ------------------------------------------------------------------------- */
