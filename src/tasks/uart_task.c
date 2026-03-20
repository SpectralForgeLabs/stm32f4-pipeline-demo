/*
 * uart_task.c
 */

#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4xx.h"

#include "uart.h"
#include "spi.h"
#include "uart_task.h"
#include "timer.h"
#include "buildinfo.h"

#include <stdio.h>
#include <string.h>

/** Defines */
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
/** Global/Static variables */

/** Functions */
void vTaskUart(void * varg)
{
    (void)varg; // Unused parameter
    
    char buffer[128];
    uint32_t microseconds = 0;
    
    
    UartInit();
    
    sprintf(buffer, "BOOT OK\n");
    UartTx((uint8_t *)buffer, strlen(buffer));

    sprintf(buffer, "BRANCH: %s\n", build_info.branch);
    UartTx((uint8_t *)buffer, strlen(buffer));

    sprintf(buffer, "BUILD NUMBER: %s\n", build_info.build_number);
    UartTx((uint8_t *)buffer, strlen(buffer));
    
    sprintf(buffer, "SHA: %s\n", build_info.git_sha);
    UartTx((uint8_t *)buffer, strlen(buffer));
    while(1)
    {

        // Delay 1 second
        vTaskDelay(pdMS_TO_TICKS(250));
        // Do something with microseconds value
        microseconds = usec_time_get();
        /** Causes missed bytes occasionaly, need to button up */
        // UartTx((uint8_t *)&microseconds, sizeof(microseconds));
        // UartTx((uint8_t *)&microseconds, sizeof(microseconds));
    }
}
