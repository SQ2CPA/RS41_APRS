void *__dso_handle = 0;

#include "hal/system.h"
#include "hal/i2c.h"
#include "hal/delay.h"
#include "hal/datatimer.h"
#include "config.h"
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_usart.h>
#include "gps.h"
#include <stdio.h>
#include <stdint.h>

#define I2C_BUS_CLOCK_SPEED 100000

uint32_t counter = 0;

void handle_timer_tick()
{
    
}

void usart_gps_send_byte(uint8_t data)
{
    return; // UNCOMMENT FOR DEBUG ONLY
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {}
    USART_SendData(USART1, data);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {}
}

void usart_gps_send_string(const char *str)
{
    while (*str) {
        usart_gps_send_byte((uint8_t)*str++);
    }
}

void gps_power_enable() 
{
    GPIO_InitTypeDef gpio_init;
    
    gpio_init.GPIO_Pin = GPIO_Pin_7;
    gpio_init.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpio_init);

    GPIO_ResetBits(GPIOB, GPIO_Pin_7);

    USART_Cmd(USART1, ENABLE);
}

void gps_power_disable() 
{
    GPIO_InitTypeDef gpio_init;

    gpio_init.GPIO_Pin = GPIO_Pin_7;
    gpio_init.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpio_init);

    USART_Cmd(USART1, DISABLE);
}

int main(void)
{
    system_handle_timer_tick = handle_timer_tick;

    system_init();

    gps_init();

    gps_power_disable();

    delay_ms(1000);

    usart_gps_send_string("System initialized\r\n");

    delay_ms(250);

    #ifdef CONFIG_STARTUP_TONE_ENABLE
    usart_gps_send_string("Sending startup tone\r\n");
    // TODO
    usart_gps_send_string("Startup tone sent\r\n");
    #endif

    gps_power_enable();

    while (true) {
        gps_receive();

        uint8_t hasFIX = gps_hasFix();

        if (hasFIX == 1) {
            int seconds = gps_getSeconds();
            int minutes = gps_getMinutes();

            // TODO

            gps_power_disable();

            // TODO
            
            gps_power_enable();
        }

        counter = (counter + 1) % SYSTEM_SCHEDULER_TIMER_TICKS_PER_SECOND;
    }
}
