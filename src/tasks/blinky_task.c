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

    char by[6];
    while(1)
    {
        // Delay 1 second
        vTaskDelay(pdMS_TO_TICKS(500));
        GPIOA->ODR ^= (GPIO_ODR_OD6 | GPIO_ODR_OD7);
        
        /** Slave device is tied to PBO */
        GPIOB->BSRR = GPIO_BSRR_BR0;
        by[0] = SpiTxRx(0x9f);
        by[1] = SpiTxRx(0xff);
        by[2] = SpiTxRx(0xff);
        by[3] = SpiTxRx(0xff);
        by[4] = SpiTxRx(0xff);
        by[5] = SpiTxRx(0xff);
        by[6] = SpiTxRx(0xff);

        GPIOB->BSRR = GPIO_BSRR_BS0;
    }
}
