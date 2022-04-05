#include "gd32f10x.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

static inline void SysInit(void);
void led_activity(void *pvParameters);
void send_status(void *pvArguments);

int main(){
	SysInit();
	xTaskCreate(led_activity, "led_activity", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
	xTaskCreate(send_status, "Get_status", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	vTaskStartScheduler();
	for(;;){}
		return 0;
}


void led_activity(void *pvParameters){
		for(;;){
			vTaskDelay(pdMS_TO_TICKS(1000));
			GPIO_OCTL(GPIOC)^=(1<<13);
		}
}

void send_status(void *pvArguments){
	for(;;){
	 vTaskDelay(1000);
	}
}

 static inline void SysInit(void){
		RCU_APB2EN |= RCU_APB2EN_PCEN | RCU_APB2EN_PAEN;
	 gpio_init(GPIOC, GPIO_MODE_OUT_OD, GPIO_OSPEED_10MHZ, GPIO_PIN_13); // It's led for indicate activity
 }
 