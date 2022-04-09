#include "user_task.h"


TaskHandle_t ad8400_task_handle = NULL;
TaskHandle_t main_task_handle = NULL;
TaskHandle_t sample_task_handle = NULL;

void ad8400_task(void *pvParameters)
{
	for (;;)
	{
		
	}
}
void main_task(void *pvParameters)
{
	struct pulse timeArray[0x0AU];
	uint8_t indexArray = 0x00U;
	
	for (;;)
	{
		 if(pdPASS == xQueueReceive(cap_signal, &timeArray[indexArray], 0)){
				++indexArray;
			 if(indexArray == 0x06U){      //6th samples received
				 //Check for valid 
			 }
		 }
	}
}

void sample_task(void *pvParameters){
		bool last_state, curr_state = false;
		uint16_t time_val = 0x00U;
		uint32_t sysTick = 0x00U;
		struct pulse curr_pulse;
	 for(;;){
		//Upd variables
			last_state = curr_state;
			++sysTick;
		//Sample input signal
		if ((GPIO_ISTAT(SAMPLE_PORT) & SAMPLE_PIN) == SAMPLE_PIN){
			curr_state = true;
		}
		else{
			 curr_state = false;
		}
		if(last_state == curr_state){
			time_val++;
		}
		else{
			 xQueueSendToBack(cap_signal, &((struct pulse){.state = last_state, .time = time_val}), 0);
			 time_val = 0x00U;
		}
		//Signal generation 
		
		//LED activity
		if((sysTick%500) == 0U){
			GPIO_OCTL(LED_PORT)^=LED_PIN;
		}
		vTaskDelay(pdMS_TO_TICKS(1));
	 }
}
