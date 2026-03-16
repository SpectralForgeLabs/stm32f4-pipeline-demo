/*
 * spi.c
 */

#include "stm32f4xx.h"
#include <stddef.h>
#include <string.h>
#include "spi.h"
#include "gpio.h"

/** Defines */
#define SPI_CR1_BR_16 (SPI_CR1_BR_1 | SPI_CR1_BR_0)

#define FLASH_CS_LOW()   GPIO_CLR(flash_cs)
#define FLASH_CS_HIGH()  GPIO_SET(flash_cs)

#define dma0_isr DMA2_Stream0_IRQHandler
/** Global/Static variables */
volatile uint8_t spi_busy = 0U;
volatile uint8_t spi_done = 0U;
volatile uint8_t spi_error = 0U;
gpio_handle_t flash_cs;

/** Functions */
void ConfigureSpiDma(void);
uint8_t SpiDmaStart(uint8_t const * tx, uint8_t * rx, size_t len);

/*******************************************************************************
 * @brief DMA2_Stream0 interrupt handler
 */
void dma0_isr(void)
{
    // Check if transfer complete interrupt flag is set
    if (DMA2->LISR & DMA_LISR_TCIF0) 
    {
        // Handle transfer complete event (e.g., signal a task, set a flag, etc.)
        DMA2->LIFCR = DMA_LIFCR_CFEIF0 |
                      DMA_LIFCR_CDMEIF0 |
                      DMA_LIFCR_CTEIF0 |
                      DMA_LIFCR_CHTIF0 |
                      DMA_LIFCR_CTCIF0 |
                      DMA_LIFCR_CFEIF3 |
                      DMA_LIFCR_CDMEIF3 |
                      DMA_LIFCR_CTEIF3 |
                      DMA_LIFCR_CHTIF3 |
                      DMA_LIFCR_CTCIF3;

        DMA2_Stream0->CR &= ~DMA_SxCR_EN;
        DMA2_Stream3->CR &= ~DMA_SxCR_EN;

        SPI1->CR2 &= ~(SPI_CR2_RXDMAEN | SPI_CR2_TXDMAEN);

        while (SPI1->SR & SPI_SR_BSY);

        FLASH_CS_HIGH();
        spi_busy = 0U;
        spi_done = 1U;
    }

    if (DMA2->LISR & (DMA_LISR_TEIF0 | DMA_LISR_DMEIF0 | DMA_LISR_FEIF0))
    {
        DMA2->LIFCR = DMA_LIFCR_CFEIF0 |
                      DMA_LIFCR_CDMEIF0 |
                      DMA_LIFCR_CTEIF0 |
                      DMA_LIFCR_CHTIF0 |
                      DMA_LIFCR_CTCIF0 |
                      DMA_LIFCR_CFEIF3 |
                      DMA_LIFCR_CDMEIF3 |
                      DMA_LIFCR_CTEIF3 |
                      DMA_LIFCR_CHTIF3 |
                      DMA_LIFCR_CTCIF3;

        DMA2_Stream0->CR &= ~DMA_SxCR_EN;
        DMA2_Stream3->CR &= ~DMA_SxCR_EN;
        SPI1->CR2 &= ~(SPI_CR2_RXDMAEN | SPI_CR2_TXDMAEN);

        while (SPI1->SR & SPI_SR_BSY);

        FLASH_CS_HIGH();

        spi_busy = 0U;
        spi_error = 1U;
    }
}

/*******************************************************************************
 * @brief Initializes SPI1 peripheral and configures DMA for SPI transfers
 */
void SpiInit(void)
{
    /** 
     * Enable clock to SPI
     */
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    
    gpio_config_t flash_cs_cfg =
    {
        .port = GPIO_PORT_B,
        .pin = GPIO_PIN_0,
        .mode = GPIO_MODE_OUTPUT,
        .output_type = GPIO_OUTPUT_PUSHPULL,
        .pull = GPIO_PULL_NONE,
        .speed = GPIO_SPEED_HIGH,
    };
    flash_cs = gpio_init(&flash_cs_cfg);
    /** Set mux for PB3/4/5: SCK MISO MOSI AF5*/
    gpio_config_t flash_sck_cfg =
    {
        .port = GPIO_PORT_B,
        .pin = GPIO_PIN_3, // SCK
        .mode = GPIO_MODE_ALT_FUNC,
        .output_type = GPIO_OUTPUT_PUSHPULL,
        .pull = GPIO_PULL_NONE,
        .speed = GPIO_SPEED_HIGH,
        .alternate_function = 5U
    };
    gpio_handle_t flash_sck = gpio_init(&flash_sck_cfg);
    
    gpio_config_t flash_miso_cfg =
    {
        .port = GPIO_PORT_B,
        .pin = GPIO_PIN_4, // MISO
        .mode = GPIO_MODE_ALT_FUNC,
        .output_type = GPIO_OUTPUT_PUSHPULL,
        .pull = GPIO_PULL_NONE,
        .speed = GPIO_SPEED_HIGH,
        .alternate_function = 5U
    };
    gpio_handle_t flash_miso = gpio_init(&flash_miso_cfg);

    gpio_config_t flash_mosi_cfg =
    {
        .port = GPIO_PORT_B,
        .pin = GPIO_PIN_5, // MOSI
        .mode = GPIO_MODE_ALT_FUNC,
        .output_type = GPIO_OUTPUT_PUSHPULL,
        .pull = GPIO_PULL_NONE,
        .speed = GPIO_SPEED_HIGH,
        .alternate_function = 5U
    };
    gpio_handle_t flash_mosi = gpio_init(&flash_mosi_cfg);

    /** 
     * Config SPI
     */
    SPI1->CR1 |= SPI_CR1_MSTR |
                 SPI_CR1_SPE |
                 SPI_CR1_BR_16 |
                 SPI_CR1_SSM |
                 SPI_CR1_SSI;

    /** Configure DMA portion */
    ConfigureSpiDma();
    
    FLASH_CS_HIGH();
}

/*******************************************************************************
 * @brief Start SPI transfer using DMA
 * @param tx Pointer to transmit buffer
 * @param rx Pointer to receive buffer
 * @param len Length of data to transfer (must be <= SPI_MAX_TRANSFER)
 * @param block If non-zero, function will block until transfer is complete; 
 *              if zero, function will return immediately and transfer will 
 *              complete in background
 * @return 0 on success, -1 on invalid parameters, -2 if length exceeds maximum
 */
int SpiTransfer(uint8_t const * tx, uint8_t * rx, size_t len, uint8_t block)
{
    if ((tx == NULL) || (rx == NULL) || (len == 0U))
    {
        return -1;
    }

    if (len > SPI_MAX_TRANSFER)
    {
        return -2;
    }

    if (block)
    {
        while (spi_busy);
    }
    else
    {
        if (spi_busy)
        {
            return -3; // SPI is currently busy with another transfer
        }
    }

    spi_busy = 1U;
    spi_done = 0U;
    spi_error = 0U;

    FLASH_CS_LOW();
    SpiDmaStart(tx, rx, len);

    return 0;
}

/*******************************************************************************
 * @brief Starts SPI transfer using DMA
 * @param tx Pointer to transmit buffer
 * @param rx Pointer to receive buffer
 * @param len Length of data to transfer
 * @return 0 on success, non-zero on failure
 */
uint8_t SpiDmaStart(uint8_t const * tx, uint8_t * rx, size_t len)
{
    DMA2_Stream3->M0AR = (volatile uint32_t)tx;
    DMA2_Stream3->NDTR = len;
    DMA2_Stream3->CR |= DMA_SxCR_EN;

    DMA2_Stream0->M0AR = (volatile uint32_t)rx;
    DMA2_Stream0->NDTR = len;
    DMA2_Stream0->CR |= DMA_SxCR_EN;
    
    SPI1->CR2 |= SPI_CR2_RXDMAEN | SPI_CR2_TXDMAEN;

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
    
    /** 5 Select dma channel */
    /** 011: channel 3 SPI */
    /** Increment memory pointer */
    /** Set trasnfer from peripheral memory*/
    /** Enable transfer complete interrupts */
    DMA2_Stream0->CR =  DMA_SxCR_CHSEL_1 |
                        DMA_SxCR_CHSEL_0 |
                        DMA_SxCR_MINC   |
                        DMA_SxCR_TCIE;
    

    DMA2_Stream3->CR &= ~DMA_SxCR_EN;
    while(DMA2_Stream3->CR & DMA_SxCR_EN);

    /** 2 & 3 Set peripheral and memory addresses address */
    DMA2_Stream3->PAR = (uint32_t)&SPI1->DR;

    /** 5 Select dma channel */
    // 100: channel 4 selected USART TX
    DMA2_Stream3->CR =  DMA_SxCR_CHSEL_1 |
                        DMA_SxCR_CHSEL_0 |
                        DMA_SxCR_MINC   |
                        DMA_SxCR_DIR_0;


    /** Enable Timer 2 interrupt in NVIC */
    NVIC_SetPriority(DMA2_Stream0_IRQn, 5);
    NVIC_EnableIRQ(DMA2_Stream0_IRQn);
    // NVIC_SetPriority(DMA2_Stream3_IRQn, 5);
    // NVIC_EnableIRQ(DMA2_Stream3_IRQn);
}
