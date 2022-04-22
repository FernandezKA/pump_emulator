#ifndef _tim_user_h_
#define _tim_user_h_
#include "main.h"

enum ePWM{
    pwm_1 = 0x00, 
    pwm_2 = 0x01
};


void set_pwm(enum ePWM channel, uint8_t fill);
void enable_pwm(enum ePWM channel);
void disable_pwm(enum ePWM channel);


#endif