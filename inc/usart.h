#ifndef _usart_h_
#define _usart_h_
#include "main.h"

#define USART_PC USART0

void vSendByte(char Byte);

void print(char *pMsg);

void print_digit(char _digit);

void print_float(float val);


#endif