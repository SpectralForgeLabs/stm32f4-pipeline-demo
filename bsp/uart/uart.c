/*
 * uart.c
 */

#include "stm32f4xx.h"

#include "uart.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/** Defines */
#define GPIO_AFHR_AFSEL9_7 (GPIO_AFRH_AFSEL9_0 | GPIO_AFRH_AFSEL9_1 | GPIO_AFRH_AFSEL9_2)
#define GPIO_AFHR_AFSEL10_7 (GPIO_AFRH_AFSEL10_0 | GPIO_AFRH_AFSEL10_1 | GPIO_AFRH_AFSEL10_2)

#define dma7_isr DMA2_Stream7_IRQHandler
#define usart1_isr USART1_IRQHandler

#define UART_PRINTF_BUF_SIZE 256
/** Global/Static variables */
SemaphoreHandle_t uart_tx_sem = NULL;
QueueHandle_t uart_rx_queue = NULL;
uint16_t overrun_error_count = 0;
/** Functions */
void ConfigureUsartDma(void);

void usart1_isr(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (USART1->SR & USART_SR_RXNE) // Check if RXNE flag is set
    {
        char received_char = USART1->DR; // Read received data to clear RXNE flag
        // Handle received character (e.g., store in buffer, signal a task, etc.)
        xQueueSendFromISR(uart_rx_queue, &received_char, &xHigherPriorityTaskWoken);
    }

    if (USART1->SR & USART_SR_ORE)
    {
        /* clear overrun — read SR then DR */
        volatile uint32_t tmp = USART1->SR;
        tmp = USART1->DR;
        (void)tmp;
        /* optionally count overrun errors */
        overrun_error_count++;
    }
}
/*******************************************************************************
 * @brief DMA2_Stream7 interrupt handler
 */
void dma7_isr(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

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

        // Give semaphore back from ISR
        xSemaphoreGiveFromISR(uart_tx_sem, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

int uart_init(void)
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

    /** Enable DMA for USART1 */
    USART1->CR3 |= USART_CR3_DMAT;
    ConfigureUsartDma();

    // Create semaphore — start as available
    uart_tx_sem = xSemaphoreCreateBinary();
    if (uart_tx_sem == NULL) 
    {
        // Handle error: semaphore creation failed
        return ERROR;
    }
    xSemaphoreGive(uart_tx_sem);

    // Init rx queue
    uart_rx_queue = xQueueCreate(128, sizeof(char));
    if (uart_rx_queue == NULL)
    {
        // Handle error: queue creation failed
        return ERROR;
    }

    /** set rxne interrupt */
    USART1->CR1 |= USART_CR1_RXNEIE;
    NVIC_SetPriority(USART1_IRQn, 5);
    NVIC_EnableIRQ(USART1_IRQn);
    
    /* Start USART */
    USART1->CR1 |= USART_CR1_UE;

    return SUCCESS;
}

/*******************************************************************************
 * @brief Configures the DMA for USART1
 */
void ConfigureUsartDma(void)
{
    /** Enable periph clock */
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
    DMA2_Stream7->CR &= ~DMA_SxCR_EN;
    while(DMA2_Stream7->CR & DMA_SxCR_EN);

    /** 2 & 3 Set peripheral and memory addresses address */
    DMA2_Stream7->PAR = (uint32_t)&USART1->DR;

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
    NVIC_EnableIRQ(DMA2_Stream7_IRQn);
}


/*******************************************************************************
 * @brief Transmits data over UART using DMA
 * @param fmt Format string for printf-style output
 * @param ... Variable arguments for formatted output
 */
void uart_printf(const char *fmt, ...)
{
    static char buf[UART_PRINTF_BUF_SIZE];
    
    xSemaphoreTake(uart_tx_sem, portMAX_DELAY);
    
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, UART_PRINTF_BUF_SIZE, fmt, args);
    va_end(args);
    
    uint16_t len = strlen(buf);
    DMA2_Stream7->NDTR = len;
    DMA2_Stream7->M0AR = (uint32_t)buf;
    DMA2_Stream7->CR  |= DMA_SxCR_EN;

    /* semaphore given back in ISR */
}