#include "adc.h"

float therm_res[] = {7.91, 6.10, 4.74, 3.71, 2.94, 2.34, 1.87, 1.51, 1.22, 1, 0.82, 0.68, 0.56, 0.47, 0.39, 0.33, 0.28, 0.23, 0.20}; //with step at 5 deg. 

void adc_select_channel(uint8_t ch_num)
{
	if (ch_num == 0x02)
	{
		ADC_RSQ2(ADC0) &= ~(1<<4|1<<3|1<<2|1<<1|1<<0);
		ADC_RSQ2(ADC0) |= 0x02;
		//adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_2, ADC_SAMPLETIME_1POINT5); // WHAT IS RANK??? (0 value)
	}
	else if (ch_num == 0x03)
	{
		ADC_RSQ2(ADC0) &= ~(1<<4|1<<3|1<<2|1<<1|1<<0);
		ADC_RSQ2(ADC0) |= 0x03;
	}
	else
	{
		__NOP(); // Mistake (channel not avaliable)
	}
}

uint8_t adc_get_channel(void)
{
	return (ADC_RSQ2(ADC0) & 0X1f);
}

uint16_t adc_get_result(void)
{
	return (uint16_t)(ADC_RDATA(ADC0) & 0xFFFF);
}

// Software used function
uint16_t u16ADC_Get_Mean(adc_simple *xADC)
{
	static volatile uint32_t _sum;
	_sum = 0x00U;
	for (uint8_t i = 0; i < 0x0A; ++i)
	{
		_sum += xADC->adc_val[i];
	}
	return (uint16_t)(_sum / 0x0AU);
}
void vAddSample(adc_simple *xADC, uint16_t _value)
{
	if (xADC->countSample < 0x0BU)
	{
		xADC->adc_val[xADC->countSample++ - 1U] = _value;
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
	static float _result = 0, voltage = 0;
	static uint16_t adc;
	voltage = (float)adc/(float)77;
	measured_ad8400 = voltage;
	adc = _adc & 0x3FFU;
	_result = 0.28 * voltage * voltage + 3.66 * voltage + 37.49;
	return _result;
}

void vSimpleADC_Init(adc_simple *xADC)
{
	xADC->isFirst = true;
	xADC->countSample = 0x00U;
}

bool bGetMeanValue(uint32_t *_sum, uint16_t *_mean, uint8_t *_counter, uint16_t _adc_val)
{
	bool _status = false;
	if (*_counter < MEAN_NUMS)
	{
		*_sum += _adc_val;
		++*_counter;
	}
	else
	{
		*_mean = (uint16_t)(*_sum / MEAN_NUMS);
		_status = true;
	}
	return _status;
}

uint16_t _from_voltage(float value)
{
	return (uint16_t)value * 1241U;
}

float _to_voltage(uint16_t val){
	return (float) val/77.3F;
}


/************************************************************
*****THIS SECTION FOR THERMAL RESISTOR PARSER****************
************************************************************/
//Add new samples to the circle buffer
void add_measure(struct therm_res* xRes, uint16_t ADC_val)
{
 if(xRes->index_arr < 10U){
	 xRes->samples_arr[xRes ->index_arr++] = ADC_val;
 }
 else{  //Get circle buffer
	 xRes->index_arr = 0x00U;
	 xRes->samples_arr[xRes ->index_arr++] = ADC_val;
 }
}
//Return mean value on ADC format
uint16_t get_mean_therm(struct therm_res* xRes)
{
 volatile uint32_t _sum;
_sum = 0x00U;
	for(uint8_t i = 0; i < 10U; ++i){
		 _sum  += xRes->samples_arr[i];
	}
	return (uint16_t) _sum / 10U;
}
//Return divide coefficient for resistive divider
float get_div_coeff(struct therm_res* xRes)
{
	return (float) (3.3F/2)/xRes->v_out; 
}
//Combine all steps of conversion from voltage to temp
void get_temp_int_conversion(struct therm_res* xRes)
{
	xRes->v_out = _to_voltage(get_mean_therm(xRes));
	xRes->div_coeff = get_div_coeff(xRes);
	xRes->temp = get_temp(xRes);
}
//Reset all fields to default value
void reset_therm_struct(struct therm_res* xRes){
	 xRes->div_coeff = 0x00U; 
	xRes->index_arr = 0x00U; 
	for(uint8_t i = 0; i < 10U; ++i){
	xRes ->samples_arr[i]= 0U;
	}
	xRes->temp = 0x00U;
	xRes->v_out = 0;
}
//Convert value from voltage to temperature
uint8_t get_temp(struct therm_res* xRes){
	uint8_t index_arr = 0;
	while(index_arr < sizeof(xRes->div_coeff)/sizeof(float) || xRes->div_coeff < therm_res[index_arr]){
		++index_arr;
	}
	return (index_arr - 1)* 5;
}