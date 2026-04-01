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
#include <stdarg.h>

/** Defines */
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
/** Global/Static variables */
QueueHandle_t uart_tx_queue = NULL;
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
 * @brief Write raw bytes to output queue — dispatched by vTaskUartTx
 * @note  buf[0] of queued message = fd, maps to UART instance
 * @param fd   file descriptor (e.g. OS_TX_FD_UART1)
 * @param buf  byte buffer to send
 * @param len  number of bytes — clamped to OS_TX_BUF_SIZE
 */
void os_write(int fd, uint8_t * buf, uint16_t len)
{
    if (uart_tx_queue == NULL || buf == NULL || len == 0)
        return;
 
    if (len > OS_TX_BUF_SIZE)
        len = OS_TX_BUF_SIZE;
 
    os_tx_msg_t msg;
    msg.fd  = fd;
    msg.len = len;
    memcpy(msg.buf, buf, len);
 
    xQueueSend(uart_tx_queue, &msg, 0);
}

/*******************************************************************************
 * @brief Task to handle UART transmission
 */
void vTaskUartTx(void * varg)
{
    (void)varg; // Unused parameter
    os_tx_msg_t msg;

    uart_tx_queue = xQueueCreate(16, sizeof(os_tx_msg_t));
    if (uart_tx_queue == NULL)
    {
        return;
    }

    uart_printf("BOOT OK\n");
    uart_printf("BRANCH: %s\n",       build_info.branch);
    uart_printf("BUILD NUMBER: %s\n", build_info.build_number);
    uart_printf("SHA: %s\n",          build_info.git_sha);

    while (1)
    {
        /** Drain os_tx_queue — dispatch on fd in buf[0] */
        while (xQueueReceive(uart_tx_queue, &msg, 0) == pdPASS)
        {
            uint8_t swtch = 1;
            switch (swtch)
            {
                case OS_TX_FD_UART1:
                    uart_write(msg.buf, msg.len);
                    break;
                default:
                    /* Unknown fd — drop */
                    break;
            }
        }
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
    (void)byte;
}

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
    (void)byte;
}

