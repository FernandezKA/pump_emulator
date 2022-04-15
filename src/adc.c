#include "adc.h"

void adc_select_channel(uint8_t ch_num)
{
	 if(ch_num == 0x02){
			ADC_CTL0(ADC0)&=~((uint32_t) 0x1F);//Reset only field for channel select
			ADC_CTL0(ADC0) |= ch_num;
	 }
	 else if(ch_num == 0x03){
		  ADC_CTL0(ADC0)&=~((uint32_t) 0x1F);//Reset only field for channel select
			ADC_CTL0(ADC0) |= ch_num;
	 }
	 else{
		 __NOP();//Mistake (channel not avaliable)
	 }
}

uint8_t adc_get_channel(void){
	return (ADC_CTL0(ADC0) & 0x1F);
}

uint16_t adc_get_result(void){
	 return (uint16_t) (ADC_RDATA(ADC0) & 0xFFFF);
}
