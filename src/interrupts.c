#include "main.h"
#include "interrupts.h"
#include "user_task.h"
#include "pwm.h"

void TIMER0_Channel_IRQHandler(void)
{
	static volatile uint16_t cap_time = 0x00U;
	static volatile struct pulse cap_pulse;
	if (SET == timer_interrupt_flag_get(TIMER0, TIMER_INT_FLAG_CH0))
	{ // rising edge
		timer_interrupt_flag_clear(TIMER0, TIMER_INT_FLAG_CH0);
		cap_pulse.state = FALSE;
		cap_pulse.time = TIMER_CH0CV(TIMER0);
		TIMER_CNT(TIMER0) = 0x00U;
	}
	else if (SET == timer_interrupt_flag_get(TIMER0, TIMER_INT_FLAG_CH1))
	{//falling edge
		timer_interrupt_flag_clear(TIMER0, TIMER_INT_FLAG_CH1);
		cap_pulse.state = TRUE;
		cap_pulse.time = TIMER_CH1CV(TIMER0);
		TIMER_CNT(TIMER0) = 0x00U;
	}
	else{
		
	}
	
	//Clear all of flags
//	TIMER_INTF(TIMER0) = 0x00U;
	
	if(cap_pulse.time < 100U){
		get_invert(!cap_pulse.state);
	}
	else{
		get_invert(TRUE);
	}
	//xQueueReset(cap_signal);
	xQueueSendToBackFromISR(cap_signal, (void*) &cap_pulse, NULL);
	return;
}

void USART0_IRQHandler(void)
{
	return;
}
