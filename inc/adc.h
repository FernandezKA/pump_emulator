#ifndef _adc_h_
#define _adc_h_
#include "main.h"

struct adc_buf{
	uint8_t data[120U];
	uint8_t pMeasure;
};

void vADC_Buf_Clean(struct adc_buf* xADC);

uint8_t u8ADC_Get_Mean(struct adc_buf* xADC);



void adc_select_channel(uint8_t ch_num);

uint8_t adc_get_channel(void);

uint16_t adc_get_result(void);

#endif