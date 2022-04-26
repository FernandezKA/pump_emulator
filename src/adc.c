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


//Software used function
uint16_t u16ADC_Get_Mean(adc_simple* xADC){
	if (xADC->isFirst) {
		return 0x00U;
	}
	else{
		static uint32_t _sum = 0x00U;
		for(uint8_t i = 0; i < 0x0A; ++i){
			_sum += xADC->adc_val[i];
		}
		return (uint16_t) _sum / 0x0AU;
	}
}
void vAddSample(adc_simple* xADC, uint16_t _value){
	if(xADC->countSample < 0x0A) {
		xADC->adc_val[xADC->countSample++] = _value;
	}
	else{
		if(xADC->isFirst){
			xADC->isFirst = false;
		}			
		xADC->countSample = 0x00U;
		xADC->adc_val[xADC->countSample++] = _value;
	}
}

uint8_t u8Shift_Value(shift_reg* xShift, uint8_t _value){
	if (xShift->_isFirst){
		for(uint8_t i = 0; i < 120U; ++i){
			xShift->_val[i] = _value;
		}
		xShift->_isFirst = false;
		return xShift->_val[119U];
	}
	else{
		for(uint8_t i = 1; i <= 120U; ++i){
			xShift->_val[i] = xShift->_val[i - 1];
		}
		xShift->_val[0U] = _value;
		return xShift->_val[119U];
	}
}

