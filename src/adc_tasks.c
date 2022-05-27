//#include "main.h"
#include "user_task.h"
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
		//_AD8400_set(res_value--, 1);
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}