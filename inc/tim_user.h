#ifndef _tim_user_h_
#define _tim_user_h_
#include "main.h"

void set_pwm(uint8_t channel, uint8_t fill);
void enable_pwm(uint8_t channel);
void disable_pwm(uint8_t channel);

#endif