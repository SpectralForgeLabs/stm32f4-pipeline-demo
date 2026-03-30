#ifndef __INCuart_task_h
#define __INCuart_task_h

#ifdef __cplusplus
extern "C" {
#endif /** __cplusplus */

#include "FreeRTOS.h"
#include "queue.h"
#include <stdint.h>

/** Defines */
#define OS_TX_BUF_SIZE  256
#define OS_TX_FD_UART1  1

/** Types */
typedef struct
{
    int      fd;
    uint16_t len;
    uint8_t  buf[OS_TX_BUF_SIZE];
} os_tx_msg_t;

/** Queues */
extern QueueHandle_t uart_tx_queue;

/** Functions */
void vTaskUartTx(void *);
void vTaskUartRx(void *);
void os_write(int fd, uint8_t * buf, uint16_t len);

#ifdef __cplusplus
}
#endif /** __cplusplus */

#endif /** __INCuart_task_h */