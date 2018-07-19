/* ------------------------------------------------------------------------- */
/* File: gsm.c ------------------------------------------------------------- */
/* Author: João Spranger - a2716@alunos.ipca.pt ---------------------------- */
/* Course: Informática - Branch: Industrial -------------------------------- */
/* Class: Projecto - 2009/2010 --------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* This file is a source-code file, and contains: -------------------------- */
/* @ Gsm configuration and interaction methods source-code ----------------- */
/* ------------------------------------------------------------------------- */

// Include Project Libraries
#include "../definitions.h"
#include "../procedures.h"
#include "../usart/usart.h"
#include "gsm.h"

// Program memory global variables
const unsigned char at[] PROGMEM = "AT\r";
const unsigned char atcmgf[] PROGMEM = "AT+CMGF=0\r";
const unsigned char atcnmi[] PROGMEM = "AT+CNMI=1,1,0,0,1\r";

const unsigned char atcmgr[] PROGMEM = "AT+CMGR=";
const unsigned char atcmgs[] PROGMEM = "AT+CMGS=";

const unsigned char smsbeginning[] PROGMEM = "000100";
const unsigned char numbertype[] PROGMEM = "91";
const unsigned char smsmiddle[] PROGMEM = "0000";

const unsigned char crlf[] PROGMEM = "\r";

const unsigned char error[] PROGMEM = "ERROR\r\n";

const unsigned char fixnorth[] PROGMEM = ".outputGPS.N-";
const unsigned char fixsouth[] PROGMEM = ".outputGPS.S-";

PGM_P gsmstaticdata[] PROGMEM =
{	at,				// 0
	atcmgf,			// 1
	atcnmi,			// 2
	atcmgr,			// 3
	atcmgs,			// 4
	smsbeginning,	// 5
	numbertype,		// 6
	smsmiddle,		// 7
	crlf,			// 8
	error,			// 9
	fixnorth,		// 10
	fixsouth		// 11
};

/* ------------------------------- PROTOTYPES ------------------------------ */
void sendMODEM(unsigned char *);
unsigned char *decodePDU(unsigned char *, int);
unsigned char returnHexfromChar(unsigned char *);
unsigned char assignChar(unsigned char, unsigned char, uint8_t);
unsigned char *convert8to7bit(unsigned char *, int);
unsigned char *encodePDU(unsigned char *);
unsigned char returnCharfromHex(unsigned char, int);
unsigned char *convert7to8bit(unsigned char *);
/* ------------------------------------------------------------------------- */

/* --------------------------- PUBLIC FUNCTIONS ---------------------------- */
void initGSM()
// Prepare GSM Modem to send RS232 message when SMS arrives
{
	// Send command to test if GSM modem is fully functional ("AT\r\n")
	unsigned char command1[strlen_P((PGM_P)pgm_read_word(&(gsmstaticdata[0])))];
	strcpy_P(command1, (PGM_P)pgm_read_word(&(gsmstaticdata[0])));
	sendMODEM(command1);

	// Send command to set messages on PDU format ("AT+CMGF=0\r\n")
	unsigned char command2[strlen_P((PGM_P)pgm_read_word(&(gsmstaticdata[1])))];
	strcpy_P(command2, (PGM_P)pgm_read_word(&(gsmstaticdata[1])));
	sendMODEM(command2);

	// Send command to make phone alert on new sms received ("AT+CNMI=1,1,0,0,1\r\n")
	unsigned char command3[strlen_P((PGM_P)pgm_read_word(&(gsmstaticdata[2])))];
	strcpy_P(command3, (PGM_P)pgm_read_word(&(gsmstaticdata[2])));
	sendMODEM(command3);
}

SMS *retrieveSMS(unsigned char *cmti)
// Retrieve the SMS content
{
	// Retrieve sms index number
	unsigned char *index = returnSubStr(14, strlen(cmti) - 3, cmti);

	// Send code (with index) to retrieve sms
	unsigned char *cmgr = calloc((10 + strlen(index)) + 1, sizeof(unsigned char));
	strcpy_P(cmgr, (PGM_P)pgm_read_word(&(gsmstaticdata[3])));
	strcat(cmgr, index);
	strcat_P(cmgr, (PGM_P)pgm_read_word(&(gsmstaticdata[8])));
	sendUSART(cmgr);

	// Read echo (to clear UDR receive buffer of echo) = "AT+CMGR=code\r\r\n"
	// Size = strlen(cmgr) + 1
	free(retrieveUSART(strlen(cmgr) + 2, 0));
	free(cmgr);
	free(index);

	// Retrieve sms message ----------------------------------------------------
	// Receive (header) -> +CMGR: 0,,20\r\n
	free(retrieveUSART(20, 0));

	//IMPORTANT - CONTENT OF SMS MESSAGE
	unsigned char *pdu_received = retrieveUSART(100, 0);

	// Minimum = (\r\n)OK\r\n -> 2 char
	free(retrieveUSART(10, 2));
	// -------------------------------------------------------------------------

	// DECODE SMS --------------------------------------------------------------
	// SMSC INFORMATION HEADER
	// Acquire SMSC number length (in pseudo-octets represented by 2 char)
	unsigned char *smsc_length = returnSubStr(0, 1, pdu_received); // get substring with selection
	int smsc_nbr_length = returnHexfromChar(smsc_length) * 2;// get int value with number of chars
	free(smsc_length); // Discard information
	
	// PDU-String -------------------
	// Get number of chars representing the emmiter phone number
	unsigned char *emmiter_length = returnSubStr(4 + smsc_nbr_length, (6 + smsc_nbr_length) - 1, pdu_received); // get substring with selection
	int emmiter_nbr_length = returnHexfromChar(emmiter_length);// get int value with number of chars
	if (emmiter_nbr_length % 2 != 0) emmiter_nbr_length++; // Case length != pair, char before last is 'F' (just for filling)

	// Get emmiter phone number (coded) into unsigned char *
	unsigned char *emmiter = returnSubStr(8 + smsc_nbr_length, (8 + smsc_nbr_length + emmiter_nbr_length) - 1, pdu_received); // get substring with selection

	// SMS (decoded) length (number of chars)
	unsigned char *sms_length = returnSubStr(26 + smsc_nbr_length + emmiter_nbr_length, 27 + smsc_nbr_length + emmiter_nbr_length, pdu_received);
	int sms_nbr_length = returnHexfromChar(sms_length);
	free(sms_length);

	// Get PDU sms (coded)
	unsigned char *sms_coded = returnSubStr(28 + smsc_nbr_length + emmiter_nbr_length, strlen(pdu_received) - 3, pdu_received);

	// Get PDU sms after decoding
	unsigned char *sms_decoded = decodePDU(sms_coded, sms_nbr_length + 1);

	free(sms_coded);
	free(pdu_received);

	SMS *sms = calloc(1, sizeof(SMS));

	sms -> address_length = calloc(strlen(emmiter_length) + 1, sizeof(unsigned char));
	strcpy(sms -> address_length, emmiter_length);
	free(emmiter_length);

	sms -> address = calloc(strlen(emmiter) + 1, sizeof(unsigned char));
	strcpy(sms -> address, emmiter);
	free(emmiter);

	sms -> content = calloc(strlen(sms_decoded) + 1, sizeof(unsigned char));
	strcpy(sms -> content, sms_decoded);
	free(sms_decoded);

	return sms;
}

void sendSMS(unsigned char *address_length, unsigned char *address, unsigned char *content)
// Send SMS with response
{
	unsigned char *content_size = calloc(3, sizeof(unsigned char));
	sprintf(content_size, "%02X", strlen(content));

	unsigned char *encoded_content = encodePDU(content);

	unsigned char *temp = returnSubStr(0, 12, content);
	if ((strcmp_P(temp, (PGM_P)pgm_read_word(&(gsmstaticdata[10]))) == 0) || (strcmp_P(temp, (PGM_P)pgm_read_word(&(gsmstaticdata[11]))) == 0))
	strcat(encoded_content, "00"); // encodePDU() fix
	free(temp);
// -----------------------------------------------------------------------------------------
	unsigned char *startofsms = calloc(strlen_P((PGM_P)pgm_read_word(&(gsmstaticdata[4]))) + 1, sizeof(unsigned char));
	strcpy_P(startofsms, (PGM_P)pgm_read_word(&(gsmstaticdata[4])));
	sendUSART(startofsms);
	free(startofsms);

	int octets_int =
	strlen_P((PGM_P)pgm_read_word(&(gsmstaticdata[5])))	+
	strlen(address_length) +
	strlen_P((PGM_P)pgm_read_word(&(gsmstaticdata[6]))) +
	strlen(address) +
	strlen_P((PGM_P)pgm_read_word(&(gsmstaticdata[7]))) +
	strlen(content_size) +
	strlen(encoded_content);

	unsigned char *octets = calloc(3, sizeof(unsigned char));
	sprintf(octets, "%d", (octets_int/2) - 1);
	sendUSART(octets);
	free(octets);
	
	unsigned char *endofstart = calloc(strlen_P((PGM_P)pgm_read_word(&(gsmstaticdata[8]))) + 1, sizeof(unsigned char));
	strcpy_P(endofstart, (PGM_P)pgm_read_word(&(gsmstaticdata[8])));
	sendUSART(endofstart);
	free(endofstart);

	// Free response
	free(retrieveUSART(20, 0));

    unsigned char *begin = calloc(strlen_P((PGM_P)pgm_read_word(&(gsmstaticdata[5]))) + 1, sizeof(unsigned char));
	strcpy_P(begin, (PGM_P)pgm_read_word(&(gsmstaticdata[5])));
	sendUSART(begin);
	free(begin);

	sendUSART(address_length);

    unsigned char *tp = calloc(strlen_P((PGM_P)pgm_read_word(&(gsmstaticdata[6]))) + 1, sizeof(unsigned char));
	strcpy_P(tp, (PGM_P)pgm_read_word(&(gsmstaticdata[6])));
	sendUSART(tp);
	free(tp);

	sendUSART(address);

    unsigned char *mid = calloc(strlen_P((PGM_P)pgm_read_word(&(gsmstaticdata[7]))) + 1, sizeof(unsigned char));
	strcpy_P(mid, (PGM_P)pgm_read_word(&(gsmstaticdata[7])));
	sendUSART(mid);
	free(mid);

	sendUSART(content_size);
	free(content_size);

	sendUSART(encoded_content);
	free(encoded_content);

	// Enviar char 0x1A	para enviar a sms.
	sendChar(0x1A);
	
	// RETRIEVE USART RX ECHO FROM COMMAND SENT + RESPONSE
	free(retrieveUSART(100, 0));

	unsigned char *status = retrieveUSART(20,0); // +CMGS: (INDEX)\r\n or ERROR\r\n
	if (strcmp_P(status,(PGM_P)pgm_read_word(&(gsmstaticdata[9]))) == 0) ;
	else free(retrieveUSART(10, 2)); // \r\nOK\r\n
	free(status);
}
/* ------------------------------------------------------------------------- */

/* --------------------------- PRIVATE FUNCTIONS --------------------------- */
void sendMODEM(unsigned char *cmd)
{
	uint8_t test = FALSE;
	while (test == FALSE)
	{
		sendUSART(cmd);
		// Maximum Response Size = strlen(cmd) + 8 [\rERROR\r\n]
		unsigned char *response = retrieveUSART(30, (strlen(cmd) + 2)); // First response is echo

		uint8_t i;
		for (i = (strlen(cmd)); i < (strlen(response) - 2); i++)
		{
			if ((response[i] == 'O') & (response[i + 1] == 'K')) test = TRUE;
			// else repeat
		}
		free(response);
	}
}

// Método privado para concatenar hexadecimal 
unsigned char assignChar(unsigned char value, unsigned char hex, unsigned char test)
{
	if (test == FALSE) value = hex;
	else
	{
		value = value << 4;
		value = value ^ hex;
	}
	return value;
}

// Método para converter para hex partindo de array de char com hex
unsigned char returnHexfromChar(unsigned char *input)
{    
	uint8_t test = FALSE;
	unsigned char c = 0;

	uint8_t i;
	for (i = 0; i < strlen(input); i++)
	{
		switch(input[i])
		{
			case '0':
				c = assignChar(c, 0x00, test);
				test = TRUE;
				break;
			case '1':
				c = assignChar(c, 0x01, test);
				test = TRUE;
				break;
			case '2':
				c = assignChar(c, 0x02, test);
				test = TRUE;
				break;
			case '3':
				c = assignChar(c, 0x03, test);
				test = TRUE;
				break;
			case '4':
				c = assignChar(c, 0x04, test);
				test = TRUE;
				break;
			case '5':
				c = assignChar(c, 0x05, test);
				test = TRUE;
				break;
			case '6':
				c = assignChar(c, 0x06, test);
				test = TRUE;
				break;
			case '7':
				c = assignChar(c, 0x07, test);
				test = TRUE;
				break;
			case '8':
				c = assignChar(c, 0x08, test);
				test = TRUE;
				break;
			case '9':
				c = assignChar(c, 0x09, test);
				test = TRUE;
				break;
			case 'A':
				c = assignChar(c, 0x0A, test);
				test = TRUE;
				break;
			case 'B':
				c = assignChar(c, 0x0B, test);
				test = TRUE;
				break;
			case 'C':
				c = assignChar(c, 0x0C, test);
				test = TRUE;
				break;
			case 'D':
				c = assignChar(c, 0x0D, test);
				test = TRUE;
				break;
			case 'E':
				c = assignChar(c, 0x0E, test);
				test = TRUE;
				break;
			case 'F':
				c = assignChar(c, 0x0F, test);
				test = TRUE;
				break;
		}
	}
	return c;
}

unsigned char *convert8to7bit(unsigned char *input, int strsize)
{
	unsigned char *converted = calloc(strsize, sizeof(unsigned char));

	unsigned char hibits = 0;
	unsigned char lobits = 0;
	unsigned char bits = 7;
	unsigned char rest = 0;

	uint8_t counter = 0;
	uint8_t i;
	for (i = 0; i < strlen(input); i++)
	{   
		if (bits == 0)
		{
			converted[counter] = hibits;
			counter++;
			bits = 7;
			rest = 0;
			hibits = 0;
		}
		lobits = input[i] << (byte - bits);
		lobits = lobits >> (byte - bits - rest);
		converted[counter] = lobits | hibits;
		counter++;
		hibits = input[i] >> bits;
		bits--;
		rest++;
	}
	converted[counter] = '\0';

	return converted;
}

// Método para descodificar a mensagem SMS
unsigned char *decodePDU(unsigned char *input, int strsize)
{
	unsigned char *smsbytes = calloc(((strlen(input)/2) + 1), sizeof(unsigned char));

	uint8_t counter = 0;
	uint8_t i;
	for (i = 0; i < strlen(input); i++)
	{
		unsigned char *temp = calloc(3, sizeof(unsigned char));
		temp[0] = input[i];
		temp[1] = input[i + 1];
		temp[2] = '\0';
		smsbytes[counter] = returnHexfromChar(temp);
		free(temp);

		counter++;
		i++;
	}
	smsbytes[counter] = '\0';

	unsigned char *decoded = convert8to7bit(smsbytes, strsize);
	free(smsbytes);

	return decoded;
}

// Método para converter para array de char com hex partindo de int
unsigned char returnCharfromHex(unsigned char input, int index)
{
	unsigned char *temp = calloc(3, sizeof(unsigned char));
	sprintf(temp, "%02X", input);
	
    unsigned char c = temp[index];
	free(temp);
	
	return c;
}

unsigned char *convert7to8bit(unsigned char *input)
{
	unsigned char *converted = calloc(60, sizeof(unsigned char));

	unsigned char hibits = 0;
	unsigned char lobits = 0;
	unsigned char bits = 0;

	uint8_t counter = 0;
	uint8_t i;
	for (i = 0; i < strlen(input); i++)
	{
		if (bits == 7) bits = 0;
		else
		{
            lobits = input[i] >> bits;
			hibits = input[i + 1] << (byte - bits - 1);
			converted[counter] = lobits | hibits;
			counter++;
			bits++;
		}
	}
	converted[counter] = '\0';
	return converted;
}

// Método para codificar a mensagem SMS
unsigned char *encodePDU(unsigned char *input)
{
	unsigned char *converted = convert7to8bit(input);
	// Add 2 chars because of encodePDU() fix (above)
	unsigned char *result = calloc(((strlen(converted) * 2 ) + 1) + 2, sizeof(unsigned char));

	uint8_t counter = 0;
	uint8_t i;
	for (i = 0; i < strlen(converted); i++)
    {
        result[counter] = returnCharfromHex(converted[i], 0); counter++;
        result[counter] = returnCharfromHex(converted[i], 1); counter++;
    }
    result[counter] = '\0';
	free(converted);

	return result;
}
/* ------------------------------------------------------------------------- */
