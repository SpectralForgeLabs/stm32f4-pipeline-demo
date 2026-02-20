/*
 * uart_task.c
 */

#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4xx.h"

#include "uart.h"
#include "spi.h"
#include "uart_task.h"

/** Defines */

/** Global/Static variables */

/** Functions */
void vTaskUart(void * varg)
{
    UartInit();

    while(1)
    {
        // Delay 1 second
        vTaskDelay(pdMS_TO_TICKS(500));
        USART1->DR = 0x50;
    }
}
