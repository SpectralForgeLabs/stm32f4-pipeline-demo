/* stubs/gpio_stub.c */
#include "gpio.h"

int stub_gpio_led_state = 0;

void stub_gpio_set(gpio_handle_t handle)
{
    (void)handle;
    stub_gpio_led_state = 0;
}

void stub_gpio_clr(gpio_handle_t handle)
{
    (void)handle;
    stub_gpio_led_state = 1;
}