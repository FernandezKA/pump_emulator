#ifndef _pwm_h_
#define _pwm_h_
#include "main.h"

struct pwm{
	uint16_t index;
	uint16_t ones;
	bool is_measured;
};

struct pwm_validator{
	uint8_t valid_index; 
	bool is_valid;
};

extern struct pwm PWM;
extern struct pwm_validator PWM_VALIDATOR;

void add_sample_pwm(struct pwm* xPWM);

uint8_t get_pwm_fill(struct pwm* xPWM);

void get_clear_pwm_measure(struct pwm* xPWM);

void get_invert(bool _state);

bool check_pwm_valid(struct pwm_validator* xPWM_VAL, uint16_t _cap_time);

void get_pwm_error_action(void);

void get_pwm_action(uint8_t _pwm_fill);

#endif