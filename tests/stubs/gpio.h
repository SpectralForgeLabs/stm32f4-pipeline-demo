/*
 * stubs/gpio.h
 * Shadows bsp/gpio/gpio.h for host unit test builds.
 * Replaces hardware macro chain with state-tracking stubs.
 */
#ifndef __INCgpio_h
#define __INCgpio_h

#include <stdint.h>

/** Minimal types to satisfy parser */
typedef uint8_t gpio_handle_t;

/** Track LED state for assertions */
extern int stub_gpio_led_state;  /* 1 = on (CLR), 0 = off (SET) */

void stub_gpio_set(gpio_handle_t handle);
void stub_gpio_clr(gpio_handle_t handle);

/** Override hardware macros */
#define GPIO_SET(handle)    stub_gpio_set(handle)
#define GPIO_CLR(handle)    stub_gpio_clr(handle)
#define GPIO_READ(handle)   (0)
#define GPIO_TOGGLE(handle) do {} while(0)

#endif /* __INCgpio_h */