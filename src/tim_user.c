#include "tim_user.h"

void set_pwm(uint8_t channel, uint8_t fill)
{
	if (0x02U == channel)
	{
		timer_channel_output_pulse_value_config(TIMER2, TIMER_CH_2, 10 * fill - 1U);
	}
	else if (0x03U == channel)
	{
		timer_channel_output_pulse_value_config(TIMER2, TIMER_CH_3, 10 * fill - 1U);
	}
	else
	{
		__NOP();
	}
}

void enable_pwm(uint8_t channel)
{
	if (0x02U == channel)
	{
		TIMER_CHCTL2(TIMER2) |= TIMER_CHCTL2_CH2EN;
	}
	else if (0x03U == channel)
	{
		TIMER_CHCTL2(TIMER2) |= TIMER_CHCTL2_CH3EN;
	}
	else
	{
		__NOP();
	}
}
void disable_pwm(uint8_t channel)
{
	if (0x02U == channel)
	{
		TIMER_CHCTL2(TIMER2) &= ~TIMER_CHCTL2_CH2EN;
	}
	else if (0x03U == channel)
	{
		TIMER_CHCTL2(TIMER2) &= ~TIMER_CHCTL2_CH3EN;
	}
	else
	{
		__NOP();
	}
}

bool get_pwm_state(uint8_t channel)
{
	bool state = false;
	if (0x02U == channel)
	{
		if ((TIMER_CHCTL2(TIMER2) & TIMER_CHCTL2_CH2EN) == TIMER_CHCTL2_CH2EN)
		{
			state = true;
		}
	}
	else if (0x03U == channel)
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