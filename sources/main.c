#include "main.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

static inline void SysInit(void);

void led_activity(void *pvParameters){
	 vTaskDelay(100);
	 __NOP();
}

int main(){
	
	if(pdTRUE != xTaskCreate(led_activity,"Blinker", 	configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL)) {
		__NOP();
	}
	 vTaskStartScheduler();
}

static inline void SysInit(void){

}