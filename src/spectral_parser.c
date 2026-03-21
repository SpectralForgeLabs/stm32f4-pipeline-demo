/* spectral_parser.c */
#include "buildinfo.h"
#include "uart.h"
#include "gpio.h"
#include "spectral_parser.h"
#include "blinky_task.h"

/** Defines */

/** Typedef, Enums, Classes */

/** Global/Static variables */

/** Functions */
static void cmd_ping(spectral_cmd_t *cmd);
static void cmd_led_on(spectral_cmd_t *cmd);
static void cmd_led_off(spectral_cmd_t *cmd);
static void cmd_status(spectral_cmd_t *cmd);
static void cmd_unknown(spectral_cmd_t *cmd);

void spectral_parse(uint8_t *data, uint8_t len)
{
    if (data == NULL || len == 0)
    {
        cmd_unknown(NULL);
        return;
    }

    spectral_cmd_t cmd =
    {
        .id          = (spectral_cmd_id_t)data[0],
        .payload     = len > 1 ? &data[1] : NULL,
        .payload_len = len > 1 ? len - 1  : 0,
        .valid       = false
    };

    switch(cmd.id)
    {
        case SPECTRAL_CMD_PING:     cmd.valid = true; cmd_ping(&cmd);     break;
        case SPECTRAL_CMD_LED_ON:   cmd.valid = true; cmd_led_on(&cmd);   break;
        case SPECTRAL_CMD_LED_OFF:  cmd.valid = true; cmd_led_off(&cmd);  break;
        case SPECTRAL_CMD_STATUS:   cmd.valid = true; cmd_status(&cmd);   break;
        default:                                      cmd_unknown(&cmd);  break;
    }
}

/** Handlers — private, defined right here */
static void cmd_ping(spectral_cmd_t *cmd)
{
    (void)cmd;
    uart_printf("SPECTRAL: PONG\n");
}

static void cmd_led_on(spectral_cmd_t *cmd)
{
    (void)cmd;
    GPIO_CLR(led);
    uart_printf("SPECTRAL: LED ON\n");
}

static void cmd_led_off(spectral_cmd_t *cmd)
{
    (void)cmd;
    GPIO_SET(led);
    uart_printf("SPECTRAL: LED OFF\n");
}

static void cmd_status(spectral_cmd_t *cmd)
{
    (void)cmd;
    uart_printf("SPECTRAL: STATUS OK\n");
    uart_printf("SPECTRAL: BUILD  %s\n", build_info.build_number);
    uart_printf("SPECTRAL: SHA    %s\n", build_info.git_sha);
    uart_printf("SPECTRAL: BRANCH %s\n", build_info.branch);
}

static void cmd_unknown(spectral_cmd_t *cmd)
{
    if (cmd)
        uart_printf("SPECTRAL: Unknown CMD 0x%02X\n", cmd->id);
    else
        uart_printf("SPECTRAL: Empty frame\n");
}