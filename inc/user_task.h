#ifndef _user_task_h_
#define _user_task_h_

#include "main.h"

// Tasks
void get_activity(void *pvParameters);
void capture_signal(void *pvParameters);
void signal_generation(void *pvParameters);
void pwm_generation(void *pvParameters);
void ad8400_task(void *pvParameters);
void main_task(void *pvParameters);

extern TaskHandle_t get_activity_handle;
extern TaskHandle_t capture_signal_handle;
extern TaskHandle_t signal_generation_handle;
extern TaskHandle_t pwm_generation_handle;
extern TaskHandle_t ad8400_task_handle;
extern TaskHandle_t main_task_handle;

//queues for tasks
extern QueueHandle_t pwm_value;



#endif