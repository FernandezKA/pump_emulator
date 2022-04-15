#ifndef _adc_h_
#define _adc_h_
#include "main.h"

void adc_select_channel(uint8_t ch_num);

uint8_t adc_get_channel(void);

uint16_t adc_get_result(void);

#endif