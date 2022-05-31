#include "pwm.h"

void add_sample_pwm(struct pwm *xPWM)
{
	if (xPWM->index < 2000U)
	{
		xPWM->index++;
		((GPIO_ISTAT(SAMPLE_PORT) & SAMPLE_PIN) == SAMPLE_PIN)?(xPWM->ones++):(__NOP());
	}
	(xPWM->index == 2000U) ? (xPWM->is_measured = true) : (xPWM->is_measured = false);
}

uint8_t get_pwm_fill(struct pwm *xPWM)
{
	return (xPWM->ones * 100U) / xPWM->index;
}

void get_invert(bool _state)
{
	_state ? (GPIO_OCTL(INV_PORT) &= ~INV_PIN) : (GPIO_OCTL(INV_PORT) |= INV_PIN);
}

void get_clear_pwm_measure(struct pwm *xPWM)
{
	xPWM->index = 0x00U;
	xPWM->ones = 0x00U;
	xPWM->is_measured = false;
}

bool check_pwm_valid(struct pwm_validator *xPWM_VAL, uint16_t _cap_time)
{
	if (_cap_time < 10U)
	{
		if (xPWM_VAL->valid_index < 0xFF)
		{
			xPWM_VAL->valid_index++;
		}
	}
	else
	{
		if (xPWM_VAL->valid_index > 0)
		{
			xPWM_VAL->valid_index--;
		}
	}

	(xPWM_VAL->valid_index > 0x80U) ? (xPWM_VAL->is_valid = true) : (xPWM_VAL->is_valid = false);

	return xPWM_VAL->is_valid;
}

void get_pwm_error_action(void)
{
	//  Stop generation on pwm_2
	if (pwm_def_task_handle != NULL)
	{
		vTaskSuspend(pwm_def_task_handle);
	}
	disable_pwm(pwm_1);
	// Set default value on pwm_1
	set_pwm(10U, pwm_2);
}

void get_pwm_action(uint8_t _pwm_fill)
{
	if (_pwm_fill < 11U)
	{
		get_pwm_error_action();
	}
	else
	{
		// resume pwm_2 task
		if (pwm_def_task_handle == NULL)
		{
			xTaskCreate(pwm_def_task, "pwm_def_task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &pwm_def_task_handle);
		}
		else
		{
			vTaskResume(pwm_def_task_handle);
		}
		if(SysTime > 10U){
			enable_pwm(pwm_2);
		}
		enable_pwm(pwm_1);
		if (_pwm_fill < 41U)
		{
			set_pwm(pwm_1, 30U);
		}
		else if (_pwm_fill < 81U)
		{
			set_pwm(pwm_1, 50U);
		}
		else
		{
			set_pwm(pwm_1, 80U);
		}
	}
}