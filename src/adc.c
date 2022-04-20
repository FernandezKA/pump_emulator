#include "adc.h"

void adc_select_channel(uint8_t ch_num)
{
	if (ch_num == 0x02)
	{
		//			ADC_CTL0(ADC0)&=~((uint32_t) 0x1F);//Reset only field for channel select
		//			ADC_CTL0(ADC0) |= ch_num;
		adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_2, ADC_SAMPLETIME_1POINT5); // WHAT IS RANK??? (0 value)
	}
	else if (ch_num == 0x03)
	{
		// ADC_CTL0(ADC0)&=~((uint32_t) 0x1F);//Reset only field for channel select
		// ADC_CTL0(ADC0) |= ch_num;
		adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_3, ADC_SAMPLETIME_1POINT5); // WHAT IS RANK??? (0 value)
	}
	else
	{
		__NOP(); // Mistake (channel not avaliable)
	}
}

uint8_t adc_get_channel(void)
{
	return (ADC_CTL0(ADC0) & 0x1F);
}

uint16_t adc_get_result(void)
{
	return (uint16_t)(ADC_RDATA(ADC0) & 0xFFFF);
}

uint8_t get_value_0(uint16_t adc_value)
{
	adc_value = 0x0FFC & adc_value; //Use only 10 MSB bits
	adc_value = adc_value >> 2;
	float _tmp_val = 0x00U;
	_tmp_val = 0.0886 * adc_value + 0.5765;
	return (uint8_t)_tmp_val;
}

uint8_t get_value_1(uint16_t adc_value)
{
	adc_value = 0x0FFC & adc_value; //Use only 10 MSB bits
	adc_value = adc_value >> 2;
	float _tmp_val = 0x00U;
	_tmp_val = 1.12 * adc_value * adc_value + 14.63 * adc_value + 149.96;
	return (uint8_t)_tmp_val;
}
