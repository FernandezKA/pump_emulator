#include "adc.h"

void adc_select_channel(uint8_t ch_num)
{
	if (ch_num == 0x02)
	{
		adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_2, ADC_SAMPLETIME_1POINT5); // WHAT IS RANK??? (0 value)
	}
	else if (ch_num == 0x03)
	{
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

// Software used function
uint16_t u16ADC_Get_Mean(adc_simple *xADC)
{
		static uint32_t _sum = 0x00U;
		for (uint8_t i = 0; i < 0x0A; ++i)
		{
			_sum += xADC->adc_val[i];
		}
		return (uint16_t)_sum / 0x0AU;
}
void vAddSample(adc_simple *xADC, uint16_t _value)
{
	if (xADC->countSample < 0x0A)
	{
		xADC->adc_val[xADC->countSample++] = _value;
	}
	else
	{
		if (xADC->isFirst)
		{
			xADC->isFirst = false;
		}
		xADC->countSample = 0x00U;
		xADC->adc_val[xADC->countSample++] = _value;
	}
}

uint8_t u8Shift_Value(shift_reg *xShift, uint8_t _value)
{
	if (xShift->_isFirst)
	{
		for (uint8_t i = 0; i < 120U; ++i)
		{
			xShift->_val[i] = _value;
		}
		xShift->_isFirst = false;
		return xShift->_val[119U];
	}
	else
	{
		for (uint8_t i = 1; i <= 120U; ++i)
		{
			xShift->_val[i] = xShift->_val[i - 1];
		}
		xShift->_val[0U] = _value;
		return xShift->_val[119U];
	}
}

void vShiftInit(shift_reg *xReg)
{
	xReg->_isFirst = true;
}

// ADC conversion

uint8_t u8GetConversionValue(uint16_t _adc)
{
	static float _result = 0;
	static uint16_t adc;
	adc = _adc & 0x3FFU;
	_result = 0.28 * adc * adc + 3.66 * adc + 37.49;
	return _result;
}

void vSimpleADC_Init(adc_simple *xADC)
{
	xADC->isFirst = true;
	xADC->countSample = 0x00U;
}

bool bGetMeanValue(uint32_t* _sum, uint16_t* _mean, uint8_t* _counter, uint16_t _adc_val){
	bool _status = false;
	if(*_counter <MEAN_NUMS){
		*_sum += _adc_val;
		++*_counter;
	}
	else{
		*_mean = (uint16_t) (*_sum / MEAN_NUMS);
		_status = true;
	}
	return _status;
}
