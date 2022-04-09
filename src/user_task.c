#include "user_task.h"

TaskHandle_t ad8400_task_handle = NULL;
TaskHandle_t main_task_handle = NULL;
TaskHandle_t sample_task_handle = NULL;
TaskHandle_t response_task_handle = NULL;

void ad8400_task(void *pvParameters)
{
	for (;;)
	{
	}
}

void main_task(void *pvParameters)
{
	struct pulse timeArray[0x0AU];
	uint8_t indexArray = 0x00U;

	for (;;)
	{
		if (pdPASS == xQueueReceive(cap_signal, &timeArray[indexArray], 0))
		{
			++indexArray;
			if (indexArray == 0x06U)
			{ // 6th samples received
			  // Check for valid
				xTaskCreate(response_task, "response_task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &response_task_handle);

			}

		}
	}
}

void sample_task(void *pvParameters)
{
	bool last_state, curr_state = false;
	uint16_t time_val = 0x00U;
	uint32_t sysTick = 0x00U;
	struct pulse curr_pulse;
	for (;;)
	{
		// Upd variables
		last_state = curr_state;
		++sysTick;
		// Sample input signal
		if ((GPIO_ISTAT(SAMPLE_PORT) & SAMPLE_PIN) == SAMPLE_PIN)
		{
			curr_state = true;
		}
		else
		{
			curr_state = false;
		}

		if (last_state == curr_state)
		{
			time_val++;
		}
		else
		{
			xQueueSendToBack(cap_signal, &((struct pulse){.state = last_state, .time = time_val}), 0);
			time_val = 0x00U;
		}
		// Signal generation

		// LED activity
		if ((sysTick % 500) == 0U)
		{
			GPIO_OCTL(LED_PORT) ^= LED_PIN;
		}
		vTaskDelay(pdMS_TO_TICKS(1));
	}
}

void response_task(void *pvParameters)
{
	const struct pulse response[] = {{.state = false, .time = 140}, {.state = true, .time = 160}, {.state = false, .time = 140}, {.state = true, .time = 160}, {.state = false, .time = 140}, {.state = true, .time = 160}};
	uint8_t response_index = 0x00U;
	for (;;)
	{
		if (response[response_index].state)
		{
			GPIO_OCTL(RESPONSE_PORT) |= RESPONSE_PIN;
		}
		else
		{
			GPIO_OCTL(RESPONSE_PORT) &= ~RESPONSE_PIN;
		}
		if (response_index < 0x06U)
		{
			vTaskDelay(pdMS_TO_TICKS(response[response_index++].time));
		}
		else
		{
			vTaskDelete(response_task_handle);
		}
	}
}
