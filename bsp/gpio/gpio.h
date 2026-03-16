/*
 * gpio.c
 */
#ifndef __INCgpio_h
#define __INCgpio_h

#include "stm32f4xx.h"

#ifdef __cplusplus
extern "C" {
#endif /** __cplusplus */

/** Defines */
#define GPIO_INVALID_HANDLE ((gpio_handle_t)0xFFU)
#define GPIO_BASE           (AHB1PERIPH_BASE)

#define GPIO_HANDLE(port, pin) \
    ((((uint8_t)(port)) << 4U) | ((uint8_t)(pin) & 0x0FU))

#define GPIO_HANDLE_PORT(handle) \
    (((handle) >> 4U) & 0x0FU)

#define GPIO_HANDLE_PIN(handle) \
    ((handle) & 0x0FU)

#define GPIO_PORT_STRIDE      (0x400UL)

#define GPIO_PORT_PTR(port) \
    ((GPIO_TypeDef *)(GPIO_BASE + ((uint32_t)(port) * GPIO_PORT_STRIDE)))

#define GPIO_FROM_HANDLE(handle) GPIO_PORT_PTR(GPIO_HANDLE_PORT(handle))

#define GPIO_SET(handle) \
    (GPIO_FROM_HANDLE(handle)->BSRR = 1 << (GPIO_HANDLE_PIN(handle)))

#define GPIO_CLR(handle) \
    (GPIO_FROM_HANDLE(handle)->BSRR = 1 << (GPIO_HANDLE_PIN(handle) + 16U))

#define GPIO_READ(handle) \
    ((((GPIO_FROM_HANDLE(handle)->IDR & 1 << GPIO_HANDLE_PIN(handle))) != 0U) ? 1U : 0U)

#define GPIO_TOGGLE(handle)                                                \
    (GPIO_FROM_HANDLE(handle)->ODR ^= 1 << GPIO_HANDLE_PIN(handle))
/** Typedef, Enums, Classes */
typedef uint8_t gpio_handle_t;

typedef enum
{
    GPIO_PORT_A = 0,
    GPIO_PORT_B,
    GPIO_PORT_C,
    GPIO_PORT_D,
    GPIO_PORT_E,
    GPIO_PORT_F,
    GPIO_PORT_G,
    GPIO_PORT_H,
    GPIO_PORT_I
} gpio_port_t;

typedef enum
{
    GPIO_PIN_0  = 0,
    GPIO_PIN_1  = 1,
    GPIO_PIN_2  = 2,
    GPIO_PIN_3  = 3,
    GPIO_PIN_4  = 4,
    GPIO_PIN_5  = 5,
    GPIO_PIN_6  = 6,
    GPIO_PIN_7  = 7,
    GPIO_PIN_8  = 8,
    GPIO_PIN_9  = 9,
    GPIO_PIN_10 = 10,
    GPIO_PIN_11 = 11,
    GPIO_PIN_12 = 12,
    GPIO_PIN_13 = 13,
    GPIO_PIN_14 = 14,
    GPIO_PIN_15 = 15
} gpio_pin_t;

typedef enum
{
    GPIO_MODE_INPUT = 0,
    GPIO_MODE_OUTPUT,
    GPIO_MODE_ALT_FUNC,
    GPIO_MODE_ANALOG
} gpio_mode_t;

typedef enum
{
    GPIO_OUTPUT_PUSHPULL = 0,
    GPIO_OUTPUT_OPENDRAIN
} gpio_output_type_t;

typedef enum
{
    GPIO_PULL_NONE = 0,
    GPIO_PULL_UP,
    GPIO_PULL_DOWN
} gpio_pull_t;

typedef enum
{
    GPIO_SPEED_LOW = 0,
    GPIO_SPEED_MEDIUM,
    GPIO_SPEED_HIGH,
    GPIO_SPEED_VERY_HIGH
} gpio_speed_t;

typedef enum
{
    GPIO_STATE_LOW = 0,
    GPIO_STATE_HIGH
} gpio_state_t;

typedef struct
{
    gpio_port_t         port;
    gpio_pin_t          pin;
    gpio_mode_t         mode;
    gpio_output_type_t  output_type;
    gpio_pull_t         pull;
    gpio_speed_t        speed;
    uint8_t             alternate_function;
    gpio_state_t        initial_state;
} gpio_config_t;

/** Global/Static variables */

/** Functions */
gpio_handle_t gpio_init(const gpio_config_t * cfg);

#ifdef __cplusplus
}
#endif /** __cplusplus */
#endif /** __INCgpio_h */