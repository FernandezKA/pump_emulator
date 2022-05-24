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
TaskHandle_t adc_task_handle = NULL;
TaskHandle_t pwm_def_task_handle = NULL;
TaskHandle_t uart_info_task_handle = NULL;

// This task used for definition AD8400 \
resistance with discrete timing, when defined by user
void ad8400_0_task(void *pvParameters)
{
#define sample_time 10U
	uint8_t res_value = 0x00U;
	static shift_reg reg; // shift register for delay buffer
	static adc_simple _adc;
	static adc_state _eStateADC = not_measured;
	static uint16_t _u16Measure = 0x00U;
	static uint8_t _u8NewConversion = 0x00U;

	// For LF filter for measure
	static uint16_t mean_adc;
	static uint32_t sum_adc = 0x00U;
	static uint8_t counter_adc = 0x00U;
	static uint16_t _u16Mean = 0x00U;
	adc_select_channel(0x02U);
	vShiftInit(&reg);
	vSimpleADC_Init(&_adc);

	for (;;)
	{
		// Get new measure
		_u16Measure = adc_regular_data_read(ADC0);
		;
		if (_adc.isFirst)
		{ // Get mark voltage on bus
			switch (_eStateADC)
			{
			case not_measured:
				vAddSample(&_adc, _u16Measure);
				if (_adc.countSample == 0x0B)
				{ // Get 1 sec. measure
					_u16Mean = u16ADC_Get_Mean(&_adc);
					// Detect bus state
					if (_u16Mean < _from_voltage(0.5))
					{
						_eStateADC = error;
					}
					else if (_u16Mean > _from_voltage(2.7))
					{
						_eStateADC = error;
					}
					else
					{
						_eStateADC = measured;
					}
				}
				vTaskDelay(pdMS_TO_TICKS(100U));
				break;

			case measured: // From this state we can get out only with reset
				// Get action with shift register
				if (bGetMeanValue(&sum_adc, &mean_adc, &counter_adc, _u16Measure))
				{													   // 10 samples is received
					_u8NewConversion = u8GetConversionValue(mean_adc); // TODO: this used fixedd value for test, after add value from ADC
					_AD8400_set(u8Shift_Value(&reg, _u8NewConversion), 0);
				}
				else
				{			 // Wait new sample
					__NOP(); // For debug
				}
				vTaskDelay(pdMS_TO_TICKS(100U)); // 100 ms * 10 samples = 1 sec from measure to measure
				break;

			case error:
				// Reset flags of state
				_eStateADC = not_measured;
				vSimpleADC_Init(&_adc);			  // Reset all of parameters into measured simplest ADC
				vTaskDelay(pdMS_TO_TICKS(1000U)); // Wait 1 sec. for new measure, then repeat again
				break;
			}
		}
		else
		{ // Get value into the shift register
		}
		//_AD8400_set(res_value++, 0);
	}
}

void ad8400_1_task(void *pvParameters)
{
	uint8_t res_value = 250U;
	for (;;)
	{
		if (res_value > 40U)
		{
			_AD8400_set(res_value--, 1);
		}
		vTaskDelay(pdMS_TO_TICKS(6000));
	}
}

// It's a main task. Detect input sequence, then get action with defined rules.
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
			// set_pwm(pwm_1, 10U);
			// enable_pwm(pwm_1);
			enable_pwm(pwm_2);
			pwm_enable_once = true;
		}
		// If stop request isn't received more then _diff_time_stop_responce -> get suspend responce task
		if (SysTime - _begin_responce_task > _diff_time_stop_responce)
		{
			if (NULL != response_task_handle)
			{
				vTaskSuspend(response_task_handle);
				GPIO_OCTL(RESPONSE_PORT) &= ~RESPONSE_PIN; // RESET PIN TO LOW STATE
			}
			_begin_responce_task = 0x00U;
		}
		/*************************************************************************************************
		 * ***********************************************************************************************
		 * ***********************************************************************************************/
		// If state of line isn't different more then _edge_cap_val, then detect error
		if (SysTime - _last_capture_time > _edge_capture_val)
		{ // Check edge states of line (connected to Vss or Vdd)
			// disable_pwm(pwm_1);
			_valid_index = 0x00U;
			set_pwm(pwm_1, 10U);
			set_pwm(pwm_2, 10U);
			disable_pwm(pwm_1);
			GPIO_OCTL(LED_START_PORT) |= START_LED;
		}
		else{
			GPIO_OCTL(LED_START_PORT) &=~START_LED;
		}
		/***********************************************************************************************/
		// If new sample is loaded into queue => parse it
		if (pdPASS == xQueueReceive(cap_signal, &_tmp_pulse, 0)) // Check capture signal
		{
			_last_capture_time = SysTime;
			// Divide by 3 groups - with knowledge timings
			if (_tmp_pulse.time < 15) // PWM case
			{
				_mode = pwm_input;
				_valid_index = 0x00U;
				// enable_pwm(pwm_1);
			}

			else if (_tmp_pulse.time > 100 && _tmp_pulse.time < 300) // Request start detect
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
					}
				}

				// Imp: valid start sequence index will be cutted from 3 to 2, for most sensitive detecting
				if (_valid_index >= 0x06U) // valid_index - 1, because count from 0
				{
					print("Start seq. detected\n\r");
					vTaskResume(response_task_handle);
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
				if (_tmp_pulse.state)
				{
					if (_tmp_pulse.time > stop_seq && _tmp_pulse.time < stop_seq + max_dev_stop)
					{
						_mode = stop_input;
						_valid_index = 0x00U;
					}
					else
					{
						_mode = undef;
					}
				}
				else
				{
					_mode = undef;
					_valid_index = 0x00U;
				}
			}
		}
		/**********************************************************************************************/
		switch (_mode)
		{
		case pwm_input:
			_mode = undef; // reset state for next capture
						   // Repeat input signal with inversion on PA0
			if ((GPIO_ISTAT(SAMPLE_PORT) & SAMPLE_PIN) == SAMPLE_PIN)
			{
				GPIO_OCTL(INV_PORT) &= ~INV_PIN;
			}
			else
			{
				GPIO_OCTL(INV_PORT) |= INV_PIN;
			}
			/************************************************************/
			if (pdPASS == xQueueReceive(pwm_value, &_pwm_measured, 0))
			{
				if (_pwm_measured < 11U)
				{
					// Set filling on PB1
					set_pwm(pwm_1, 0U);
					// Set filling on PA0
					if (NULL != pwm_def_task_handle)
					{
						vTaskSuspend(pwm_def_task_handle);
						set_pwm(pwm_2, 10U);
					}
				}
				else
				{
					// Def pwm filling value on PA0
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
			else
			{
				// PWM measure less then 2 sec.
				__NOP(); // Need to delete, only for debug
			}
			break;

		case start_input:
			print("Start request detected\n\r");
			// RESET pwm to default value
			set_pwm(pwm_1, 10U);
			set_pwm(pwm_2, 10U);
			xQueueReset(pwm_value);
			_valid_index = 0x00U;
			_begin_responce_task = SysTime;
			_mode = undef;
			break;

		case stop_input:
			set_pwm(pwm_1, 10U);
			set_pwm(pwm_2, 10U);
			xQueueReset(pwm_value);
			print("Stop detected\n\r");
			if (NULL != response_task_handle)
			{
				vTaskSuspend(response_task_handle);
				GPIO_OCTL(RESPONSE_PORT) &= ~RESPONSE_PIN; // RESET PIN TO LOW STATE
			}
			else
			{
				// TODO: Add msg about unstarted task
			}
			_mode = undef;
			break;

		case undef:
			__NOP();
			break;
		}
		vTaskDelay(pdMS_TO_TICKS(1)); // Get answering timings
	}
}
/*************************************************************************************************
 * ***********************************************************************************************
 * ***********************************************************************************************/
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
			time_val = 0x00U;
		}

		// LED activity, blink every second
		if ((sysTick % 250) == 0U)
		{

			GPIO_OCTL(LED_RUN_PORT) ^= RUN_LED; // Get led activity
			// led activity with start capture
//			if (response_task_handle != NULL)
//			{
//				if (eTaskGetState(response_task_handle) != eSuspended)
//				{
//					GPIO_OCTL(LED_START_PORT) ^= START_LED;
//				}
//				else
//				{
//					GPIO_OCTL(LED_START_PORT) &= ~START_LED;
//				}
//			}
//			else
//			{
//				GPIO_OCTL(LED_START_PORT) &= ~START_LED;
//			}

			// LED ACT WITH IC
			if (time_val < 2000U)
			{ // Idle state edge is 5 sec.
				GPIO_OCTL(LED_ACT_PORT) ^= ACT_LED;
			}
			else
			{
				GPIO_OCTL(LED_ACT_PORT) &= ~ACT_LED;
			}
		}
		vTaskDelay(pdMS_TO_TICKS(1));
	}
}
/*************************************************************************************************
 * ***********************************************************************************************
 * ***********************************************************************************************/
// This task used for genrating responce signal at start request
void response_task(void *pvParameters)
{
	const struct pulse response[] = {{.state = true, .time = 100}, {.state = false, .time = 20}, {.state = true, .time = 180}, {.state = false, .time = 100},\
	{.state = true, .time = 100}, {.state = false, .time = 10}, {.state = true, .time = 100}, {.state = false, .time = 100}};
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
		if (response_index < 0x08U)
		{
			vTaskDelay(pdMS_TO_TICKS(response[response_index++].time));
		}
		else
		{
			response_index = 0x00U;
		}
	}
}

/*************************************************************************************************
 * ***********************************************************************************************
 * ***********************************************************************************************/
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
/*************************************************************************************************
 * ***********************************************************************************************
 * ***********************************************************************************************/
// This task used for more different pwm fillings forming (begin at detect pwm_fill on pwm_in)
void pwm_def_task(void *pvParameters)
{
	for (;;)
	{
		set_pwm(pwm_2, 50U);
		vTaskDelay(pdMS_TO_TICKS(10000));
		set_pwm(pwm_2, 45U);
		vTaskDelay(pdMS_TO_TICKS(10000));
		set_pwm(pwm_2, 55U);
		vTaskDelay(pdMS_TO_TICKS(10000));
		set_pwm(pwm_2, 45U);
		vTaskDelay(pdMS_TO_TICKS(10000));
	}
}
/*************************************************************************************************
 * ***********************************************************************************************
 * ***********************************************************************************************/
// This task used for send uart info messages
void uart_info_task(void *pvParameters)
{
	static uint8_t u8Rec_Buff;
	for (;;)
	{
		// Check fifo buff state
		while (pdPASS == xQueueReceive(uart_info, &u8Rec_Buff, 0))
		{
			vSendByte(u8Rec_Buff);
		}
		xQueueReset(uart_info);
		vTaskDelay(pdMS_TO_TICKS(1U)); // Check info buffer every second
	}
}

/***********************************************
//End of tasks functions
***********************************************/
