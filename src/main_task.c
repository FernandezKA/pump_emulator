#include "user_task.h"
void reset_pwm_value(void)
{
	set_pwm(pwm_1, 0U);
	set_pwm(pwm_2, 10U);
}

void reset_flags(void)
{
	start_req = bus_error = pwm_detect = false;
}

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
	// Used for detect empty line (connected to Vss or Vdd)
	static uint32_t _last_capture_time = 0x00U;
	const static uint32_t _edge_capture_val = 0x02U;
	// This variable for input measured pwm_value
	static uint8_t _pwm_measured = 0x00U;
	// For value from ADC
	// set_pwm(pwm_1, 0x0AU);
	// enable_pwm(pwm_1);
	disable_pwm(pwm_1);
	static bool pwm_enable_once = false;
	static bool pwm_main_enable = false;
	print("Ver. 2.2, 2022-05-20");
	/*************************************************************************************************
	 * ***********************************************************************************************
	 * ***********************************************************************************************/
	for (;;)
	{
		
		
		// If state of line isn't different more then _edge_cap_val, then detect error
		if (bus_error)
		{ // Check edge states of line (connected to Vss or Vdd)
			// disable_pwm(pwm_1);
			//reset_flags();
			_valid_index = 0x00U;
			set_pwm(pwm_2, 10U);
			disable_pwm(pwm_1);
			GPIO_OCTL(LED_ERROR_PORT) |= ERROR_LED;
		}
		else
		{
			GPIO_OCTL(LED_ERROR_PORT) &= ~ERROR_LED;
		}
		
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
		 * ***********************************************************************************************
		 * ***********************************************************************************************/
		/***********************************************************************************************/
		// Only parse start and stop detections
		if (pdPASS == xQueueReceive(cap_signal, &_tmp_pulse, 0)) // Check capture signal
		{
			_last_capture_time = SysTime;
			if (_tmp_pulse.time > 100 && _tmp_pulse.time < 300) // Request start detect
			{
				if (!_tmp_pulse.state)
				{ // High state
					if (_tmp_pulse.time > 144 && _tmp_pulse.time < 176U)
					{
						++_valid_index; // Detect valid of sequence at index, valid == 4
						set_pwm(pwm_2, 10U);
						reset_pwm_value();
					}
					else
					{
						_valid_index = 0x00U;
						//reset_flags();
						start_req = false;
					}
				}
				else
				{ // Low state
					if (_tmp_pulse.time > 136 && _tmp_pulse.time < 154)
					{
						++_valid_index;
						set_pwm(pwm_2, 10U);
						reset_pwm_value();
					}
					else
					{
						_valid_index = 0x00U;
						start_req = false;
						//reset_flags();
					}
				}
				if (_valid_index >= 0x05U) // valid_index - 1, because count from 0
				{
					vTaskDelay(pdMS_TO_TICKS(146U));
					if ((GPIO_ISTAT(SAMPLE_PORT) & SAMPLE_PIN) == SAMPLE_PIN)
					{
						vTaskResume(response_task_handle);
						reset_pwm_value();
						// xQueueReset(pwm_value);
						_begin_responce_task = SysTime;
						start_req = true;
					}
					else
					{
						start_req = false;
						//reset_flags();
					}
					_valid_index = 0x00U;
				}
			}

			else // Request stop detect
			{
				if (_tmp_pulse.state)
				{
					if (_tmp_pulse.time > stop_seq && _tmp_pulse.time < stop_seq + max_dev_stop)
					{
						reset_pwm_value();
						xQueueReset(pwm_value);
						if (NULL != response_task_handle)
						{
							vTaskSuspend(response_task_handle);
							GPIO_OCTL(RESPONSE_PORT) &= ~RESPONSE_PIN; // RESET PIN TO LOW STATE
							start_req = false;
						}
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
		if (pdPASS == xQueueReceive(pwm_value, &_pwm_measured, 0))
		{ // It's pwm mode, measure success
			_last_capture_time = SysTime;
			//pwm_detect = true;
			if (_pwm_measured < 11U)
			{
				// Set filling on PB1
				set_pwm(pwm_1, 0U);
				// Set filling on PA0
				if (NULL != pwm_def_task_handle)
				{
					vTaskSuspend(pwm_def_task_handle);
					reset_pwm_value();
				}
			}
			else
			{
				// reset_pwm_value();
				//  Def pwm filling value on PA0
				if (NULL == pwm_def_task_handle)
				{
					if (pdPASS == xTaskCreate(pwm_def_task, "pwm_def_task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, &pwm_def_task_handle))
					{
						__NOP();
					}
				}
				else
				{
					vTaskResume(pwm_def_task_handle);
				}
				// Def pwm_12 value
				if (_pwm_measured < 41U)
				{
					set_pwm(pwm_1, 30U);
				}
				else if (_pwm_measured < 81U)
				{
					set_pwm(pwm_1, 50U);
				}
				else
				{
					set_pwm(pwm_1, 80U);
				}
				if (pwm_enable_once)
				{
					enable_pwm(pwm_1);
				}
			}
		}
		vTaskDelay(pdMS_TO_TICKS(1)); // Get answering timings
	}
}