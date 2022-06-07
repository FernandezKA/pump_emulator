#include "user_task.h"

// This task get timings for other tasks
void sample_task(void *pvParameters)
{
	uint32_t sysTick = 0x00U;

	for (;;)
	{
		vTaskDelay(pdMS_TO_TICKS(1));
		// Simplest time counter with inc. time = 1 sec
		if (sysTick == 999U)
		{
			sysTick = 0x00U;
			++SysTime;
		}
		else
		{
			++sysTick;
		}
		// LED activity, blink every second
		if ((sysTick % 250) == 0U)
		{
			get_blink();
		}
	}
}
/*This function get led indicate*/
void get_blink(void)
{
	GPIO_OCTL(LED_LIFE_PORT) ^= LIFE_LED; // Get led activity
	// LED ACT WITH IC
	if ((start_req | pwm_detect))
	{
		GPIO_OCTL(LED_RUN_PORT) ^= RUN_LED;
	}
	else
	{
		GPIO_OCTL(LED_RUN_PORT) &= ~RUN_LED;
	}
}