#ifndef _pwm_h_
#define _pwm_h_
#include "main.h"
//Stis struct used for measuring pwm filling
struct pwm{
	uint16_t index;
	uint16_t ones;
	bool is_measured;
	bool is_valid;
};


extern struct pwm PWM;

//For writing new samples into structure, value captured from gpio
void add_sample_pwm(struct pwm* xPWM);
//Return pwm filling in percents
uint8_t get_pwm_fill(struct pwm* xPWM);
//Reset all fields pwm data structure
void get_clear_pwm_measure(struct pwm* xPWM);
//Invert signal on from request line to invert line
void get_invert(bool _state);
//Used for invalid parsing situation 
void get_pwm_error_action(void);
//Used, when pwm is detected, define values on pwm_1 && pwm_2 lines
void get_pwm_action(uint8_t _pwm_fill);

#endif