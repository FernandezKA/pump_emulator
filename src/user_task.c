/*
 * Project: pump emulator
 * File: user_task.c
 * Author: FernandezKA
 */

#include "user_task.h"

TaskHandle_t ad8400_0_task_handle = NULL;
TaskHandle_t ad8400_1_task_handle = NULL;
TaskHandle_t main_task_handle = NULL;
TaskHandle_t sample_task_handle = NULL;
TaskHandle_t response_task_handle = NULL;
TaskHandle_t send_info_task_handle = NULL;
TaskHandle_t adc_task_handle = NULL;
TaskHandle_t pwm_def_handle = NULL;

static inline void print(char *_data);

// This task used for definition AD8400 \
resistance with discrete timing, when defined by user
void ad8400_0_task(void *pvParameters)
{
	uint8_t res_value = 0x00U;
	for (;;)
	{
		vTaskDelay(pdMS_TO_TICKS(1000));
		_AD8400_set(res_value++, 0);
	}
}

void ad8400_1_task(void *pvParameters)
{
	uint8_t res_value = 0x00U;
	for (;;)
	{
		vTaskDelay(pdMS_TO_TICKS(1000));
		_AD8400_set(res_value++, 1);
	}
}

// It's a main task. Detect input sequence, then get action with defined rules.
void main_task(void *pvParameters)
{
	static struct pulse _tmp_pulse;
	static enum work_mode _mode;
	static uint8_t _valid_index = 0x00U;
	// Valid times definitions
	const static uint16_t valid_high = 160;
	const static uint16_t valid_low = 140;
	const static uint16_t stop_seq = 800;
	// Max deviation definitions
	const static uint16_t max_dev_high = valid_high / 10; // 10%
	const static uint16_t max_dev_low = valid_low / 10;	  // 10%
	const static uint16_t max_dev_stop = stop_seq / 10;	  // 10%
	// Used for action with timout
	static uint32_t _begin_responce_task = 0x00U;
	const uint32_t _diff_time_stop_responce = 0x14U;
	// Used for detect empty line (connected to Vss or Vdd)
	static uint32_t _last_capture_time = 0x00U;
	const static uint32_t _edge_capture_val = 0x0AU;
	// This variable for input measured pwm_value
	static uint8_t _pwm_measured = 0x00U;
	set_pwm(pwm_1, 0x0AU);
	enable_pwm(pwm_1);

	for (;;)
	{
		// Get pwm_2 only after 10sec. waiting
		//		if (SysTime > 10U && _mode != pwm_input)
		//		{
		//			set_pwm(pwm_2, 10);
		//			enable_pwm(pwm_2);
		//		}
		// If stop request isn't received more than _diff_time_stop_responce -> then suspend responce task
		if (SysTime - _begin_responce_task > _diff_time_stop_responce)
		{
			//vTaskSuspend(response_task_handle);
			_begin_responce_task = 0x00U;
		}

		// If state of line isn't different more then _edge_cap_val, then detect error
		if (SysTime - _last_capture_time > _edge_capture_val)
		{ // Check edge states of lilne (connected to Vss or Vdd)
			disable_pwm(pwm_1);
		}

		// If new sample is loaded into queue => parse it
		if (pdPASS == xQueueReceive(cap_signal, &_tmp_pulse, 0)) // Check capture signal
		{
			_last_capture_time = SysTime;
			// Divide by 3 groups - with knowledge timings
			if (_tmp_pulse.time < 120) // PWM case
			{
				_mode = pwm_input;
				enable_pwm(pwm_1);
			}
			else if (_tmp_pulse.time > 100 && _tmp_pulse.time < 500) // Request start detect
			{
				if (_tmp_pulse.state)
				{ // High state
					if (abs(_tmp_pulse.time - valid_high) < max_dev_high)
					{
						++_valid_index; // Detect valid of sequence at index, valid == 4
					}
					else
					{
						_valid_index = 0x00U;
					}
				}
				else
				{ // Low state
					if (abs(_tmp_pulse.time - valid_low) < max_dev_low)
					{
						++_valid_index;
					}
					else
					{
						_valid_index = 0x00U;
					}
				}

				if (_valid_index >= 0x03U) // valid_index - 1, because count from 0
				{
					_mode = start_input;
					_valid_index = 0x00U;
				}
				else
				{
					_mode = undef;
				}
			}
			else // Request stop detect
			{
				if ((abs(_tmp_pulse.time - stop_seq) < max_dev_stop))
				{
					_mode = stop_input;
				}
				else
				{
					_mode = undef;
				}
			}
		}

		switch (_mode)
		{
		case pwm_input:
			_mode = undef; // reset state for next capture
			if (pdPASS == xQueueReceive(pwm_value, &_pwm_measured, 0))
			{
				if (_pwm_measured < 11U)
				{
					set_pwm(pwm_1, 0U);
				}
				else if (_pwm_measured < 41U)
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
				enable_pwm(pwm_1);
			}
			else
			{
				// PWM measure less then 2 sec.
				__NOP(); // Need to delete, only for debug
			}
			break;

		case start_input:
			//				if (pdPASS != xTaskCreate(response_task, "responce_tesk", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &response_task_handle))
			//				{
			//					ERROR_HANDLER();
			//				}
			// vTaskResume(response_task_handle); // Begin generation responce for start request
			_valid_index = 0x00U;
			_begin_responce_task = SysTime;
			_mode = undef;
			break;

		case stop_input:
			// vTaskDelete(response_task_handle);
			//  vTaskSuspend(response_task_handle); // Suspend responce_task (TODO: delete for free space)
			_mode = undef;
			break;

		case undef:
			__NOP();
			break;
		}

		vTaskDelay(pdMS_TO_TICKS(1)); // Get answering timings
	}
}

// This task answering request pin, and send samples into queue in main process. Also used for blink led
void sample_task(void *pvParameters)
{
	bool last_state, curr_state = false;
	uint16_t time_val = 0x00U;
	uint32_t sysTick = 0x00U;
	struct pulse curr_pulse;
	static uint16_t _intSysCounter = 0x00U;
	static uint16_t _samples_pwm = 0x00U;
	static uint16_t _samples_pwm_index = 0x00U;
	static uint8_t pwm_fill = 0x00U;

	for (;;)
	{
		// Get pwm filling value
		if (_samples_pwm_index < 1999U)
		{ // Get pwm measuring at 2 seconds

			if ((GPIO_ISTAT(SAMPLE_PORT) & SAMPLE_PIN) == SAMPLE_PIN)
			{
				++_samples_pwm;
				++_samples_pwm_index;
			}
			else
			{
				++_samples_pwm_index;
			}
		}
		else
		{
			pwm_fill = (_samples_pwm * 100U) / _samples_pwm_index;
			_samples_pwm_index = 0x00U;
			_samples_pwm = 0x00U;
			xQueueSendToBack(pwm_value, &pwm_fill, 0);
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
			// Repeat input signal with inversion on PA0
			if (curr_state)
			{
				GPIO_OCTL(INV_PORT) &= ~INV_PIN;
			}
			else
			{
				GPIO_OCTL(INV_PORT) |= INV_PIN;
			}
			time_val = 0x00U;
		}

		// LED activity, blink every second
		if ((sysTick % 500) == 0U)
		{
			GPIO_OCTL(LED_PORT) ^= LED_PIN;
		}
		vTaskDelay(pdMS_TO_TICKS(1));
	}
}

// This task used for genrating responce signal at start request
void response_task(void *pvParameters)
{
	const struct pulse response[] = {{.state = true, .time = 180}, {.state = false, .time = 100}, {.state = true, .time = 100}, {.state = false, .time = 10}, {.state = true, .time = 190}, {.state = false, .time = 100}};
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
			response_index = 0x00U;
		}
	}
}

// TODO: delete, and use global variable for more effective using of memory
//  This task send info message to USART
void send_info_task(void *pvParameters)
{

	char _msg[0xFFU];
	uint8_t _arr_ptr = 0x00U;
	for (;;)
	{

		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

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

// This task used for more different pwm fillings forming (begin at detect pwm_fill on pwm_in)
void pwm_def_task(void *pvParameters)
{
	for (;;)
	{
		set_pwm(3, 50);
		vTaskDelay(pdMS_TO_TICKS(10000));
		set_pwm(3, 45);
		vTaskDelay(pdMS_TO_TICKS(10000));
		set_pwm(3, 55);
		vTaskDelay(pdMS_TO_TICKS(10000));
		set_pwm(3, 45);
		vTaskDelay(pdMS_TO_TICKS(10000));
	}
}

/***********************************************
//End of task functions
***********************************************/
