#include "user_task.h"

TaskHandle_t ad8400_task_handle = NULL;
TaskHandle_t main_task_handle = NULL;
TaskHandle_t sample_task_handle = NULL;
TaskHandle_t response_task_handle = NULL;
TaskHandle_t send_info_task_handle = NULL;

void ad8400_task(void *pvParameters)
{
	for (;;)
	{
	}
}

void main_task(void *pvParameters)
{
	static struct pulse _tmp_pulse;
	static enum work_mode _mode;
	static uint8_t _valid_index = 0x00U;
	static uint16_t _pwm_hight_val = 0x00U;
	static uint16_t _pwm_low_val = 0x00U;
	static double pwm_fill = 0x00U;
	static uint8_t _pwm_index = 0x00U;
	const static uint16_t valid_high = 160;
	const static uint16_t valid_low = 140;
	const static uint16_t stop_seq = 800;
	const static uint16_t max_dev_high = valid_high / 10; // 10%
	const static uint16_t max_dev_low = valid_low / 10;	  // 10%
	const static uint16_t max_dev_stop = stop_seq * 0.3;  // 10%
	for (;;)
	{
		if (pdPASS == xQueueReceive(cap_signal, &_tmp_pulse, 0)) // Check capture signal
		{
			usart_data_transmit(USART0, ((_tmp_pulse.time >> 8) & 0xFFU));
			usart_data_transmit(USART0, (_tmp_pulse.time & 0xFFU));
			if (_tmp_pulse.time < 100)
			{
				if (_pwm_index < 10)
				{
					_mode = undef;
					if (_tmp_pulse.state)
					{
						_pwm_hight_val += _tmp_pulse.time;
					}
					else
					{
						_pwm_low_val += _tmp_pulse.time;
					}
					++_pwm_index;
				}
				else
				{
					_mode = pwm_input;
					pwm_fill = _pwm_hight_val / (_pwm_hight_val + _pwm_low_val);
					_pwm_index = 0x00U;
					_pwm_hight_val = 0x00U;
					_pwm_hight_val = 0x00U;
				}
			}
			else if (_tmp_pulse.time > 100 && _tmp_pulse.time < 500)
			{
				if (_tmp_pulse.state)
				{ // High state
					if (abs(_tmp_pulse.time - valid_high) < max_dev_high)
					{
						++_valid_index;
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

				if (_valid_index > 0x3U)
				{
					_mode = start_input;
					_valid_index = 0x00U;
				}
				else
				{
					_mode = undef;
				}
			}
			else
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
			_mode = undef;
			// TODO: Add pwm generate
			break;

		case start_input:
			vTaskResume(response_task_handle);
			_valid_index = 0x00U;
			_mode = undef;
			break;

		case stop_input:
			vTaskSuspend(response_task_handle);
			_mode = undef;
			break;

		case undef:

			break;
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
			xQueueSendToBack(cap_signal, &((struct pulse){.state = curr_state, .time = time_val}), 0);
			time_val = 0x00U;
		}
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

void send_info_task(void *pvParameters)
{

	for (;;)
	{
		vTaskDelay(100);
	}
}