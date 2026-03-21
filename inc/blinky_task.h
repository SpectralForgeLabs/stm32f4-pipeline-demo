#ifndef __INCblinky_task_h
#define __INCblinky_task_h

#include "gpio.h"

#ifdef __cplusplus
extern "C" {
#endif /** __cplusplus */

extern gpio_handle_t led;

void vTaskBlinky(void *);

#ifdef __cplusplus
}
#endif /** __cplusplus */
#endif /** __INCblink_task_h */