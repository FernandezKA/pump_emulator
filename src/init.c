#include "main.h"

void _clk_init(void)
{
  RCU_APB2EN |= RCU_APB2EN_PAEN | RCU_APB2EN_PBEN | RCU_APB2EN_PCEN;
  RCU_APB2EN |= RCU_APB2EN_USART0EN;
  RCU_APB2EN |= RCU_APB2EN_TIMER0EN | RCU_APB1EN_TIMER1EN;
  RCU_APB2EN |= RCU_APB2EN_ADC0EN;

  RCU_AHBEN |= RCU_AHBEN_DMA0EN | RCU_AHBEN_DMA1EN;
  RCU_APB1EN |= RCU_APB1EN_TIMER1EN;
  RCU_APB1EN |= RCU_APB1EN_TIMER2EN;
  RCU_APB1EN |= RCU_APB1EN_TIMER3EN;

  rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV8);
}

void _gpio_init(void)
{
  gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_13); // It's led for indicate activity
  gpio_init(GPIOA, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_0);     // input signal
  gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10); // signal response
  gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0);  // inverted state for input caapture signal
  gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1);   // PWM_12, TIM3_CH3
  gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0);   // PWM_OUT, PA0
  gpio_init(GPIOA, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_8); //REQUEST INPUT CAPTURE

  gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_2); // ADC0_CH2
  gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_3); // ADC0_CH3

  gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_2);  // NSS_0
  gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_10); // NSS_1
  gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_7);  // SDI
  gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_5);  // SCK

  gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_7); // D8
  gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_6); // D9
}

void _usart_init(void)
{
  usart_deinit(USART0);
  usart_baudrate_set(USART0, 115200UL);
  usart_parity_config(USART0, USART_PM_NONE);
  usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
  //  usart_receive_config(USART0, USART_RECEIVE_ENABLE);
  //  usart_interrupt_enable(USART0, USART_INT_RBNE);
  gpio_afio_deinit();
  gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
  //  //gpio_init(GPIOA, GPIO_MODE_IPD, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
  usart_enable(USART0);
}

// This timer used for input capture signal from request line
void _tim0_init(void)
{
  // CONFIGURE INPUT FILTER
  TIMER_PSC(TIMER0) = 10799U; //Fsampling is 10 kHz
  TIMER_CAR(TIMER0) = 0xFFFF; //Get MAX. value
	TIMER_CHCTL0(TIMER0) |= (1<<9 | 1<<0); //CH0 - RISING EDGE, CH1 - FALLING EDGE
	TIMER_CHCTL2(TIMER0) |= TIMER_CHCTL2_CH0EN | TIMER_CHCTL2_CH1EN | TIMER_CHCTL2_CH1P;
  TIMER_CTL0(TIMER0) |= TIMER_CTL0_CEN;
}

// Used for generation pwm_11 && pwm_12 signal
void _tim1_init(void)
{
  rcu_periph_clock_enable(RCU_AF);
  static timer_oc_parameter_struct timer_ocintpara;
  static timer_parameter_struct timer_initpara;
  timer_deinit(TIMER1);

  /* TIMER1 configuration */
  timer_initpara.prescaler = 1079;
  timer_initpara.alignedmode = TIMER_COUNTER_EDGE;
  timer_initpara.counterdirection = TIMER_COUNTER_UP;
  timer_initpara.period = 999;
  timer_initpara.clockdivision = TIMER_CKDIV_DIV1;
  timer_initpara.repetitioncounter = 0;
  timer_init(TIMER1, &timer_initpara);

  /* CH1,CH2 and CH3 configuration in PWM mode1 */
  timer_ocintpara.ocpolarity = TIMER_OC_POLARITY_HIGH;
  timer_ocintpara.outputstate = TIMER_CCX_ENABLE;
  timer_ocintpara.ocnpolarity = TIMER_OCN_POLARITY_HIGH;
  timer_ocintpara.outputnstate = TIMER_CCXN_DISABLE;
  timer_ocintpara.ocidlestate = TIMER_OC_IDLE_STATE_LOW;
  timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;

  timer_channel_output_config(TIMER1, TIMER_CH_0, &timer_ocintpara);

  /* CH0 configuration in PWM mode1,duty cycle 75% */
  timer_channel_output_pulse_value_config(TIMER1, TIMER_CH_0, 599);
  timer_channel_output_mode_config(TIMER1, TIMER_CH_0, TIMER_OC_MODE_PWM0);
  timer_channel_output_shadow_config(TIMER1, TIMER_CH_0, TIMER_OC_SHADOW_DISABLE);

  /* auto-reload preload enable */
  timer_auto_reload_shadow_enable(TIMER1);
  /* auto-reload preload enable */
  timer_enable(TIMER1);
  disable_pwm(pwm_1);
  disable_pwm(pwm_2);
}

void _tim2_init(void)
{

  rcu_periph_clock_enable(RCU_AF);
  static timer_oc_parameter_struct timer_ocintpara;
  static timer_parameter_struct timer_initpara;

  timer_deinit(TIMER2);

  /* TIMER1 configuration */
  timer_initpara.prescaler = 1079;
  timer_initpara.alignedmode = TIMER_COUNTER_EDGE;
  timer_initpara.counterdirection = TIMER_COUNTER_UP;
  timer_initpara.period = 999;
  timer_initpara.clockdivision = TIMER_CKDIV_DIV1;
  timer_initpara.repetitioncounter = 0;
  timer_init(TIMER2, &timer_initpara);

  /* CH1,CH2 and CH3 configuration in PWM mode1 */
  timer_ocintpara.ocpolarity = TIMER_OC_POLARITY_HIGH;
  timer_ocintpara.outputstate = TIMER_CCX_ENABLE;
  timer_ocintpara.ocnpolarity = TIMER_OCN_POLARITY_HIGH;
  timer_ocintpara.outputnstate = TIMER_CCXN_DISABLE;
  timer_ocintpara.ocidlestate = TIMER_OC_IDLE_STATE_LOW;
  timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;

  //  timer_channel_output_config(TIMER2, TIMER_CH_2, &timer_ocintpara);
  timer_channel_output_config(TIMER2, TIMER_CH_3, &timer_ocintpara);

  /* CH3 configuration in PWM mode1,duty cycle 75% */
  timer_channel_output_pulse_value_config(TIMER2, TIMER_CH_3, 599);
  timer_channel_output_mode_config(TIMER2, TIMER_CH_3, TIMER_OC_MODE_PWM0);
  timer_channel_output_shadow_config(TIMER2, TIMER_CH_3, TIMER_OC_SHADOW_DISABLE);

  /* auto-reload preload enable */
  timer_auto_reload_shadow_enable(TIMER2);
  /* auto-reload preload enable */
  timer_enable(TIMER2);
}

void _tim3_init(void)
{
  __NOP();
}

void _adc_init(void)
{
  adc_deinit(ADC0);
  adc_mode_config(ADC_MODE_FREE);
  adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT); // LSB alignment
  ADC_CTL1(ADC0) |= (1 << 19 | 1 << 18 | 1 << 17);
  adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_3, ADC_SAMPLETIME_1POINT5); // WHAT IS RANK??? (0 value)
  adc_enable(ADC0);
}
