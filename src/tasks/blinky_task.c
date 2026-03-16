/*
 * blinky_task.c
 */

#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4xx.h"

#include "uart.h"
#include "spi.h"
#include "blinky_task.h"

#include <stdbool.h>
/** Defines */

/** Global/Static variables */
static uint8_t spi_rx_buf[256];
/** Functions */
void vTaskBlinky(void * varg)
{

    // Quick sanity: enable GPIOD clock (F407 Discovery LED port)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    GPIOA->MODER |= GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0;
    GPIOA->OTYPER |= GPIO_OTYPER_OT6 | GPIO_OTYPER_OT7;

    GPIOA->ODR |= GPIO_ODR_OD6 | GPIO_ODR_OD7;

    GPIOB->MODER |= GPIO_MODER_MODE0_0;
    GPIOB->OTYPER &= ~GPIO_OTYPER_OT0;

    SpiInit();

    uint8_t by[4] = {0x9f, 0xff, 0xff, 0xff};
    uint8_t UNIQUE_ID[9] = {0xab, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    while(1)
    {
        // Delay 1 second
        vTaskDelay(pdMS_TO_TICKS(500));
        GPIOA->ODR ^= (GPIO_ODR_OD6 | GPIO_ODR_OD7);
        
        SpiTransfer(by, spi_rx_buf, sizeof(by), true);
        SpiTransfer(UNIQUE_ID, spi_rx_buf, sizeof(UNIQUE_ID), true);
    }
}
