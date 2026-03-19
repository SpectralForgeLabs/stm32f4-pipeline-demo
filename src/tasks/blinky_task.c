/*
 * blinky_task.c
 */

#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4xx.h"

#include "uart.h"
#include "spi.h"
#include "gpio.h"
#include "blinky_task.h"

#include <stdbool.h>
/** Defines */

/** Global/Static variables */
static uint8_t spi_rx_buf[256];
/** Functions */
void vTaskBlinky(void * varg)
{
    (void)varg; // Unused parameter
    
    // Quick sanity: enable GPIOD clock (F407 Discovery LED port)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    gpio_config_t led_cfg =
    {
        .port = GPIO_PORT_A,
        .pin = GPIO_PIN_6,
        .mode = GPIO_MODE_OUTPUT,
        .output_type = GPIO_OUTPUT_PUSHPULL,
        .pull = GPIO_PULL_NONE,
        .speed = GPIO_SPEED_LOW,
        .alternate_function = 0U,
        .initial_state = GPIO_STATE_LOW
    };
    gpio_handle_t led = gpio_init(&led_cfg);

    SpiInit();

    uint8_t by[4] = {0x9f, 0xff, 0xff, 0xff};
    uint8_t UNIQUE_ID[9] = {0xab, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    while(1)
    {
        // Delay 1 second
        vTaskDelay(pdMS_TO_TICKS(500));
        GPIO_TOGGLE(led);
        
        SpiTransfer(by, spi_rx_buf, sizeof(by), true);
        SpiTransfer(UNIQUE_ID, spi_rx_buf, sizeof(UNIQUE_ID), true);
    }
}
