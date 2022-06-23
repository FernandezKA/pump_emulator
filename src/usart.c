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
	volatile uint8_t _index = 0;
	volatile char last_symbol = 0x00, curr_symbol = 0x00;
	while ((_index < 0x80U) && !(last_symbol == 0x0A && curr_symbol == 0x0D) && !(last_symbol == 0x0D && curr_symbol == 0x0A))
	{
		last_symbol = curr_symbol;
		curr_symbol = pMsg[_index++];
		vSendByte(curr_symbol);
	}
}

void print_0(char *pMsg){
	uint8_t index = 0; 
	while(pMsg[index] != '\0'){
		vSendByte(pMsg[index++]);
	}
}

void uart_info_task(void *pvParameters)
{
	static uint8_t u8Rec_Buff;
	for (;;)
	{
		get_temp_int_conversion(&therm_int);
		print_0("\n\r*************************************\n\r");
		taskYIELD();
		print_0("\n\rMeasured voltage from PA2: ");
		print_float(measured_ad8400);		
		print_0("\n\rTemperature from the thermistor: ");
		//taskYIELD();
		print_temp(therm_int.temp);
		taskYIELD();
		// print_digit(global_adc_1);
		print_0("\n\rDevice state: ");
		//taskYIELD();
		if (bus_error)
		{
			print_0("Bus error detected");
		}
		taskYIELD();
		if (pwm_detect)
		{
			print_0("PWM signal detected, with filling: ");
			print_digit(get_pwm_fill(&PWM));
		}
		taskYIELD();
		if (start_req)
		{
			print_0("Respond sequence generation enable");
		}
		taskYIELD();
		//vSendByte('\n');
		taskYIELD();
		print_0("\n\rValue for AD8400 after measure: ");
		print_digit(conversion_result);
		vTaskDelay(pdMS_TO_TICKS(1000U)); // Check info buffer every second
	}
}

void print_digit(char _digit)
{
	char first_digit, second_digit, third_digit;
	first_digit = (_digit / 100) + '0';
	second_digit = (_digit / 10) % 10 + '0';
	third_digit = (_digit % 10) + '0';
	vSendByte(first_digit);
	vSendByte(second_digit);
	vSendByte(third_digit);
}

void print_float(float val)
{
	char first_digit, second_digit, third_digit;
	uint8_t _digit = (uint8_t)(val * (float)100);
	first_digit = (_digit / 100) + '0';
	second_digit = (_digit / 10) % 10 + '0';
	third_digit = (_digit % 10) + '0';
	vSendByte(first_digit);
	vSendByte('.');
	vSendByte(second_digit);
	vSendByte(third_digit);
}

void print_temp(uint8_t _tmp){
	char first_digit, second_digit, third_digit;
	signed int tmp_sgn = 0;
	tmp_sgn = _tmp - 20;
	if(tmp_sgn < 0){
			first_digit = '-';
			//first_digit = (abs(tmp_sgn) / 100) + '0';
			second_digit = (abs(tmp_sgn) / 10) % 10 + '0';
			third_digit = (abs(tmp_sgn) % 10) + '0';
	}
	else{
			first_digit = (tmp_sgn / 100) + '0';
			second_digit = (tmp_sgn / 10) % 10 + '0';
			third_digit = (tmp_sgn % 10) + '0';
	}
	vSendByte(first_digit);
	vSendByte(second_digit);
	vSendByte(third_digit);
}