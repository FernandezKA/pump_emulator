/*
 * Project: pump emulator
 * File: user_task.c
 * Author: FernandezKA
 */

// User defines
#define PWM_TIME_EDGE 20U
#include "user_task.h"

TaskHandle_t ad8400_0_task_handle = NULL;
TaskHandle_t ad8400_1_task_handle = NULL;
TaskHandle_t main_task_handle = NULL;
TaskHandle_t sample_task_handle = NULL;
TaskHandle_t response_task_handle = NULL;
TaskHandle_t adc_task_handle = NULL;
TaskHandle_t pwm_def_task_handle = NULL;
TaskHandle_t uart_info_task_handle = NULL;

// It's a main task. Detect input sequence, then get action with defined rules.

/*************************************************************************************************
 * ***********************************************************************************************
 * ***********************************************************************************************/

/*************************************************************************************************
 * ***********************************************************************************************
 * ***********************************************************************************************/
// This task used for genrating responce signal at start request
void response_task(void *pvParameters)
{
	const struct pulse response[] = {{.state = true, .time = 100}, {.state = false, .time = 20}, {.state = true, .time = 180}, {.state = false, .time = 100}, {.state = true, .time = 100}, {.state = false, .time = 10}, {.state = true, .time = 190}, {.state = false, .time = 100}};
	uint8_t response_index = 0x00U;
	for (;;)
	{
		if (!response[response_index].state)
		{
			GPIO_OCTL(RESPONSE_PORT) |= RESPONSE_PIN;
		}
		else
		{
			GPIO_OCTL(RESPONSE_PORT) &= ~RESPONSE_PIN;
		}
		if (response_index < 0x08U)
		{
			vTaskDelay(pdMS_TO_TICKS(response[response_index++].time));
		}
		else
		{
			response_index = 0x00U;
		}
	}
}

/*************************************************************************************************
 * ***********************************************************************************************
 * ***********************************************************************************************/
// This task used for answering value from ADC
void adc_task(void *pvParameters)
{
	static uint16_t adc_value_0 = 0x00U;
	static uint16_t adc_value_1 = 0x00U;
	for (;;)
	{
		if (!adc_flag_get(ADC0, ADC_FLAG_STRC))
		{
			if (adc_flag_get(ADC0, ADC_FLAG_EOC))
			{
				adc_flag_clear(ADC0, ADC_FLAG_EOC);
				if (0x02 == adc_get_channel())
				{
					adc_value_0 = adc_regular_data_read(ADC0);
					adc_select_channel(0x03);
				}
				else
				{
					adc_value_1 = adc_regular_data_read(ADC0);
					adc_select_channel(0x02);
				}
				adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);
			}
		}
		else
		{
			if (adc_flag_get(ADC0, ADC_FLAG_EOC))
			{
				adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);
			}
		}
		vTaskDelay(pdMS_TO_TICKS(1));
		if (adc_flag_get(ADC0, ADC_FLAG_EOC))
		{
			adc_flag_clear(ADC0, ADC_FLAG_EOC);

			adc_value_0 = adc_regular_data_read(ADC0);
			adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);
		}
		else
		{
			if (adc_flag_get(ADC0, ADC_FLAG_STRC))
			{
				adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);
			}
			else
			{
				adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);
			}
		}
	}
}
/*************************************************************************************************
 * ***********************************************************************************************
 * ***********************************************************************************************/
// This task used for more different pwm fillings forming (begin at detect pwm_fill on pwm_in)
void pwm_def_task(void *pvParameters)
{
	for (;;)
	{
		set_pwm(pwm_2, 50U);
		vTaskDelay(pdMS_TO_TICKS(10000));
		set_pwm(pwm_2, 45U);
		vTaskDelay(pdMS_TO_TICKS(10000));
		set_pwm(pwm_2, 55U);
		vTaskDelay(pdMS_TO_TICKS(10000));
		set_pwm(pwm_2, 45U);
		vTaskDelay(pdMS_TO_TICKS(10000));
	}
}
/*************************************************************************************************
 * ***********************************************************************************************
 * ***********************************************************************************************/
// This task used for send uart info messages
void uart_info_task(void *pvParameters)
{
	static uint8_t u8Rec_Buff;
	for (;;)
	{
		// Check fifo buff state
		while (pdPASS == xQueueReceive(uart_info, &u8Rec_Buff, 0))
		{
			vSendByte(u8Rec_Buff);
		}
		xQueueReset(uart_info);
		vTaskDelay(pdMS_TO_TICKS(1U)); // Check info buffer every second
	}
}

/***********************************************
//End of tasks functions
***********************************************/