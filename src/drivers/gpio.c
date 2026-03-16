/*
 * gpio.c
 */

#include <stddef.h>
#include <string.h>

#include "gpio.h"
/** Defines */

/** Typedef, Enums, Classes */

/** Global/Static variables */

/** Functions */

gpio_config_t led_cfg =
{
    .port = GPIO_PORT_A,
    .pin = GPIO_PIN_5,
    .mode = GPIO_MODE_OUTPUT,
    .output_type = GPIO_OUTPUT_PUSHPULL,
    .pull = GPIO_PULL_NONE,
    .speed = GPIO_SPEED_LOW,
    .alternate_function = 0U,
    .initial_state = GPIO_STATE_LOW
};

/*******************************************************************************
 * @brief Initializes a GPIO pin based on the provided configuration
 * @param cfg Pointer to a gpio_config_t structure 
 *            containing the desired pin configuration
 * @return 0 on success, non-zero on failure
 */
gpio_handle_t gpio_init(const gpio_config_t * cfg)
{
    GPIO_TypeDef * port;
    uint32_t pin;
    gpio_handle_t handle;

    if (cfg == NULL)
    {
        return GPIO_INVALID_HANDLE;
    }

    if (cfg->pin > 15U)
    {
        return GPIO_INVALID_HANDLE;
    }

    port = GPIO_PORT_PTR(cfg->port);
    pin = (uint32_t)cfg->pin;
    handle = GPIO_HANDLE(cfg->port, cfg->pin);

    port->MODER &= ~ (3U << (pin * 2U));
    port->MODER |= cfg->mode << (pin * 2U);

    port->OTYPER &= ~(1U << pin);
    port->OTYPER |= cfg->output_type << pin;

    port->PUPDR &= ~ (3U << (pin * 2U));
    port->PUPDR |= cfg->pull << (pin * 2U);

    port->OSPEEDR &= ~ (3U << (pin * 2U));
    port->OSPEEDR |= cfg->speed << (pin * 2U);
    if (cfg->mode == GPIO_MODE_ALT_FUNC)
    {
        if (pin < 8U)
        {
            port->AFR[0] &= ~(0xFU << (pin * 4U));
            port->AFR[0] |= cfg->alternate_function << (pin * 4U);
        }
        else
        {
            port->AFR[1] &= ~(0xFU << ((pin - 8U) * 4U));
            port->AFR[1] |= cfg->alternate_function << ((pin - 8U) * 4U);
        }
    }
    if (cfg->initial_state == GPIO_STATE_HIGH)
    {
        port->BSRR = (1UL << pin); // Set pin high
    }
    else
    {
        port->BSRR = (1UL << (pin + 16U)); // Set pin low
    }
    return handle;
}

// int gpio_write(gpio_handle_t pin, gpio_state_t state)
// {

// }
// int gpio_toggle(gpio_handle_t pin)
// {

// }
// gpio_state_t gpio_read(gpio_handle_t pin)
// {

// }