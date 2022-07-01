#include "main.h"

QueueHandle_t pwm_value;
QueueHandle_t cap_signal;
QueueHandle_t sig_gen_flag;
QueueHandle_t adc_0;
QueueHandle_t adc_1_val;

bool isCapture = false;

static inline void SysInit(void);
static inline void irq_enable(void);

uint32_t SysTime = 0x00U;
bool start_req = false, bus_error = false, pwm_detect = false;
uint8_t measured_pwm = 0x00U;
//User struct for periph 
struct pwm PWM;
struct therm_res therm_int; 
uint8_t conversion_result;
float measured_ad8400 = 0x00U;
uint8_t inc_val_ad8400 = 0x00U;
int main()
 {
	SysInit();
	cap_signal = xQueueCreate(10, sizeof(struct pulse));
	adc_0 = xQueueCreate(10, sizeof(uint16_t));
	reset_therm_struct(&therm_int);
	if (pdPASS != xTaskCreate(main_task, "main_task", configMINIMAL_SECURE_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &main_task_handle))
		ERROR_HANDLER();
	if (pdPASS != xTaskCreate(sample_task, "sample_task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &sample_task_handle))
		ERROR_HANDLER();
	if (pdPASS != xTaskCreate(adc_task, "adc_task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &adc_task_handle))
		ERROR_HANDLER();
	if (pdPASS != xTaskCreate(ad8400_0_task, "ad8400_0 task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &ad8400_0_task_handle))
		ERROR_HANDLER();
	if (pdPASS != xTaskCreate(ad8400_1_task, "ad8400_1 task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &ad8400_1_task_handle))
		ERROR_HANDLER();
	if (pdPASS != xTaskCreate(response_task, "responce_tesk", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &response_task_handle))
		ERROR_HANDLER();
	if (pdPASS != xTaskCreate(uart_info_task, "uart info task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &uart_info_task_handle))
		ERROR_HANDLER();
	vTaskSuspend(response_task_handle);
	irq_enable();
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
	_tim0_init();//INPUT CAPTURE REQUEST
	_tim1_init();//PWM generation
	_tim2_init();//PWM GENERATION
	_adc_init();
}

void ERROR_HANDLER(void)
{
	for (;;)
	{
		__NOP();
	}
}

static inline void irq_enable(void){
	nvic_irq_enable(TIMER0_Channel_IRQn, 9, 9);
	NVIC_SetPriorityGrouping(0);
}