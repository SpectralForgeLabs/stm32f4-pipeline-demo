#ifndef __INCuart_h
#define __INCuart_h

#ifdef __cplusplus
extern "C" {
#endif /** __cplusplus */

void UartInit(void);
void UartTx(uint8_t * data, uint16_t size);

#ifdef __cplusplus
}
#endif /** __cplusplus */
#endif /** __INCuart_h */