#include "main.h"
#include "interrupts.h"

void TIMER0_Channel_IRQHandler(void)
{
	if (SET == timer_interrupt_flag_get(TIMER0, TIMER_INT_CH0))
	{ // Falling edge
		timer_interrupt_flag_clear(TIMER0, TIMER_INT_CH0);
	}
	else if (SET == timer_interrupt_flag_get(TIMER0, TIMER_INT_CH1))
	{
		timer_interrupt_flag_clear(TIMER0, TIMER_INT_CH1);
	}
}

void USART0_IRQHandler(void)
{
	return;
}
