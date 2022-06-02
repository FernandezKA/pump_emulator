#include "user_task.h"

void main_task(void *pvParameters)
{
	// Static variable for this task
	static struct pulse _tmp_pulse;
	static enum work_mode _mode;
	static uint8_t _valid_index = 0x00U;
	// Valid times definitions

	const static uint16_t valid_high = 160U;
	const static uint16_t valid_low = 140U;
	const static uint16_t stop_seq = 800;
	// Max deviation definitions
	const static uint16_t max_dev_high = 16;  // 10%
	const static uint16_t max_dev_low = 14;	  // 10%
	const static uint16_t max_dev_stop = 300; // 10%
	// Used for action with timout
	static uint32_t _begin_responce_task = 0x00U;
	const uint32_t _diff_time_stop_responce = 0x14U;

	disable_pwm(pwm_1);
	static bool pwm_enable_once = false;
	static bool pwm_main_enable = false;
	print("Ver. 2.2, 2022-05-20\n\r");
	/*************************************************************************************************
	 * ***********************************************************************************************
	 * ***********************************************************************************************/
	for (;;)
	{
		(bus_error) ? (GPIO_OCTL(LED_ERROR_PORT) |= ERROR_LED):(GPIO_OCTL(LED_ERROR_PORT) &= ~ERROR_LED);		
		// First pwm enable
		if (SysTime > 2U && !pwm_main_enable)
		{
			enable_pwm(pwm_1);
			set_pwm(pwm_1, 10U);
			pwm_main_enable = true;
		}

		if (SysTime > 10U && !pwm_enable_once)
		{
			set_pwm(pwm_2, 10U);
			enable_pwm(pwm_2);
			pwm_enable_once = true;
		}

		if(_valid_index > 0U){
			get_clear_pwm_measure(&PWM);
			get_pwm_error_action();
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
			if (_tmp_pulse.time > 100 && _tmp_pulse.time < 300) // Request start detect
			{
				if (!_tmp_pulse.state)
				{ // High state
					if (_tmp_pulse.time > 144 && _tmp_pulse.time < 176U)
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
					if (_tmp_pulse.time > 136 && _tmp_pulse.time < 154)
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
				if (_tmp_pulse.state)
				{
					if (_tmp_pulse.time > stop_seq && _tmp_pulse.time < stop_seq + max_dev_stop)
					{
						//This case not depend on valid_index, because pwm action handled here
						get_clear_pwm_measure(&PWM);
						get_pwm_error_action();
						if (NULL != response_task_handle)
						{
							vTaskSuspend(response_task_handle);
							start_req = false;
						}
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
