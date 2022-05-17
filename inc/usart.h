#ifndef _usart_h_
#define _usart_h_
#include "main.h"

#define USART_PC USART0

void vSendByte(uint8_t Byte);

void vPrint(uint8_t *pMsg);

#endif