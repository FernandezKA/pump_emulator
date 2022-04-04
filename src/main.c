#include "gd32f10x.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

static inline void SysInit(void);
void led_activity(void *pvParameters);

int main(){
	SysInit();
	xTaskCreate( led_activity, "Check", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
	vTaskStartScheduler();
	for(;;){}
		return 0;
}


void led_activity(void *pvParameters){
		for(;;){
			vTaskDelay(300);
			GPIO_OCTL(GPIOC)^=(1<<13);
			 __NOP();
		}
}

 static inline void SysInit(void){
		RCU_APB2EN |= RCU_APB2EN_PCEN | RCU_APB2EN_PAEN;
	 gpio_init(GPIOC, GPIO_MODE_OUT_OD, GPIO_OSPEED_10MHZ, GPIO_PIN_13); // It's led for indicate activity
 }
 