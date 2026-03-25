/*
 * stubs/uart_task.h
 * Shadows uart_task.h for host unit test builds.
 * Provides os_write and fd defines without FreeRTOS dependency.
 */
#ifndef __INCuart_task_h
#define __INCuart_task_h

#include <stdint.h>

/** FD defines — must match production uart_task.h */
#define OS_IO_BUF_SIZE   128
#define OS_TX_FD_UART1   1

/** os_write — implemented by os_write_stub.c in test builds */
void os_write(int fd, uint8_t *buf, uint16_t len);

#endif /* __INCuart_task_h */