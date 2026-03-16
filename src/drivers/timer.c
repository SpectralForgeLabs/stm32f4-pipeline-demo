/*
 * timer.c
 */

#include "stm32f4xx.h"
#include "timer.h"
#include <stdlib.h>

#define timer2_irq TIM2_IRQHandler

static volatile uint32_t microseconds = 0;

void timer2_irq(void)
{
    if (TIM2->SR & TIM_SR_UIF) // Check if update interrupt flag is set
    {
        TIM2->SR &= ~TIM_SR_UIF; // Clear the update interrupt flag
    }
}

/// @brief 
/// @param frequency 
void Timer2Init(uint32_t frequency)
{
    /** Enable clock to Timer 2 */
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    /** Set prescaler and auto-reload for 1 second delay */
    TIM2->PSC = 16 - 1; // Prescaler for 16 MHz clock to get 1 us timer clock
    TIM2->ARR = 0xffffffff;  // Auto-reload for max count (~71 minutes)
    TIM2->EGR = TIM_EGR_UG;   // critical
    
    /** Set to count up mode */
    TIM2->CR1 &= ~TIM_CR1_DIR; // Count up mode

    /** Enable update interrupt */
    TIM2->DIER |= TIM_DIER_UIE;

    /** Start the timer */
    TIM2->CR1 |= TIM_CR1_CEN;

    /** Enable Timer 2 interrupt in NVIC */
    NVIC_SetPriority(TIM2_IRQn, 1);
    NVIC_EnableIRQ(TIM2_IRQn);
}

/// @brief 
/// @param  
void Timer2Stop(void)
{
    /** Stop the timer */
    TIM2->CR1 &= ~TIM_CR1_CEN;

    /** Disable update interrupt */
    TIM2->DIER &= ~TIM_DIER_UIE;

    /** Disable Timer 2 interrupt in NVIC */
    NVIC_DisableIRQ(TIM2_IRQn);
}

/// @brief 
/// @param  
void Timer2Reset(void)
{
    /** Reset the timer counter */
    TIM2->CNT = 0;
}