#include "usart.h"

bool print(char* pMsg){
    static uint8_t pIndex = 0x00U;
		static bool _state = false;
    char currChar = 0x00U;
    char lastChar = 0x00U;
    while(pIndex != 0xFF && !(currChar == 0x0DU && lastChar == 0x0AU) && !(currChar = 0x0AU && lastChar == 0x0DU)){
        //TODO: Add chack TXE buff flag
			usart_data_transmit(USART_PC, pMsg[pIndex++]);
    }
		((pIndex == 0xFFU) ? (_state = false) : (_state = true)); 
		return _state;
}

