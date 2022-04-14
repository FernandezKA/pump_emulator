#include "main.h"

void _clk_init(void)
{
    RCU_APB2EN |= RCU_APB2EN_PAEN | RCU_APB2EN_PBEN | RCU_APB2EN_PCEN;
    RCU_APB2EN |= RCU_APB2EN_USART0EN;
    RCU_APB2EN |= RCU_APB1EN_TIMER1EN;
    RCU_APB1EN |= RCU_APB1EN_TIMER2EN;
		RCU_APB1EN |= RCU_APB1EN_TIMER3EN;
}

void _gpio_init(void)
{
    gpio_init(GPIOC, GPIO_MODE_OUT_OD, GPIO_OSPEED_10MHZ, GPIO_PIN_13);      // It's led for indicate activity
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_0); // input signal
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_11);      // signal response
		gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0); //PWM_11, TIM3_CH3
		gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1); //PWM_12, TIM3_CH4
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

void _tim2_init(void)
{
	
	rcu_periph_clock_enable(RCU_AF);
		static timer_oc_parameter_struct timer_ocintpara;
    static timer_parameter_struct timer_initpara;
	
		timer_deinit(TIMER2);

    /* TIMER1 configuration */
    timer_initpara.prescaler         = 1070;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 1000;
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER2,&timer_initpara);
	
	    /* CH1,CH2 and CH3 configuration in PWM mode1 */
    timer_ocintpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;
    timer_ocintpara.outputstate  = TIMER_CCX_ENABLE;
    timer_ocintpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
    timer_ocintpara.outputnstate = TIMER_CCXN_DISABLE;
    timer_ocintpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
    timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;
		
		timer_channel_output_config(TIMER2,TIMER_CH_1,&timer_ocintpara);
    timer_channel_output_config(TIMER2,TIMER_CH_2,&timer_ocintpara);
    timer_channel_output_config(TIMER2,TIMER_CH_3,&timer_ocintpara);
		
		    /* CH1 configuration in PWM mode1,duty cycle 25% */
    timer_channel_output_pulse_value_config(TIMER2,TIMER_CH_1,399);
    timer_channel_output_mode_config(TIMER2,TIMER_CH_1,TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(TIMER2,TIMER_CH_1,TIMER_OC_SHADOW_DISABLE);

    /* CH2 configuration in PWM mode1,duty cycle 50% */
    timer_channel_output_pulse_value_config(TIMER2,TIMER_CH_2,799);
    timer_channel_output_mode_config(TIMER2,TIMER_CH_2,TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(TIMER2,TIMER_CH_2,TIMER_OC_SHADOW_DISABLE);

    /* CH3 configuration in PWM mode1,duty cycle 75% */
    timer_channel_output_pulse_value_config(TIMER2,TIMER_CH_3,899);
    timer_channel_output_mode_config(TIMER2,TIMER_CH_3,TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(TIMER2,TIMER_CH_3,TIMER_OC_SHADOW_DISABLE);


	    /* auto-reload preload enable */
    timer_auto_reload_shadow_enable(TIMER2);
    /* auto-reload preload enable */
    timer_enable(TIMER2);
}

void _tim3_init(void)
{
	TIMER_CAR(TIMER3) = 1000U;
	TIMER_PSC(TIMER3) = 1079U;
	TIMER_CH3CV(TIMER3) = 499U;
	TIMER_CHCTL2(TIMER3) |= TIMER_CHCTL2_CH3EN;
	TIMER_CTL0(TIMER3) = TIMER_CTL0_CEN;
}

void _nvic_enable(void)
{
	nvic_irq_enable(USART0_IRQn, 2, 2); // For UART0_PC
	nvic_irq_enable(TIMER1_IRQn, 1, 1); //request input capture 
	nvic_irq_enable(TIMER2_IRQn, 2, 1); //Detect filling pwm signal 
}
