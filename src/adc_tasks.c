//#include "main.h"
#include "user_task.h"

void ad8400_0_task(void *pvParameters)
{
#define sample_time 10U
	uint8_t res_value = 0x00U;
	static shift_reg reg; // shift register for delay buffer
	static adc_simple _adc;
	static adc_state _eStateADC = not_measured;
	static uint16_t _u16Measure = 0x00U;
	static uint8_t _u8NewConversion = 0x00U;

	//LPF filter for measure
	static uint16_t mean_adc;
	static uint32_t sum_adc = 0x00U;
	static uint8_t counter_adc = 0x00U;
	static volatile uint16_t _u16Mean = 0x00U;
	adc_select_channel(0x02U);
	vShiftInit(&reg);
	vSimpleADC_Init(&_adc);
	for (;;)
	{
		// Get new measure
		if(pdPASS == xQueueReceive(adc_0, &_u16Measure, 0)){
		//_u16Measure = adc_regular_data_read(ADC0);
		if (_adc.isFirst)
		{ // Get mark voltage on bus
			switch (_eStateADC)
			{
			case not_measured:
				vAddSample(&_adc, _u16Measure);
				if (_adc.countSample == 0x0B)
				{ // Get 1 sec. measure
					_u16Mean = 0x00U;
					_u16Mean = u16ADC_Get_Mean(&_adc);
					// Detect bus state
					if (_u16Mean < 39U)
					{
						_eStateADC = error;
					}
					else if (_u16Mean > 208U)
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
					conversion_result = u8Shift_Value(&reg, _u8NewConversion);
					_AD8400_set(conversion_result, 1);
				}
				else
				{			 // Wait new sample
					__NOP(); // For debug
				}
				vTaskDelay(pdMS_TO_TICKS(100U)); // 100 ms * 10 samples = 1 sec from measure to measure
				break;

			case error:
				// Reset flags of state
				conversion_result = 0x00U;
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
}

void ad8400_1_task(void *pvParameters)
{
	uint8_t res_value = 250U;
	for (;;)
	{
		if(res_value > 40){
			_AD8400_set(res_value--, 0); //U4
		}
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}
// This task used for measure value on ADC
void adc_task(void *pvParameters)
{
	static uint16_t adc_value_0 = 0x00U;
	static uint16_t adc_value_1 = 0x00U;
	for (;;)
	{
		//Check conversion
			if (adc_flag_get(ADC0, ADC_FLAG_EOC))//If conversion is complete - parse it
			{
				adc_flag_clear(ADC0, ADC_FLAG_EOC);
				if (0x02 == adc_get_channel()) //Answer number of channel, then set new channel (other)
				{
					adc_value_0 = adc_regular_data_read(ADC0) >> 4;
					xQueueSendToBack(adc_0, &adc_value_0, 0);
					//xQueueSendToBack(adc_0, adc_value_0>>4);
					adc_select_channel(0x03);
				}
				else
				{
					// adc_value_1 = adc_regular_data_read(ADC0);
					// global_adc_1 = adc_value_1>>4;
					add_measure(&therm_int, adc_regular_data_read(ADC0)>>4);
					adc_select_channel(0x02);
				}
				//adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);
			}
			adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);
		vTaskDelay(pdMS_TO_TICKS(50));
	}
}