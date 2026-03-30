/* stubs/gpio_stub.h */
#ifndef __INCgpio_stub_h
#define __INCgpio_stub_h

extern int stub_gpio_led_state;

void stub_gpio_set(void);
void stub_gpio_clr(void);

/* Override hardware macros */
#define GPIO_SET(x) stub_gpio_set()
#define GPIO_CLR(x) stub_gpio_clr()

#endif /* __INCgpio_stub_h */