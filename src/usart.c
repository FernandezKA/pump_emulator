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
	xQueueReset(uart_info);
	while ((_index < 0x80U) && !(last_symbol == 0x0A && curr_symbol == 0x0D) && !(last_symbol == 0x0D && curr_symbol == 0x0A))
	{
		last_symbol = curr_symbol;
		curr_symbol = pMsg[_index++];
		vSendByte(curr_symbol);
	}
}

void uart_info_task(void *pvParameters)
{
	static uint8_t u8Rec_Buff;
	for (;;)
	{
		get_temp_int_conversion(&therm_int);

		print("*************************************\n\r");
		taskYIELD();
		print("Temperature from the thermistor: \n\r");
		taskYIELD();
		print_digit(therm_int.temp);
		taskYIELD();
		// print_digit(global_adc_1);
		print("Device state: \n\r");
		taskYIELD();
		if (bus_error)
		{
			print("Bus error detected\n\r");
		}
		taskYIELD();
		if (pwm_detect)
		{
			print("PWM signal detected, with filling: \n\r");
			print_digit(get_pwm_fill(&PWM));
		}
		taskYIELD();
		if (start_req)
		{
			print("Respond sequence generation enable\n\r");
		}
		taskYIELD();
		vSendByte('\n');
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
	vSendByte('\n');
	vSendByte('\r');
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
	vSendByte('\n');
	vSendByte('\r');
}