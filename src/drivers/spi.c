/*
 * spi.c
 */

#include "stm32f4xx.h"
#include <stddef.h>
#include <string.h>
#include "spi.h"
/** Defines */
#define GPIO_AFRL_AFSEL3_5 (GPIO_AFRL_AFSEL3_0 | GPIO_AFRL_AFSEL3_2)
#define GPIO_AFRL_AFSEL4_5 (GPIO_AFRL_AFSEL4_0 | GPIO_AFRL_AFSEL4_2)
#define GPIO_AFRL_AFSEL5_5 (GPIO_AFRL_AFSEL5_0 | GPIO_AFRL_AFSEL5_2)

#define SPI_CR1_BR_16 (SPI_CR1_BR_1 | SPI_CR1_BR_0)

#define FLASH_CS_LOW()   (GPIOB->BSRR = GPIO_BSRR_BR0)
#define FLASH_CS_HIGH()  (GPIOB->BSRR = GPIO_BSRR_BS0)

#define dma0_isr DMA2_Stream0_IRQHandler
#define dma3_isr DMA2_Stream3_IRQHandler
/** Global/Static variables */
char spi_rx_buf[256];
char spi_tx_buf[256];
/** Functions */
void ConfigureSpiDma(void);
/*******************************************************************************
 * @brief DMA2_Stream0 interrupt handler
 */
void dma0_isr(void)
{
    if (DMA2->LISR & DMA_LISR_TCIF0) // Check if transfer complete interrupt flag is set
    {
        // Handle transfer complete event (e.g., signal a task, set a flag, etc.)
        DMA2->LIFCR =
        DMA_LIFCR_CFEIF0 |
        DMA_LIFCR_CDMEIF0 |
        DMA_LIFCR_CTEIF0 |
        DMA_LIFCR_CHTIF0 |
        DMA_LIFCR_CTCIF0;

        DMA2_Stream0->CR &= ~DMA_SxCR_EN;
        SPI1->CR2 &= ~SPI_CR2_RXDMAEN;
    }
}

/*******************************************************************************
 * @brief DMA2_Stream3 interrupt handler
 */
void dma3_isr(void)
{
    if (DMA2->LISR & DMA_LISR_TCIF3) // Check if transfer complete interrupt flag is set
    {
        // Handle transfer complete event (e.g., signal a task, set a flag, etc.)
        DMA2->LIFCR =
        DMA_LIFCR_CFEIF3 |
        DMA_LIFCR_CDMEIF3 |
        DMA_LIFCR_CTEIF3 |
        DMA_LIFCR_CHTIF3 |
        DMA_LIFCR_CTCIF3;

        DMA2_Stream3->CR &= ~DMA_SxCR_EN;
        SPI1->CR2 &= ~SPI_CR2_TXDMAEN;
        FLASH_CS_HIGH();
    }
}

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

uint8_t SpiTxRx(uint8_t * by, size_t len)
{
    memcpy(spi_tx_buf, by, len);
    /** Enable DMA for SPI1 */
    ConfigureSpiDma();
    SPI1->CR2 |= SPI_CR2_RXDMAEN | SPI_CR2_TXDMAEN;

    FLASH_CS_LOW();

    DMA2_Stream0->CR |= DMA_SxCR_EN;
    DMA2_Stream3->CR |= DMA_SxCR_EN;

    return 0;
}
/*******************************************************************************
 * @brief Configures the DMA for SPI1
 * @note: Use DMA2 stream 0 channel 3 for rx and stream 3 channel 3 for tx
 */
void ConfigureSpiDma(void)
{
    /** Enable periph clock */
    if (!(RCC->AHB1ENR & RCC_AHB1ENR_DMA2EN)) 
    {
        RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
    }
    DMA2_Stream0->CR &= ~DMA_SxCR_EN;
    while(DMA2_Stream0->CR & DMA_SxCR_EN);

    /** 2 & 3 Set peripheral and memory addresses address */
    DMA2_Stream0->PAR = (uint32_t)&SPI1->DR;
    DMA2_Stream0->M0AR = (volatile uint32_t)spi_rx_buf;

    /** 5 Select dma channel */
    // 100: channel 4 selected USART TX
    DMA2_Stream0->CR |= (DMA_SxCR_CHSEL_1 | DMA_SxCR_CHSEL_0);
    /** Increment memory pointer */
    DMA2_Stream0->CR |= DMA_SxCR_MINC;
    
    /** Set trasnfer from peripheral memory*/
    DMA2_Stream0->CR &= ~(DMA_SxCR_DIR);

    /** Enabel transfer complete interrupts */
    DMA2_Stream0->CR |= DMA_SxCR_TCIE;

    DMA2_Stream3->CR &= ~DMA_SxCR_EN;
    while(DMA2_Stream3->CR & DMA_SxCR_EN);

    /** 2 & 3 Set peripheral and memory addresses address */
    DMA2_Stream3->PAR = (uint32_t)&SPI1->DR;
    DMA2_Stream3->M0AR = (volatile uint32_t)spi_tx_buf;

    /** 5 Select dma channel */
    // 100: channel 4 selected USART TX
    DMA2_Stream3->CR |= (DMA_SxCR_CHSEL_1 | DMA_SxCR_CHSEL_0);
    /** Increment memory pointer */
    DMA2_Stream3->CR |= DMA_SxCR_MINC;
    
    /** Set transfer from memory to peripheral */
    DMA2_Stream3->CR |= (DMA_SxCR_DIR_0);

    /** Enabel transfer complete interrupts */
    DMA2_Stream3->CR |= DMA_SxCR_TCIE;
    /** Enable Timer 2 interrupt in NVIC */
    NVIC_SetPriority(DMA2_Stream0_IRQn, 5);
    NVIC_EnableIRQ(DMA2_Stream0_IRQn);
    NVIC_SetPriority(DMA2_Stream3_IRQn, 5);
    NVIC_EnableIRQ(DMA2_Stream3_IRQn);

    /** enable dma */
    // DMA1_Stream7->CR |= DMA_SxCR_EN;

    // Use stream 5 for channel 4 USART RX
}
