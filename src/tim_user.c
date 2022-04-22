#include "tim_user.h"

void set_pwm(enum ePWM channel, uint8_t fill)
{
	if (pwm_1 == channel)
	{
		timer_channel_output_pulse_value_config(TIMER2, TIMER_CH_2, 10 * fill - 1U);
	}
	else if (pwm_2 == channel)
	{
		timer_channel_output_pulse_value_config(TIMER2, TIMER_CH_3, 10 * fill - 1U);
	}
	else
	{
		__NOP();
	}
}

void enable_pwm(enum ePWM channel)
{
	if (pwm_1 == channel)
	{
		TIMER_CHCTL2(TIMER2) |= TIMER_CHCTL2_CH2EN;
	}
	else if (pwm_2 == channel)
	{
		TIMER_CHCTL2(TIMER2) |= TIMER_CHCTL2_CH3EN;
	}
	else
	{
		__NOP();
	}
}
void disable_pwm(enum ePWM channel)
{
	if (pwm_1 == channel)
	{
		TIMER_CHCTL2(TIMER2) &= ~TIMER_CHCTL2_CH2EN;
	}
	else if (pwm_2 == channel)
	{
		TIMER_CHCTL2(TIMER2) &= ~TIMER_CHCTL2_CH3EN;
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
		if ((TIMER_CHCTL2(TIMER2) & TIMER_CHCTL2_CH2EN) == TIMER_CHCTL2_CH2EN)
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