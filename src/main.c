#include "main.h"

QueueHandle_t pwm_value;
QueueHandle_t cap_signal;
QueueHandle_t sig_gen_flag;
QueueHandle_t adc_0_val;
QueueHandle_t adc_1_val;
QueueHandle_t uart_info;

bool isCapture = false;

static inline void SysInit(void);

uint32_t SysTime = 0x00U;
bool start_req = false, bus_error = false, pwm_detect = false;
int main()
{
	SysInit();
	pwm_value = xQueueCreate(1, sizeof(uint8_t));
	cap_signal = xQueueCreate(1, sizeof(struct pulse));
	sig_gen_flag = xQueueCreate(1, sizeof(bool));
	adc_0_val = xQueueCreate(5, sizeof(uint8_t));
	adc_1_val = xQueueCreate(5, sizeof(uint8_t));
	uart_info = xQueueCreate(0x40, sizeof(uint8_t));

	if (pdPASS != xTaskCreate(main_task, "main_task", configMINIMAL_SECURE_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, &main_task_handle))
		ERROR_HANDLER();
	if (pdPASS != xTaskCreate(sample_task, "sample_task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &sample_task_handle))
		ERROR_HANDLER();
	if (pdPASS != xTaskCreate(adc_task, "adc_task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &adc_task_handle))
		ERROR_HANDLER();
	if (pdPASS != xTaskCreate(ad8400_0_task, "ad8400_0 task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &ad8400_0_task_handle))
		ERROR_HANDLER();
	if (pdPASS != xTaskCreate(ad8400_1_task, "ad8400_1 task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &ad8400_1_task_handle))
		ERROR_HANDLER();
	if (pdPASS != xTaskCreate(response_task, "responce_tesk", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, &response_task_handle))
		ERROR_HANDLER();
	//	if (pdPASS != xTaskCreate(uart_info_task, "uart info task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &uart_info_task_handle))
	//		ERROR_HANDLER();

	vTaskSuspend(response_task_handle);
	vTaskStartScheduler();
	for (;;)
		;
	return 0;
}

static inline void SysInit(void)
{
	_clk_init();
	_gpio_init();
	_usart_init();
	_tim1_init();
	_tim2_init();
	_tim3_init();
	_adc_init();
}

void ERROR_HANDLER(void)
{
	for (;;)
	{
		__NOP();
	}
}