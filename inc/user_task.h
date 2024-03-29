#ifndef _user_task_h_
#define _user_task_h_

#include "main.h"
#include "usart.h"
#include "pwm.h"

enum work_mode
{
	pwm_input,
	start_input,
	stop_input,
	undef
};

struct pulse
{
	bool state;
	uint16_t time;
};

// Tasks
void sample_task(void *pvParameters);
void ad8400_0_task(void *pvParameters);
void ad8400_1_task(void *pvParameters);
void main_task(void *pvParameters);
void response_task(void *pvParameters);
void adc_task(void *pvParameters);
void pwm_def_task(void *pvParameters);
void uart_info_task(void *pvParameters);

extern TaskHandle_t ad8400_0_task_handle;
extern TaskHandle_t ad8400_1_task_handle;
extern TaskHandle_t main_task_handle;
extern TaskHandle_t sample_task_handle;
extern TaskHandle_t response_task_handle;
extern TaskHandle_t adc_task_handle;
extern TaskHandle_t pwm_def_task_handle;
extern TaskHandle_t uart_info_task_handle;

// queues for tasks
extern QueueHandle_t cap_signal;
extern QueueHandle_t adc_0;

void reset_pwm_value(void);
void reset_flags(void);


#endif