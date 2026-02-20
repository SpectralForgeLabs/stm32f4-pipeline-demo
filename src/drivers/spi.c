/*
 * spi.c
 */

#include "stm32f4xx.h"

#include "spi.h"

/** Defines */
#define GPIO_AFRL_AFSEL3_5 (GPIO_AFRL_AFSEL3_0 | GPIO_AFRL_AFSEL3_2)
#define GPIO_AFRL_AFSEL4_5 (GPIO_AFRL_AFSEL4_0 | GPIO_AFRL_AFSEL4_2)
#define GPIO_AFRL_AFSEL5_5 (GPIO_AFRL_AFSEL5_0 | GPIO_AFRL_AFSEL5_2)

#define SPI_CR1_BR_16 (SPI_CR1_BR_1 | SPI_CR1_BR_0)
/** Global/Static variables */

/** Functions */

void SpiInit(void)
{
    /** 
     * Enable clock to SPI
     */
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    
    /** Set mux for PB3/4/5: SCK MISO MOSI AF5*/
    GPIOB->MODER |= GPIO_MODER_MODER3_1 |
                    GPIO_MODER_MODER4_1 |
                    GPIO_MODER_MODER5_1;
    GPIOB->AFR[0] |= GPIO_AFRL_AFSEL3_5 | GPIO_AFRL_AFSEL4_5 | GPIO_AFRL_AFSEL5_5;

    /** 
     * Config SPI
     */
    SPI1->CR1 |= SPI_CR1_MSTR |
                 SPI_CR1_SPE |
                 SPI_CR1_BR_16 |
                 SPI_CR1_SSM |
                 SPI_CR1_SSI;
}

uint8_t SpiTxRx(uint8_t by)
{
    while ((SPI1->SR & SPI_SR_TXE) != SPI_SR_TXE) { }
    SPI1->DR = by;              // 8-bit access is safest
    while ((SPI1->SR & SPI_SR_RXNE) != SPI_SR_RXNE) { }
    return SPI1->DR;
}


