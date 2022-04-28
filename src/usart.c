#include "usart.h"

bool print(char* pMsg){
    static uint8_t pIndex = 0x00U;
		static bool _state = false;
    char currChar = 0x00U;
    char lastChar = 0x00U;
    while(pIndex != 0xFF && !(currChar == 0x0DU && lastChar == 0x0AU) && !(currChar = 0x0AU && lastChar == 0x0DU)){
        //TODO: Add chack TXE buff flag
			while((USART_STAT(USART_PC) & USART_STAT_TBE) != USART_STAT_TBE) {__NOP();} //Wait, while TX buffer isn't empty, then send next byte
			USART_DATA(USART_PC) = pMsg[pIndex++];
			//usart_data_transmit(USART_PC, pMsg[pIndex++]); //Send current, then increment array pointer
    }
		((pIndex == 0xFFU) ? (_state = false) : (_state = true)); //If we can't detect \n\r sequence, data transmission isn't successfull
		//Chech at iteration overflow counter
		return _state;
}

