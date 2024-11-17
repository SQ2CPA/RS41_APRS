#include <stdint.h>
#include <system_stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_flash.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_dma.h>
#include <stm32f10x_adc.h>
#include <stm32f10x_tim.h>
#include <misc.h>

#include "system.h"
#include "config.h"
#include "delay.h"

#define BUTTON_PRESS_LONG_COUNT SYSTEM_SCHEDULER_TIMER_TICKS_PER_SECOND

#define ADC1_DR_Address ((uint32_t) 0x4001244C)

__IO uint16_t dma_buffer_adc[1];

volatile uint32_t button_pressed = 0;

void (*system_handle_timer_tick)() = NULL;

static volatile uint32_t systick_counter = 0;

static void nvic_init()
{
#ifdef  VECT_TAB_RAM
    NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else  // VECT_TAB_FLASH
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
#endif
}

static void rcc_init()
{
    RCC_DeInit();
    RCC_HSEConfig(RCC_HSE_ON);

    ErrorStatus hse_status = RCC_WaitForHSEStartUp();
    if (hse_status != SUCCESS) {
        while (true) {}
    }
    //SystemInit();

    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
    FLASH_SetLatency(FLASH_Latency_0);

    #ifndef CONFIG_STM32_CLOCK_6MHZ
    RCC_HCLKConfig(RCC_SYSCLK_Div1); // 24 MHz
    #else
    RCC_HCLKConfig(RCC_SYSCLK_Div4); // 6 MHz
    #endif

    RCC_PCLK1Config(RCC_HCLK_Div1);
    RCC_PCLK2Config(RCC_HCLK_Div1);

    RCC_SYSCLKConfig(RCC_SYSCLKSource_HSE);

    while (RCC_GetSYSCLKSource() != 0x04);
}

static void gpio_init()
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_AFIO, DISABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    GPIO_InitTypeDef gpio_init;

    // GPS power at PB4 PB5 PB6 PB7
    // gpio_init.GPIO_Pin = GPIO_Pin_4;
    // gpio_init.GPIO_Mode = GPIO_Mode_Out_PP;
    // gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    // GPIO_Init(GPIOB, &gpio_init);

    // gpio_init.GPIO_Pin = GPIO_Pin_5;
    // gpio_init.GPIO_Mode = GPIO_Mode_Out_PP;
    // gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    // GPIO_Init(GPIOB, &gpio_init);

    // gpio_init.GPIO_Pin = GPIO_Pin_6;
    // gpio_init.GPIO_Mode = GPIO_Mode_Out_PP;
    // gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    // GPIO_Init(GPIOB, &gpio_init);

    // gpio_init.GPIO_Pin = GPIO_Pin_7;
    // gpio_init.GPIO_Mode = GPIO_Mode_Out_PP;
    // gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    // GPIO_Init(GPIOB, &gpio_init);

    // SI5351+TXCO power at 
    // gpio_init.GPIO_Pin = GPIO_Pin_1;
    // gpio_init.GPIO_Mode = GPIO_Mode_Out_PP;
    // gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    // GPIO_Init(GPIOA, &gpio_init);

    // gpio_init.GPIO_Pin = GPIO_Pin_2;
    // gpio_init.GPIO_Mode = GPIO_Mode_Out_PP;
    // gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    // GPIO_Init(GPIOA, &gpio_init);

    // gpio_init.GPIO_Pin = GPIO_Pin_3;
    // gpio_init.GPIO_Mode = GPIO_Mode_Out_PP;
    // gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    // GPIO_Init(GPIOA, &gpio_init);

    // gpio_init.GPIO_Pin = GPIO_Pin_4;
    // gpio_init.GPIO_Mode = GPIO_Mode_Out_PP;
    // gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    // GPIO_Init(GPIOA, &gpio_init);

    // // PV voltage at PA5
    // gpio_init.GPIO_Pin = GPIO_Pin_5;
    // gpio_init.GPIO_Mode = GPIO_Mode_AIN;
    // gpio_init.GPIO_Speed = GPIO_Speed_10MHz;
    // GPIO_Init(GPIOA, &gpio_init);

    // External trigger at PA6
    gpio_init.GPIO_Pin = GPIO_Pin_5;
    gpio_init.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio_init);   

    // Converter at PB8
    gpio_init.GPIO_Pin = GPIO_Pin_8;
    gpio_init.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpio_init);
}

/**
 * Configure continuous DMA transfer for ADC from pins PA5 and PA6
 * to have battery voltage and button state available at all times.
 */
static void dma_adc_init()
{
    DMA_DeInit(DMA1_Channel1);

    DMA_InitTypeDef dma_init;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    dma_init.DMA_BufferSize = 1; // 2
    dma_init.DMA_DIR = DMA_DIR_PeripheralSRC;
    dma_init.DMA_M2M = DMA_M2M_Disable;
    dma_init.DMA_MemoryBaseAddr = (uint32_t) &dma_buffer_adc;
    dma_init.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    dma_init.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dma_init.DMA_Mode = DMA_Mode_Circular;
    dma_init.DMA_PeripheralBaseAddr = ADC1_DR_Address;
    dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    dma_init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dma_init.DMA_Priority = DMA_Priority_High;
    DMA_Init(DMA1_Channel1, &dma_init);

    DMA_Cmd(DMA1_Channel1, ENABLE);

    RCC_ADCCLKConfig(RCC_PCLK2_Div2);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    ADC_InitTypeDef adc_init;
    adc_init.ADC_Mode = ADC_Mode_Independent;
    adc_init.ADC_ScanConvMode = ENABLE; // ENABLE
    adc_init.ADC_ContinuousConvMode = ENABLE;
    adc_init.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    adc_init.ADC_DataAlign = ADC_DataAlign_Right;
    adc_init.ADC_NbrOfChannel = 1; // 2
    ADC_Init(ADC1, &adc_init);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 1, ADC_SampleTime_28Cycles5);
    // ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 1, ADC_SampleTime_28Cycles5);
    // ADC_RegularChannelConfig(ADC1, ADC_Channel_Vrefint, 1, ADC_SampleTime_28Cycles5);

    // ADC1 DMA requests are routed to DMA1 Channel1
    ADC_DMACmd(ADC1, ENABLE);
    ADC_Cmd(ADC1, ENABLE);

    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1));

    // Start new calibration (ADC must be off at that time)
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1));

    // Start conversion (will be endless as we are in continuous mode)
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

void system_shutdown()
{
    GPIO_SetBits(GPIOA, GPIO_Pin_12);
}

void system_scheduler_timer_init()
{
    // Timer frequency = TIM_CLK/(TIM_PSC+1)/(TIM_ARR + 1)
    // TIM_CLK =
    // TIM_PSC = Prescaler
    // TIM_ARR = Period
    TIM_DeInit(TIM4);

    TIM_TimeBaseInitTypeDef tim_init;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_TIM4, DISABLE);

    #ifndef CONFIG_STM32_CLOCK_6MHZ
    tim_init.TIM_Prescaler = 24 - 1; // tick every 1/1000000 s 24 MHz
    #else
    tim_init.TIM_Prescaler = 6 - 1; // tick every 1/1000000 s 6 MHz
    #endif

    tim_init.TIM_CounterMode = TIM_CounterMode_Up;
    tim_init.TIM_Period = 100 - 1; // update every 1/10000 s
    tim_init.TIM_ClockDivision = TIM_CKD_DIV1;
    tim_init.TIM_RepetitionCounter = 0;

    TIM_TimeBaseInit(TIM4, &tim_init);

    TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);

    NVIC_InitTypeDef nvic_init;
    nvic_init.NVIC_IRQChannel = TIM4_IRQn;
    nvic_init.NVIC_IRQChannelPreemptionPriority = 3;
    nvic_init.NVIC_IRQChannelSubPriority = 1;
    nvic_init.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic_init);

    TIM_Cmd(TIM4, ENABLE);
}

uint16_t system_get_battery_voltage_millivolts()
{
    return ADC_GetConversionValue(ADC1);

    // return (uint16_t) ((((float) dma_buffer_adc[0]) / 4096.0f) * 3300.0f);

    // const float VREFINT = 1.2f;  // Zaktualizuj w zależności od modelu STM32
    // uint16_t adc_value = ADC_GetConversionValue(ADC1);  // Odczytaj wartość z ADC
    
    // uint16_t voltage = (uint16_t)((VREFINT * 4096.0f / adc_value) * 1000);
    // return voltage;
}

void system_disable_tick()
{
    TIM_Cmd(TIM4, DISABLE);
    NVIC_DisableIRQ(TIM4_IRQn);
    TIM_ITConfig(TIM4, TIM_IT_Update, DISABLE);
    TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
}

void system_enable_tick()
{
    TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
    NVIC_EnableIRQ(TIM4_IRQn);
    TIM_Cmd(TIM4, ENABLE);
}

void system_disable_irq()
{
    __disable_irq();
}

void system_enable_irq()
{
    __enable_irq();
}

void system_init()
{
    rcc_init();
    nvic_init();
    gpio_init();
    // dma_adc_init(); not required by now
    delay_init();

    system_scheduler_timer_init();

    RCC_ClocksTypeDef RCC_Clocks;
    RCC_GetClocksFreq(&RCC_Clocks);

    delay_ms(100);

    #ifndef CONFIG_STM32_CLOCK_6MHZ
    SysTick_Config(SystemCoreClock / 10000); // 24 MHz
    #else
    SysTick_Config(SystemCoreClock / 10000 / 4); // 6 MHz
    #endif
}

uint32_t system_get_tick()
{
    return systick_counter;
}

void SysTick_Handler()
{
    systick_counter++;
}

void TIM4_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);

        system_handle_timer_tick();

    }
}
