#include "user_task.h"

void main_task(void *pvParameters)
{
	// Static variable for this task
	static struct pulse _tmp_pulse;
	static enum work_mode _mode;
	static uint8_t _valid_index = 0x00U;
	// Valid times definitions

	const static uint16_t valid_high = 1600U;
	const static uint16_t valid_low = 1400U;
	const static uint16_t stop_seq = 8000;
	// Max deviation definitions
	const static uint16_t max_dev_high = 160;  // 10%
	const static uint16_t max_dev_low = 140;	  // 10%
	const static uint16_t max_dev_stop = 3000; // 10%
	// Used for action with timout
	static uint32_t _begin_responce_task = 0x00U;
	const uint32_t _diff_time_stop_responce = 0x14U;
	uint32_t last_cap_time = 0x03U;

	disable_pwm(pwm_1);
	disable_pwm(pwm_2);
	static bool pwm_enable_once = false;
	static bool pwm_main_enable = false;
	
	PWM.is_valid = false;
	pwm_detect = false;
	get_pwm_error_action();
	get_clear_pwm_measure(&PWM);
	
	print("Ver. 2.2, 2022-05-20\n\r");
	/*************************************************************************************************
	 * ***********************************************************************************************
	 * ***********************************************************************************************/
	for (;;)
	{
		//Get bus error led indication 
		(bus_error) ? (GPIO_OCTL(LED_ERROR_PORT) |= ERROR_LED):(GPIO_OCTL(LED_ERROR_PORT) &= ~ERROR_LED);	
		//Get pwm measure
		if(PWM.is_valid){//Only measure PWM signal
			add_sample_pwm(&PWM);	
		}

		if (SysTime > 10U && !pwm_enable_once)
		{
			set_pwm(pwm_1, 10U);
			enable_pwm(pwm_1);
			pwm_enable_once = true;
		}

		if(_valid_index > 0U){
			get_clear_pwm_measure(&PWM);
			get_pwm_error_action();
		} 
		
		if(SysTime - last_cap_time > 2U){//It's a bus error
			bus_error = true;
			pwm_detect = false;
			get_clear_pwm_measure(&PWM);
			get_pwm_error_action();			
		}
		else{
			bus_error = false;
		}

		// If stop request isn't received more then _diff_time_stop_responce -> get suspend responce task
		if (SysTime - _begin_responce_task > _diff_time_stop_responce)
		{
			//reset_flags();
			if (NULL != response_task_handle)
			{
				vTaskSuspend(response_task_handle);
				GPIO_OCTL(RESPONSE_PORT) &= ~RESPONSE_PIN; // RESET PIN TO LOW STATE
			}
			start_req = false;
			_begin_responce_task = 0x00U;
		}
		/*************************************************************************************************
		************************************************************************************************/
		// Only parse start and stop detections
		if (pdPASS == xQueueReceive(cap_signal, &_tmp_pulse, 0)) // Check capture signal
		{
			last_cap_time = SysTime;
			//Check for pwm signal
			if(_tmp_pulse.time <= 100){ //It's a pwm signal 
				PWM.is_valid = true;
				if(PWM.is_measured){
					get_pwm_action(get_pwm_fill(&PWM));
					pwm_detect = true;
				}
			}
			else{
				PWM.is_valid = false;
				pwm_detect = false;
				get_pwm_error_action();
				get_clear_pwm_measure(&PWM);
			}
			//Chech for start request signal 
			if (_tmp_pulse.time > 1000 && _tmp_pulse.time < 3000) // Request start detect
			{
				if (_tmp_pulse.state)
				{ // High state
					if (_tmp_pulse.time > 1440 && _tmp_pulse.time < 1760U)
					{
						++_valid_index; // Detect valid of sequence at index, valid == 4
					}
					else
					{
						_valid_index = 0x00U;
						start_req = false;
					}
				}
				else
				{ // Low state
					if (_tmp_pulse.time > 1360 && _tmp_pulse.time < 1540)
					{
						++_valid_index;
					}
					else
					{
						_valid_index = 0x00U;
						start_req = false;
					}
				}
				if (_valid_index >= 0x05U) // valid_index - 1, because count from 0
				{
					vTaskDelay(pdMS_TO_TICKS(146U)); //check state on bus on edge + time - dev
					if ((GPIO_ISTAT(SAMPLE_PORT) & SAMPLE_PIN) == SAMPLE_PIN)
					{
						vTaskResume(response_task_handle);
						_begin_responce_task = SysTime;
						start_req = true;
					}
					else
					{
						start_req = false;
					}
					GPIO_OCTL(RESPONSE_PORT) &= ~RESPONSE_PIN; // RESET PIN TO LOW STATE
					_valid_index = 0x00U;
				}
			}

			else // Request stop detect
			{
				if (!_tmp_pulse.state)
				{
					if (_tmp_pulse.time > stop_seq && _tmp_pulse.time < stop_seq + max_dev_stop)
					{
						//This case not depend on valid_index, because pwm action handled here
						if (NULL != response_task_handle)
						{
							vTaskSuspend(response_task_handle);
							start_req = false;
						}
						get_clear_pwm_measure(&PWM);
						get_pwm_error_action();
						GPIO_OCTL(RESPONSE_PORT) &= ~RESPONSE_PIN; // RESET PIN TO LOW STATE
						_valid_index = 0x00U;
					}
				}
				else
				{
					_valid_index = 0x00U;
				}
			}
		}
		/**********************************************************************************************/
		vTaskDelay(pdMS_TO_TICKS(1)); // Get answering timings
	}
}
