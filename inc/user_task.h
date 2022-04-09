#ifndef _user_task_h_
#define _user_task_h_

#include "main.h"

// Tasks
void sample_task(void *pvParameters);
void ad8400_task(void *pvParameters);
void main_task(void *pvParameters);

extern TaskHandle_t ad8400_task_handle;
extern TaskHandle_t main_task_handle;
extern TaskHandle_t sample_task_handle;

//queues for tasks
extern QueueHandle_t pwm_value;
extern QueueHandle_t cap_signal;

struct pulse{
	bool state;
	uint16_t time;
};

#endif