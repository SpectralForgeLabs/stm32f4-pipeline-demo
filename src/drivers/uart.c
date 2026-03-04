/*
 * uart.c
 */

#include "stm32f4xx.h"

#include "uart.h"

/** Defines */
#define GPIO_AFHR_AFSEL9_7 (GPIO_AFRH_AFSEL9_0 | GPIO_AFRH_AFSEL9_1 | GPIO_AFRH_AFSEL9_2)
#define GPIO_AFHR_AFSEL10_7 (GPIO_AFRH_AFSEL10_0 | GPIO_AFRH_AFSEL10_1 | GPIO_AFRH_AFSEL10_2)
/** Global/Static variables */

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

    /* Start USART */
    USART1->CR1 |= USART_CR1_UE;
}

/// @brief Sends data to uart, will convert to dma trasnfer in the future
/// @param data pointer to data to be sent
/// @param size size of data to be sent
void UartTx(uint8_t * data, uint16_t size)
{
    for (uint16_t i = 0; i < size; i++)
    {
        while ((USART1->SR & USART_SR_TXE) != USART_SR_TXE) { }
        USART1->DR = data[i];
    }
}