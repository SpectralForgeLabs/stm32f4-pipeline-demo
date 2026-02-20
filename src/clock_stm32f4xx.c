/*
 * uart.c
 */

#include "stm32f4xx.h"
#include "clock_stm32f4xx.h"

/** Defines */

/** Global/Static variables */

/** Functions */

/// @brief 
/// @param  
void InitSysClock(void)
{
    // SetClockDivisors();
    EnablePortClocks();
}

/// @brief 
/// @param  
void EnablePortClocks(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN |
                    RCC_AHB1ENR_GPIOBEN |
                    RCC_AHB1ENR_GPIOCEN |
                    RCC_AHB1ENR_GPIODEN |
                    RCC_AHB1ENR_GPIOEEN;
}

/// @brief 
/// @param  
void SetClockDivisors(void)
{
    /** AHB clock not divided going into APB, Move to own routine */
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV1;
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1;
}