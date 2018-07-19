/* ------------------------------------------------------------------------- */
/* File: usart.h ----------------------------------------------------------- */
/* Author: João Spranger - a2716@alunos.ipca.pt ---------------------------- */
/* Course: Informática - Branch: Industrial -------------------------------- */
/* Class: Projecto - 2009/2010 --------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* This file is a header file, and contains: ------------------------------- */
/* @ Declaration of global methods defined at file usart.c ----------------- */
/* ------------------------------------------------------------------------- */

void initUSART(int);

void sendUSART(unsigned char *);

unsigned char *retrieveUSART(int, int);

unsigned char *retrieveUSARTGPS(void); // Only meant to be used by GPS (to avoid buffer overflow)

void sendChar(unsigned char); // Only public because of ctrl-z
