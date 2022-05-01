#include "main.h"

void _clk_init(void)
{
  RCU_APB2EN |= RCU_APB2EN_PAEN | RCU_APB2EN_PBEN | RCU_APB2EN_PCEN;
  RCU_APB2EN |= RCU_APB2EN_USART0EN;
  RCU_APB2EN |= RCU_APB2EN_TIMER0EN | RCU_APB1EN_TIMER1EN;
  RCU_APB2EN |= RCU_APB2EN_ADC0EN;

  RCU_APB1EN |= RCU_APB1EN_TIMER1EN;
  RCU_APB1EN |= RCU_APB1EN_TIMER2EN;
  RCU_APB1EN |= RCU_APB1EN_TIMER3EN;

  rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV8);
}

void _gpio_init(void)
{
  gpio_init(GPIOC, GPIO_MODE_OUT_OD, GPIO_OSPEED_10MHZ, GPIO_PIN_13);     // It's led for indicate activity
  gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_0); // input signal
  gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);     // signal response
  gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0);      // inverted state for input caapture signal
  gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1);       // PWM_12, TIM3_CH3
  gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0);       // PWM_OUT, PA0

  gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_2); // ADC0_CH2
  gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_3); // ADC0_CH3

  gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_2);  // NSS_0
  gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_10); // NSS_1
  gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_7);  // SDI
  gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_5);  // SCK
}

void _usart_init(void)
{
//  usart_deinit(USART0);
//  usart_baudrate_set(USART0, 115200UL);
//  usart_parity_config(USART0, USART_PM_NONE);
//  usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
//  usart_receive_config(USART0, USART_RECEIVE_ENABLE);
//  usart_interrupt_enable(USART0, USART_INT_RBNE);
//  gpio_afio_deinit();
//  gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
//  //gpio_init(GPIOA, GPIO_MODE_IPD, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
//  usart_enable(USART0);
}

void _tim0_init(void)
{
  static timer_ic_parameter_struct timer_icinitpara;
  static timer_parameter_struct timer_initpara;

  timer_deinit(TIMER2);

  /* TIMER2 configuration */
  timer_initpara.prescaler = 107;
  timer_initpara.alignedmode = TIMER_COUNTER_EDGE;
  timer_initpara.counterdirection = TIMER_COUNTER_UP;
  timer_initpara.period = 65535;
  timer_initpara.clockdivision = TIMER_CKDIV_DIV1;
  timer_initpara.repetitioncounter = 0;
  timer_init(TIMER2, &timer_initpara);

  /* TIMER2 CH0 PWM input capture configuration */
  timer_icinitpara.icpolarity = TIMER_IC_POLARITY_RISING;
  timer_icinitpara.icselection = TIMER_IC_SELECTION_DIRECTTI;
  timer_icinitpara.icprescaler = TIMER_IC_PSC_DIV1;
  timer_icinitpara.icfilter = 0x0;
  timer_input_pwm_capture_config(TIMER0, TIMER_CH_0, &timer_icinitpara);

  /* slave mode selection: TIMER2 */
  // timer_input_trigger_source_select(TIMER0,TIMER_SMCFG_TRGSEL_CI0FE0);
  // timer_slave_mode_select(TIMER2,TIMER_SLAVE_MODE_RESTART);

  /* select the master slave mode */
  // timer_master_slave_mode_config(TIMER0,TIMER_MASTER_SLAVE_MODE_ENABLE);

  /* auto-reload preload enable */
  timer_auto_reload_shadow_enable(TIMER0);
  /* clear channel 0 interrupt bit */
  timer_interrupt_flag_clear(TIMER0, TIMER_INT_CH0);
  /* channel 0 interrupt enable */
  timer_interrupt_enable(TIMER0, TIMER_INT_CH0);

  /* TIMER2 counter enable */
  timer_enable(TIMER0);
}

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
  TIMER_CAR(TIMER3) = 1000U;
  TIMER_PSC(TIMER3) = 1079U;
  TIMER_CH3CV(TIMER3) = 499U;
  TIMER_CHCTL2(TIMER3) |= TIMER_CHCTL2_CH3EN;
  TIMER_CTL0(TIMER3) = TIMER_CTL0_CEN;
}

void _adc_init(void)
{
  adc_deinit(ADC0);
  adc_mode_config(ADC_MODE_FREE);
  adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);                       // LSB alignment
  adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_2, ADC_SAMPLETIME_1POINT5); // WHAT IS RANK??? (0 value)
  adc_enable(ADC0);
}

