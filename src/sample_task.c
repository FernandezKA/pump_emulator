#include "user_task.h"
// This task answering request pin, and send samples into queue in main process. Also used for blink led
void sample_task(void *pvParameters)
{
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
	bus.cap_time = 0x00U;
	uint16_t pwm_cap_time = 0x00U;

	for (;;)
	{
		add_sample_pwm(&PWM);
		(check_pwm_valid(&PWM_VALIDATOR, bus.cap_time)) ? (pwm_detect = true) : (pwm_detect = false);
		{ // Check bus state
			bus.last_state = bus.curr_state;
			bus.curr_state = ((GPIO_ISTAT(SAMPLE_PORT) & SAMPLE_PIN) == SAMPLE_PIN);
			
			if(pwm_detect){
					if(PWM.is_measured){
						get_pwm_action(get_pwm_fill(&PWM));
						pwm_cap_time = SysTime;
					}
			}
			else{
					if(SysTime - pwm_cap_time > 2U){
					get_pwm_error_action();
					get_clear_pwm_measure(&PWM);
					}
			}

			if (bus.curr_state == bus.last_state)
			{
				bus.cap_time++;
			}
			else
			{
				// Check pwm valid value (on 10 edges)
				if (check_pwm_valid(&PWM_VALIDATOR, bus.cap_time))
				{
					get_invert(bus.curr_state);
				}
				else
				{
					get_invert(true); // pull down invert line
				}
				// Send into main parser
				if (bus.cap_time > 100U)
				{ // Not send short pulse for analyzing
					xQueueSendToBack(cap_signal, &((struct pulse){.state = bus.curr_state, .time = bus.cap_time}), 0);
				}
				bus.cap_time = 0x00U;
			}
		}

		{ // Reset pwm measure for long pulse

			if (bus.cap_time > 2000)
			{ // detect bus error
				bus_error = true;
				get_pwm_error_action();
			}
			else
			{
				bus_error = false;
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

		++sysTick;

		// LED activity, blink every second
		if ((sysTick % 250) == 0U)
		{
			get_blink();
		}
		// taskEXIT_CRITICAL();
		vTaskDelay(pdMS_TO_TICKS(1));
		//taskYIELD();
	}
}

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