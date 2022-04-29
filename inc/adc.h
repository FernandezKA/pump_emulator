#ifndef _adc_h_
#define _adc_h_
#include "main.h"

#define MEAN_NUMS 0x0AU

enum _adc_state{
	not_measured, 
	measured,
	error
};

struct 	_shift_reg{
	uint16_t _val[120U];
	bool _isFirst;
};

struct _adc_simple_measure{
	uint16_t adc_val[10U];
	uint8_t countSample;
	bool isFirst; 
};

typedef struct _adc_simple_measure adc_simple;
typedef struct _shift_reg shift_reg; 
typedef enum _adc_state adc_state;

//This part for user
void vSimpleADC_Init(adc_simple* xADC);
uint16_t u16ADC_Get_Mean(adc_simple* xADC);
void vAddSample(adc_simple* xADC, uint16_t _value);
bool bAdd_value(adc_simple* xADC, uint16_t _value);
uint8_t u8Shift_Value(shift_reg* xShift, uint8_t _value);
void vShiftInit(shift_reg* xReg);


//For hardware part of device
void adc_select_channel(uint8_t ch_num);
uint8_t adc_get_channel(void);
uint16_t adc_get_result(void);

//ADC conversion 

uint8_t u8GetConversionValue(uint16_t _adc);

bool bGetMeanValue(uint32_t* _sum, uint16_t* _mean, uint8_t* _counter, uint16_t _adc_val);

uint16_t _from_voltage(float value);

#endif