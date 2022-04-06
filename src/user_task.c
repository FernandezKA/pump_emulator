#include "user_task.h"

TaskHandle_t get_activity_handle = NULL;
TaskHandle_t capture_signal_handle = NULL;
TaskHandle_t signal_generation_handle = NULL;
TaskHandle_t pwm_generation_handle = NULL;
TaskHandle_t ad8400_task_ghandle = NULL;
TaskHandle_t main_task_handle = NULL;

void get_activity(void *pvParameters)
{
    for (;;)
    {
			vTaskDelay(500);
			GPIO_OCTL(GPIOC)^=(1<<13);
			vTaskResume(signal_generation_handle);
    }
}
void capture_signal(void *pvParameters)
{
	uint8_t cap_value = 0;
	_Bool last_state, curr_state = 0x00U;
    for (;;)
    {
			last_state = curr_state;
			curr_state = ((GPIO_ISTAT(GPIOB) & (1<<12)) == (1<<12));
			if(curr_state != last_state){
				xQueueSendToBack(cap_signal, (void*) &cap_value, 0);
				cap_value = 0x00U;
			}
			else{
				++cap_value;
			}
			vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void signal_generation(void *pvParameters)
{
	const uint8_t response_array[] = {180, 100, 100, 10, 190, 100};
	uint8_t response_index = 0x00U;
    for (;;)
    {
			if (0x00U == (response_index%2)){
				 GPIO_OCTL(GPIOB)|=(1<<13);
			}
			else{
				 GPIO_OCTL(GPIOB)&=~(1<<13);
			}
			if (sizeof(response_array) > response_index){
				 ++response_index;
			}
			else{
				 response_index = 0x00U;
				 vTaskSuspend(signal_generation_handle);
			}
			vTaskDelay(pdMS_TO_TICKS(response_array[response_index]));
    }
}

void pwm_generation(void *pvParameters)
{
		uint8_t pwm_fill;
		uint8_t pwm_low, pwm_high;
    for (;;)
    {
			if(pdPASS == xQueueReceive(pwm_value, &pwm_fill, 0)){
				pwm_high = pwm_fill/10;
				pwm_low = 1- - pwm_high;
			}
			GPIO_OCTL(GPIOB)|=(1<<14);
			vTaskDelay(pdMS_TO_TICKS(pwm_high));
			GPIO_OCTL(GPIOB)&=~(1<<14);
			vTaskDelay(pdMS_TO_TICKS(pwm_low));
    }
}
void ad8400_task(void *pvParameters)
{
    for (;;)
    {
    }
}
void main_task(void *pvParameters)
{
		uint8_t u8Fill = 20;
		if(pdPASS != xTaskCreate(signal_generation, "signal_generation", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1,&signal_generation_handle)) ERROR_HANDLER();
		//if(pdPASS != xTaskCreate(pwm_generation, "pwm_generation", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1,&pwm_generation_handle)) ERROR_HANDLER();
		//xQueueSend(pwm_value, &u8Fill, 0);
    for (;;)
    {
			
    }
}
