#include "main.h"

void _clk_init(void)
{
    RCU_APB2EN |= RCU_APB2EN_PAEN | RCU_APB2EN_PBEN | RCU_APB2EN_PCEN;
    RCU_APB2EN |= RCU_APB2EN_USART0EN;
		RCU_APB2EN |= RCU_APB1EN_TIMER1EN;
}

void _gpio_init(void)
{
    gpio_init(GPIOC, GPIO_MODE_OUT_OD, GPIO_OSPEED_10MHZ, GPIO_PIN_13); // It's led for indicate activity
    gpio_init(GPIOB, GPIO_MODE_IPD, GPIO_OSPEED_2MHZ, GPIO_PIN_12);     // input signal
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_11); // signal response
}

void _usart_init(void)
{
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

void _tim1_init(void)
{
		timer_deinit(TIMER1);
		timer_parameter_struct tim1; 
		tim1.prescaler = 10800;
		tim1.alignedmode = TIMER_COUNTER_EDGE;
		tim1.counterdirection = TIMER_COUNTER_UP;
		tim1.period = 1000;
		timer_init(TIMER1, &tim1);
}