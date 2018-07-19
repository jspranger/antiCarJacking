/* ------------------------------------------------------------------------- */
/* File: gsm.h ------------------------------------------------------------- */
/* Author: João Spranger - a2716@alunos.ipca.pt ---------------------------- */
/* Course: Informática - Branch: Industrial -------------------------------- */
/* Class: Projecto - 2009/2010 --------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* This file is a header file, and contains: ------------------------------- */
/* @ Definition of struct SMS used in file gsm.c and file procedures.c ----- */
/* @ Declaration of global methods defined at file gsm.c ------------------- */
/* ------------------------------------------------------------------------- */

typedef struct
{
	unsigned char *address_length;
	unsigned char *address;
	unsigned char *content;
} SMS;

void initGSM(void);

SMS *retrieveSMS(unsigned char *);

void sendSMS(unsigned char *, unsigned char *, unsigned char *);

#define byte 8
