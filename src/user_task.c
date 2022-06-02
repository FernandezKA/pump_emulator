/*
 * Project: pump emulator
 * File: user_task.c
 * Author: FernandezKA
 */

// User defines
#include "user_task.h"

TaskHandle_t ad8400_0_task_handle = NULL;
TaskHandle_t ad8400_1_task_handle = NULL;
TaskHandle_t main_task_handle = NULL;
TaskHandle_t sample_task_handle = NULL;
TaskHandle_t response_task_handle = NULL;
TaskHandle_t adc_task_handle = NULL;
TaskHandle_t pwm_def_task_handle = NULL;
TaskHandle_t uart_info_task_handle = NULL;
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

// This task used for definition pwm filling pn pwm_2
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
