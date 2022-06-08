#include "tim_user.h"

void set_pwm(enum ePWM channel, uint8_t fill)
{
	if (pwm_2 == channel)
	{
		TIMER_CH0CV(TIMER1) = 10 * fill - 1U;
		//timer_channel_output_pulse_value_config(TIMER1, TIMER_CH_0, 10 * fill - 1U);
	}
	else if (pwm_2 == channel)
	{
		TIMER_CH3CV(TIMER2) = 10 * fill - 1U;
		//timer_channel_output_pulse_value_config(TIMER2, TIMER_CH_3, 10 * fill - 1U);
	}
	else
	{
		__NOP();
	}
}

void enable_pwm(enum ePWM channel)
{
	if (pwm_2 == channel)
	{
		TIMER_CTL0(TIMER1)  |= TIMER_CTL0_CEN;
		//TIMER_CHCTL2(TIMER1) |= TIMER_CHCTL2_CH0EN;
	}
	else if (pwm_1 == channel)
	{
		TIMER_CTL0(TIMER2) |= TIMER_CTL0_CEN;
		//TIMER_CHCTL2(TIMER2) |= TIMER_CHCTL2_CH3EN;
	}
	else
	{
		__NOP();
	}
}
void disable_pwm(enum ePWM channel)
{
	if (pwm_2 == channel)
	{
		TIMER_CTL0(TIMER1)  &= ~TIMER_CTL0_CEN;
		//TIMER_CHCTL2(TIMER1) &= ~TIMER_CHCTL2_CH0EN;
	}
	else if (pwm_1 == channel)
	{
		//TIMER_CHCTL2(TIMER2) &= ~TIMER_CHCTL2_CH3EN;
		TIMER_CTL0(TIMER2) &= ~TIMER_CTL0_CEN;
	}
	else
	{
		__NOP();
	}
}

bool get_pwm_state(enum ePWM channel)
{
	bool state = false;
	if (pwm_1 == channel)
	{
		if ((TIMER_CHCTL2(TIMER1) & TIMER_CHCTL2_CH0EN) == TIMER_CHCTL2_CH0EN)
		{
			state = true;
		}
	}
	else if (pwm_2 == channel)
	{
		if ((TIMER_CHCTL2(TIMER2) & TIMER_CHCTL2_CH3EN) == TIMER_CHCTL2_CH3EN)
		{
			state = true;
		}
	}
	else
	{
		__NOP();
	}
	return state;
}