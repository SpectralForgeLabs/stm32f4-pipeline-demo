/*
 * blinky_task.c
 */

#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4xx.h"

#include "uart.h"
#include "spi.h"
#include "blinky_task.h"

/** Defines */

/** Global/Static variables */

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

    uint8_t by[6] = {0x9f, 0xff, 0xff, 0xff, 0xff, 0xff};
    while(1)
    {
        // Delay 1 second
        vTaskDelay(pdMS_TO_TICKS(500));
        GPIOA->ODR ^= (GPIO_ODR_OD6 | GPIO_ODR_OD7);
        
        SpiTxRx(by, sizeof(by));
    }
}
