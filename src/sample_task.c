#include "user_task.h"
// This task answering request pin, and send samples into queue in main process. Also used for blink led
void sample_task(void *pvParameters)
{
	bool last_state, curr_state = false;
	uint16_t time_val = 0x00U;
	uint32_t sysTick = 0x00U;
	struct pulse curr_pulse;
	static uint16_t _intSysCounter = 0x00U;
	static uint8_t count_blink = 0x00U;

	struct
	{
		bool last_state, curr_state;
		uint16_t cap_time;
	} bus;
	bus.curr_state = bus.last_state = false;
	bus.cap_time = 0x20U;

	struct
	{
		uint16_t index, ones;
		uint8_t fill;
		bool is_measured;
	} pwm;
	pwm.index = pwm.ones = pwm.fill = 0x00U;
	pwm.is_measured = false;

	for (;;)
	{
		{ // Check bus state
			bus.last_state = bus.curr_state;
			bus.curr_state = ((GPIO_ISTAT(SAMPLE_PORT) & SAMPLE_PIN) == SAMPLE_PIN);

			if (bus.curr_state == bus.last_state)
			{
				bus.cap_time++;
			}
			else
			{
				if (bus.cap_time < 15U)
				{ // iNVERT PWM SIGNAL
					bus.curr_state ? (GPIO_OCTL(INV_PORT) &= ~INV_PIN) : (GPIO_OCTL(INV_PORT) |= INV_PIN);
					pwm_detect = TRUE;
				}
				bus.cap_time = 0x00U;
			}
		}

		{ // Reset pwm measure for long pulse
			if (bus.cap_time > 15U)
			{
				pwm.is_measured = false;
				pwm.index = 0x00U;
				pwm.ones = 0x00U;
				pwm.fill = 0x00U;
				xQueueSendToBack(pwm_value, &pwm.fill, 0);
				pwm_detect = false;
				(GPIO_OCTL(INV_PORT) &= ~INV_PIN); //Pull down line 
			}
		}
		
		{ // Measure pwm
			if (pwm.is_measured)
			{
				pwm.fill = (pwm.ones * 100U) / pwm.index;
				pwm.index = pwm.ones = 0x00U;
				pwm.is_measured = false;
				xQueueSendToBack(pwm_value, &pwm.fill, 0);
			}
			else
			{
				++pwm.index;
				if ((GPIO_ISTAT(SAMPLE_PORT) & SAMPLE_PIN) == SAMPLE_PIN)
					pwm.ones++;
				if (pwm.index == 1999U)
					pwm.is_measured = true;
				else
					pwm.is_measured = false;
			}
		}

		// Simplest time counter with inc. time = 1 sec
		if (_intSysCounter == 999U)
		{
			_intSysCounter = 0x00U;
			++SysTime;
		}
		else
		{
			++_intSysCounter;
		}
		// Detect type of input signal
		//  Upd variables
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
		// Check switch from low -> hight, or reversed
		if (last_state == curr_state)
		{
			time_val++;
		}
		else
		{
			// Send pulse on queue,will be received on main process
			xQueueSendToBack(cap_signal, &((struct pulse){.state = curr_state, .time = time_val}), 0);
			isCapture = true;
			time_val = 0x00U;
		}

		// LED activity, blink every second
		if ((sysTick % 250) == 0U)
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
		vTaskDelay(pdMS_TO_TICKS(1));
	}
}
