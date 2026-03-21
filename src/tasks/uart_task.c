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
#include "spectral_framer.h"
#include "spectral_parser.h"

#include <stdio.h>
#include <string.h>

/** Defines */
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
/** Global/Static variables */
// static char buffer[128];

/** Functions */
void os_putc(uint8_t byte);

/*******************************************************************************
 * @brief Get time in tick for framer
 */
uint32_t freertos_get_time(void)
{
    return (uint32_t)xTaskGetTickCount();
}

/*******************************************************************************
 * @brief Task to handle UART transmission
 */
void vTaskUartTx(void * varg)
{
    (void)varg; // Unused parameter

    uint32_t microseconds = 0;

    uart_printf("BOOT OK\n");
    uart_printf("BRANCH: %s\n",       build_info.branch);
    uart_printf("BUILD NUMBER: %s\n", build_info.build_number);
    uart_printf("SHA: %s\n",          build_info.git_sha);

    while(1)
    {

        // Delay 1 second
        vTaskDelay(pdMS_TO_TICKS(250));
        // Do something with microseconds value
        microseconds = usec_time_get();
    }
}

/******************************************************************************* 
 * @brief
 */
void vTaskUartRx(void * varg)
{
    (void)varg; // Unused parameter
    uint8_t by;
    static framer_t framer;
    static spectral_frame_t frame;

    framer_init(&framer, freertos_get_time);

    while(1)
    {
        if (xQueueReceive(uart_rx_queue, &by, portMAX_DELAY) == pdPASS) 
        {
            framer_state_t state = framer_process(&framer, &frame, by);

            if (state == FRAME_DONE)
            {
                /** parse framed data */
                uart_printf("Received frame: len=%d, crc=%02X\n", frame.len, frame.crc);
                spectral_parse(frame.data, frame.len);
                framer_reset(&framer);
            }
            else if (state == FRAME_ERROR)
            {
                switch (framer.last_error)
                {
                    case FRAME_ERR_INVALID_SOF:
                        uart_printf("Invalid SOF\n");
                        break;
                    case FRAME_ERR_INVALID_EOF:
                        uart_printf("Invalid EOF\n");
                        break;
                    case FRAME_ERR_OVERFLOW:
                        uart_printf("Frame overflow\n");
                        break;
                    case FRAME_ERR_BAD_CRC:
                        uart_printf("Bad CRC\n");
                        break;
                    case FRAME_ERR_BYTE_TIMEOUT:
                        uart_printf("Byte timeout\n");
                        break;
                    case FRAME_ERR_FRAME_TIMEOUT:
                        uart_printf("Frame timeout\n");
                        break;
                    default:
                        uart_printf("Unknown error\n");
                        break;
                }
                framer_reset(&framer);
            }
        }
        else
        {
            // Handle error: failed to receive from queue
        }
    }
}

/*******************************************************************************
 * @brief putc function for printf, sends a single byte over UART
 * @note future me needs to put in own layer and pipe to printf
 * @param byte the byte to send
 */
void os_putc(uint8_t byte)
{
    uart_printf("%c", byte);
}

