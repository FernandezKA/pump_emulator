#include "usart.h"

void vSendByte(uint8_t Byte){
	//wait, while transmit buffer isn't empty
	while((USART_STAT(USART_PC) & USART_STAT_TBE) != USART_STAT_TBE){
		__NOP();
	}
	USART_DATA(USART_PC) = Byte;
}

void print(char *pMsg){
	static uint8_t u8MsgSize = 0x00U;
	static uint8_t u8MsgBuff[0x40U]; //Max size info msg is 64 symbol
	static char last_symbol, curr_symbol;
	while(u8MsgSize < 0x40U || (last_symbol == '\n' && curr_symbol == '\r') || (last_symbol == '\r' && curr_symbol == '\n')){
		last_symbol = curr_symbol;
		curr_symbol = pMsg[u8MsgSize++];
		xQueueSendToBack(uart_info, &curr_symbol, 0);
	}
}