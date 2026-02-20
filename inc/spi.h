#ifndef __INCspi_h
#define __INCspi_h

#ifdef __cplusplus
extern "C" {
#endif /** __cplusplus */

void SpiInit(void);
uint8_t SpiTxRx(uint8_t by);

#ifdef __cplusplus
}
#endif /** __cplusplus */
#endif /** __INCspi_h */