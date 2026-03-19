#ifndef __INCspi_h
#define __INCspi_h

#ifdef __cplusplus
extern "C" {
#endif /** __cplusplus */

#define SPI_MAX_TRANSFER 256

void SpiInit(void);
int SpiTransfer(uint8_t const * tx, uint8_t * rx, size_t len, uint8_t block);

#ifdef __cplusplus
}
#endif /** __cplusplus */
#endif /** __INCspi_h */