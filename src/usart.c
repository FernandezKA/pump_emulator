#include "user_task.h"
#include "usart.h"

void vSendByte(char Byte)
{
	// wait, while transmit buffer isn't empty
	while ((USART_STAT(USART_PC) & USART_STAT_TBE) != USART_STAT_TBE)
	{
		__NOP();
	}
	USART_DATA(USART_PC) = Byte;
}

void print(char *pMsg)
{
	static uint8_t index = 0x00U;
	while(pMsg[index] != '\0'){
		vSendByte(pMsg[index++]);
	}
}

void uart_info_task(void *pvParameters)
{
	static uint8_t u8Rec_Buff;
	for (;;)
	{
		//taskENTER_CRITICAL();
		print("Temp: ");
		print_digit(therm_int.temp);
		//print_digit(global_adc_1);
		if(bus_error){
			print("Bus error detected\n\r");
		}
		if(pwm_detect){
			print("PWM signal detected\n\r");		
		}
		if(start_req){
			print("Start request detected\n\r");
		}
		taskENTER_CRITICAL();
		get_temp_int_conversion(&therm_int);
		taskEXIT_CRITICAL();
		
		
		//taskEXIT_CRITICAL();
		vTaskDelay(pdMS_TO_TICKS(1000U)); // Check info buffer every second
	}
}

void print_digit(char _digit){
	char first_digit, second_digit, third_digit;
	first_digit = (_digit/100) + '0';
	second_digit = (_digit/10)%10 + '0';
	third_digit = (_digit%10) + '0';
	vSendByte(first_digit);
	vSendByte(second_digit);
	vSendByte(third_digit);
	vSendByte('\n');
}

void print_float(float val){
	char first_digit, second_digit, third_digit;
	uint8_t _digit = (uint8_t) (val * (float)100);
	first_digit = (_digit/100) + '0';
	second_digit = (_digit/10)%10 + '0';
	third_digit = (_digit%10) + '0';
	vSendByte(first_digit);
	vSendByte('.');
	vSendByte(second_digit);
	vSendByte(third_digit);
	vSendByte('\n');
}