#include "main.h"

QueueHandle_t pwm_value;
QueueHandle_t cap_signal;
QueueHandle_t sig_gen_flag;

static inline void SysInit(void);

int main()
{
	SysInit();
	pwm_value = xQueueCreate(1, sizeof(uint8_t));
	cap_signal = xQueueCreate(1, sizeof(struct pulse));
	sig_gen_flag = xQueueCreate(1, sizeof(bool));
	
	if (pdPASS != xTaskCreate(main_task, "main_task", configMINIMAL_SECURE_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &main_task_handle))
		ERROR_HANDLER();
	if (pdPASS != xTaskCreate(sample_task, "sample_task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &sample_task_handle))
		ERROR_HANDLER();
	if(pdPASS != xTaskCreate(response_task, "responce_task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &response_task_handle))
		ERROR_HANDLER();
	if(pdPASS != xTaskCreate(send_info_task, "send_info_task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &send_info_task_handle))
		ERROR_HANDLER();
	vTaskSuspend(response_task_handle);
	vTaskStartScheduler();
	for (;;)
	{
		
	}
	return 0;
}

static inline void SysInit(void)
{
	_clk_init();
	_gpio_init();
	_usart_init();
	_tim1_init();
}

void ERROR_HANDLER(void)
{
	for (;;)
	{
		__NOP();
	}
}