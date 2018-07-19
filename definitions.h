/* ------------------------------------------------------------------------- */
/* File: definitions.h ----------------------------------------------------- */
/* Author: João Spranger - a2716@alunos.ipca.pt ---------------------------- */
/* Course: Informática - Branch: Industrial -------------------------------- */
/* Class: Projecto - 2009/2010 --------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* This file is a header file, and contains: ------------------------------- */
/* @ Inclusions of AVR Libc libraries relevant to the project -------------- */
/* @ Aliases definitions used along the project ---------------------------- */
/* ------------------------------------------------------------------------- */

#define F_CPU 1000000UL  // 8 MHz

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <avr/sleep.h>

#define __disable_interrupt cli
#define __enable_interrupt sei

#define FALSE   0
#define TRUE    1
