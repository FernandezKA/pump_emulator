#ifndef _main_h_
#define _main_h_
//#include <stdbool.h>

#define true TRUE
#define false !true
#include <stdlib.h>

#include "gd32f10x.h"
#include "gd32f10x_usart.h"
#include "gd32f10x_timer.h"
#include "gd32f10x_adc.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "user_task.h"
#include "tim_user.h"
#include "adc.h"
#include "spi.h"
#include "usart.h"

void ERROR_HANDLER(void);

#define SAMPLE_PIN (1 << 8)
#define SAMPLE_PORT GPIOA

#define LED_RUN_PORT GPIOC
#define RUN_LED (1 << 13)
#define LED_ACT_PORT GPIOB
#define LED_START_PORT GPIOB
#define ACT_LED	(1U<<6)
#define START_LED (1U<<7)

#define RESPONSE_PORT GPIOA
#define RESPONSE_PIN (1 << 10)

#define INV_PORT GPIOB
#define INV_PIN (1<<0)


void _clk_init(void);
void _gpio_init(void);
void _usart_init(void);
void _tim0_init(void);
void _tim1_init(void);
void _tim2_init(void);
void _tim3_init(void);
void _adc_init(void);
void _nvic_enable(void);

extern uint32_t SysTime;

extern bool isCapture;

#endif