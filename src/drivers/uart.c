/*
 * uart.c
 */

#include "stm32f4xx.h"

#include "uart.h"
#include <string.h>

#define dma2_isr DMA2_Stream7_IRQHandler

void dma2_isr(void)
{
    if (DMA2->HISR & DMA_HISR_TCIF7) // Check if transfer complete interrupt flag is set
    {
        // Handle transfer complete event (e.g., signal a task, set a flag, etc.)
        DMA2->HIFCR =
        DMA_HIFCR_CFEIF7 |
        DMA_HIFCR_CDMEIF7 |
        DMA_HIFCR_CTEIF7 |
        DMA_HIFCR_CHTIF7 |
        DMA_HIFCR_CTCIF7;

        DMA2_Stream7->CR &= ~DMA_SxCR_EN;
    }
}

void ConfigureUsartDma(void);

/** Defines */
#define GPIO_AFHR_AFSEL9_7 (GPIO_AFRH_AFSEL9_0 | GPIO_AFRH_AFSEL9_1 | GPIO_AFRH_AFSEL9_2)
#define GPIO_AFHR_AFSEL10_7 (GPIO_AFRH_AFSEL10_0 | GPIO_AFRH_AFSEL10_1 | GPIO_AFRH_AFSEL10_2)
/** Global/Static variables */

uint8_t uart_buf[256];

/** Functions */

void UartInit(void)
{
    /** 
     * Enable clock to GPIOA, TX:PA_9 RX:PA_10 
     * Enable clock to USART
     */
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    
    /** Set mux for  PA9/10 AF7*/
    GPIOA->MODER |= GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1;
    GPIOA->AFR[1] |= GPIO_AFHR_AFSEL9_7 | GPIO_AFHR_AFSEL10_7;

    /** 
     * Baudrate set to 115.2 use 8.6875 for BRR 
     * DIV_fraction = 11
     * DIV_mantissa = 8
     */
     USART1->BRR |= (11 << USART_BRR_DIV_Fraction_Pos);
     USART1->BRR |= (8 << USART_BRR_DIV_Mantissa_Pos);

    /** Enable USART TX and RX leave every default 8 bit 1 stop no parity */
    USART1->CR1 |= USART_CR1_TE | USART_CR1_RE;

    USART1->CR3 |= USART_CR3_DMAT;
    ConfigureDma();

    /* Start USART */
    USART1->CR1 |= USART_CR1_UE;
}

void ConfigureUsartDma(void)
{
    /** Enable periph clock */
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
    DMA2_Stream7->CR &= ~DMA_SxCR_EN;
    while(DMA2_Stream7->CR & DMA_SxCR_EN);

    /** 2 & 3 Set peripheral and memory addresses address */
    DMA2_Stream7->PAR = (uint32_t)&USART1->DR;
    DMA2_Stream7->M0AR = (volatile uint32_t)uart_buf;

    /** 5 Select dma channel */
    // 100: channel 4 selected USART TX
    DMA2_Stream7->CR |= (DMA_SxCR_CHSEL_2);
    /** Increment memory pointer */
    DMA2_Stream7->CR |= DMA_SxCR_MINC;
    
    /** Set trasnfer from mem to peripheral */
    DMA2_Stream7->CR |= (DMA_SxCR_DIR_0);

    /** Enabel transfer complete interrupts */
    DMA2_Stream7->CR |= DMA_SxCR_TCIE;
    /** Enable Timer 2 interrupt in NVIC */
    NVIC_SetPriority(DMA2_Stream7_IRQn, 5);
    NVIC_EnableIRQ(TIM2_IRQn);

    /** enable dma */
    // DMA1_Stream7->CR |= DMA_SxCR_EN;

    // Use stream 5 for channel 4 USART RX
}

/*******************************************************************************
 * @brief Start a dma transfer of size bytes from uart_buf to USART1 DR
 * @param bytes number of bytes to transfer
 * @return void 
 */
void TransferData(uint16_t bytes)
{
    /* Wait until previous DMA transfer is no longer enabled */
    while ((DMA2_Stream7->CR & DMA_SxCR_EN) != 0U);
    

    DMA2_Stream7->NDTR = bytes;
    /** enable dma? */
    DMA2_Stream7->CR |= DMA_SxCR_EN;
}

/// @brief Sends data to uart, will convert to dma trasnfer in the future
///        DMA is channel 4: stream 5 RX and stream 7 TX for usart1
/// @param data pointer to data to be sent
/// @param size size of data to be sent
void UartTx(uint8_t * data, uint16_t size)
{
    memcpy(uart_buf, data, size);
    TransferData(size);
}