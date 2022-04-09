#ifndef _main_h_
#define _main_h_
#include <stdbool.h>

#include "gd32f10x.h"
#include "gd32f10x_usart.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "user_task.h"

void ERROR_HANDLER(void);

#define SAMPLE_PIN (1<<12)
#define SAMPLE_PORT GPIOB

#define LED_PORT GPIOC
#define LED_PIN (1<<13)

#define RESPONSE_PORT GPIOB
#define RESPONSE_PIN  (1<<11)

void _clk_init(void);
void _gpio_init(void);
void _usart_init(void);
void _tim1_init(void);

#endif