#ifndef _usart_h_
#define _usart_h_
#include "main.h"

#define USART_PC USART0

void vSendByte(uint8_t Byte);

void print(char *pMsg);

#endif