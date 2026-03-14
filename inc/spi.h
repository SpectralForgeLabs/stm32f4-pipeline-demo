#ifndef __INCspi_h
#define __INCspi_h

#ifdef __cplusplus
extern "C" {
#endif /** __cplusplus */

extern char spi_rx_buf[256];
extern char spi_tx_buf[256];

void SpiInit(void);
uint8_t SpiTxRx(uint8_t * by, size_t len);

#ifdef __cplusplus
}
#endif /** __cplusplus */
#endif /** __INCspi_h */