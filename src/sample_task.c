#include "user_task.h"

// User defines
#define ERR_BUS_TIME_SEC 0x02U
#define ERR_BUS_TIME_MS 2000U
#define PWM_TIME_EDGE 0x10U



// This task answering request pin, and send samples into queue in main process. Also used for blink led
void sample_task(void *pvParameters)
{
	uint32_t sysTick = 0x00U;
	struct pulse curr_pulse;

	struct
	{
		bool last_state, curr_state;
		uint16_t cap_time;
	} bus;
	bus.curr_state = bus.last_state = false;
	bus.cap_time = 0x00U;

	for (;;)
	{
		vTaskDelay(pdMS_TO_TICKS(1)); //Get frequency sampling is 100 Hz
		add_sample_pwm(&PWM);
		(check_pwm_valid(&PWM_VALIDATOR, bus.cap_time)) ? (pwm_detect = true) : (pwm_detect = false);
		{ // Check bus state
			bus.last_state = bus.curr_state;
			bus.curr_state = ((GPIO_ISTAT(SAMPLE_PORT) & SAMPLE_PIN) == SAMPLE_PIN);
			if (pwm_detect)
			{
				if (PWM.is_measured) //Only for fully measured value
				{
					get_pwm_action(get_pwm_fill(&PWM));
				}
			}
			else
			{
						get_clear_pwm_measure(&PWM);
						get_pwm_error_action();
			}
			if(bus_error){
				get_clear_pwm_measure(&PWM);
				get_pwm_error_action();
			}
//Detect bus error
			(bus.cap_time > ERR_BUS_TIME_MS) ? (bus_error = true):(bus_error = false);
//Get count pulse length time
			if (bus.curr_state == bus.last_state)
			{
				if(bus.cap_time < 0xFFFF){
				bus.cap_time++;
				}
			}
			else
			{
				// Check pwm valid value (on 10 edges, then invert current state
				(check_pwm_valid(&PWM_VALIDATOR, bus.cap_time)) ? (get_invert(bus.curr_state)):(get_invert(true));
				// Send into main parser
				if (bus.cap_time > PWM_TIME_EDGE)
				{ // Not send short pulse for analyzing
					//Erase pwm measure for correctly detecting filling coefficient
					get_clear_pwm_measure(&PWM);
					get_pwm_error_action();
					xQueueSendToBack(cap_signal, &((struct pulse){.state = bus.curr_state, .time = bus.cap_time}), 0);
				}
				//Reset cap. time value
				bus.cap_time = 0x00U;
			}
		}

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