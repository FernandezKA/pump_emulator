#include "user_task.h"

TaskHandle_t ad8400_task_handle = NULL;
TaskHandle_t main_task_handle = NULL;
TaskHandle_t sample_task_handle = NULL;
TaskHandle_t response_task_handle = NULL;
TaskHandle_t send_info_task_handle = NULL; 
TaskHandle_t adc_task_handle = NULL;

// This task used for definition AD8400 \
resistance with discrete timing, when defined by user
void ad8400_0_task(void *pvParameters)
{
	for (;;)
	{
	}
}

void ad8400_1_task(void *pvParameters)
{
	for (;;)
	{
	}
}

// It's a main task. Detect input sequence, then get action with defined rules.
void main_task(void *pvParameters)
{
	static struct pulse _tmp_pulse;
	static enum work_mode _mode;
	static uint8_t _valid_index = 0x00U;
	static uint16_t _pwm_hight_val = 0x00U;
	static uint16_t _pwm_low_val = 0x00U;
	static uint16_t pwm_fill = 0x00U;
	static uint8_t _pwm_index = 0x00U;
	const static uint16_t valid_high = 160;
	const static uint16_t valid_low = 140;
	const static uint16_t stop_seq = 800;
	const static uint16_t max_dev_high = valid_high / 10; // 10%
	const static uint16_t max_dev_low = valid_low / 10;	  // 10%
	const static uint16_t max_dev_stop = stop_seq * 0.3;  // 30%
	for (;;)
	{
		if (pdPASS == xQueueReceive(cap_signal, &_tmp_pulse, 0)) // Check capture signal
		{
			if (_tmp_pulse.time < 100)
			{
				if (_pwm_index < 10)
				{
					_mode = undef;
					if (!_tmp_pulse.state) // A little of black magic
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
					pwm_fill = (_pwm_hight_val * 100U) / (_pwm_hight_val + _pwm_low_val);
					_pwm_index = 0x00U;
					_pwm_hight_val = 0x00U;
					_pwm_low_val = 0x00U;
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
			if (pwm_fill > 0x32U)
			{
				set_pwm(2, 80);
				set_pwm(3, pwm_fill);
			}
			else
			{
				set_pwm(2, 10);
				set_pwm(3, pwm_fill);
			}
			enable_pwm(2);
			enable_pwm(3);
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
		vTaskDelay(pdMS_TO_TICKS(1));
	}
}

// This task answering request pin, and send samples into queue in main process. Also used for blink led
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

// This task used for genrating responce signal at start request
void response_task(void *pvParameters)
{
	const struct pulse response[] = {{.state = true, .time = 180}, {.state = false,\
	.time = 100}, {.state = true, .time = 100}, {.state = false, .time = 10},\
	{.state = true, .time = 190}, {.state = false, .time = 100}};
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
// This task send info message to USART
void send_info_task(void *pvParameters)
{

	for (;;)
	{
		vTaskDelay(100);
	}
}

//This task used for answering value from ADC
void adc_task(void *pvParameters)
{
	static uint16_t adc_value_0 = 0x00U;
	 for(;;)
	 {
		vTaskDelay(pdMS_TO_TICKS(1));
		if((ADC_STAT(ADC0) & ADC_STAT_EOC) == ADC_STAT_EOC){
			adc_value_0 = adc_regular_data_read(ADC0);
		}			
		if (!adc_flag_get(ADC0, ADC_FLAG_STRC))
		{
			adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL); 
		}
	 }
}