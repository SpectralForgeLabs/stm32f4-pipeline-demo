/* stubs/uart_stub.c */
#include <stddef.h>
#include <stdint.h>
#include "FreeRTOS_stub.h"

QueueHandle_t uart_rx_queue = NULL;

void uart_printf(const char *fmt, ...) { (void)fmt; }
void uart_write(uint8_t *buf, uint16_t len) { (void)buf; (void)len; }