#include "main.h"

QueueHandle_t pwm_value;
QueueHandle_t cap_signal;
static inline void SysInit(void);

int main()
{
	SysInit();
	pwm_value= xQueueCreate(1, sizeof(uint8_t));
	cap_signal = xQueueCreate(6, sizeof(uint8_t));
	if (pdPASS != xTaskCreate(main_task, "main_task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &main_task_handle)) ERROR_HANDLER();
	if(pdPASS != xTaskCreate(get_activity, "get_activity", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1,&get_activity_handle)) ERROR_HANDLER();
	if(pdPASS != xTaskCreate(capture_signal, "capture_signal", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1,&capture_signal_handle)) ERROR_HANDLER();
	vTaskStartScheduler();
	for (;;)
	{
	}
	return 0;
}

static inline void SysInit(void)
{
	// Init gpios
	RCU_APB2EN |= RCU_APB2EN_PAEN | RCU_APB2EN_PBEN | RCU_APB2EN_PCEN;
	gpio_init(GPIOC, GPIO_MODE_OUT_OD, GPIO_OSPEED_10MHZ, GPIO_PIN_13); // It's led for indicate activity
	gpio_init(GPIOB, GPIO_MODE_IPD, GPIO_OSPEED_2MHZ, GPIO_PIN_12);		// input signal
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_13);
	// Init usart for communication with PC
	RCU_APB2EN |= RCU_APB2EN_USART0EN;
	usart_deinit(USART0);
	usart_baudrate_set(USART0, 115200UL);
	usart_parity_config(USART0, USART_PM_NONE);
	usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
	usart_receive_config(USART0, USART_RECEIVE_ENABLE);
	usart_interrupt_enable(USART0, USART_INT_RBNE);
	gpio_afio_deinit();
	gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
	gpio_init(GPIOA, GPIO_MODE_IPD, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
	usart_enable(USART0);
}

void ERROR_HANDLER(void){
	for(;;){
		__NOP();
	}
}