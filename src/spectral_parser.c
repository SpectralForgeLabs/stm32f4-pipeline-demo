/* spectral_parser.c */
#include "buildinfo.h"
#include "uart.h"
#include "gpio.h"
#include "spectral_parser.h"
#include "blinky_task.h"
#include "uart_task.h"

#include <stdio.h>
#include <string.h>

/** Defines */
#define SPECTRAL_RESPONSE_BUF_SIZE  64

/** Typedef, Enums, Classes */

/** Global/Static variables */

/** Functions */
static void cmd_ping(spectral_cmd_t *cmd);
static void cmd_led_on(spectral_cmd_t *cmd);
static void cmd_led_off(spectral_cmd_t *cmd);
static void cmd_status(spectral_cmd_t *cmd);
static void cmd_unknown(spectral_cmd_t *cmd);

/*******************************************************************************
 * @brief Helper — builds a Spectral frame and sends via os_write
 * @param fd       output file descriptor
 * @param payload  response payload bytes
 * @param pay_len  payload length
 */
static void spectral_respond(int fd, uint8_t *payload, uint8_t pay_len)
{
    uint8_t frame[SPECTRAL_RESPONSE_BUF_SIZE];
    uint8_t crc = pay_len;
    uint8_t idx = 0;
 
    for (uint8_t i = 0; i < pay_len; i++)
        crc ^= payload[i];
 
    frame[idx++] = 0xAA;
    frame[idx++] = 0xAA;
    frame[idx++] = pay_len;
    memcpy(&frame[idx], payload, pay_len);
    idx += pay_len;
    frame[idx++] = crc;
    frame[idx++] = 0xBB;
    frame[idx++] = 0xBB;
 
    os_write(fd, frame, idx);
}
 
/*******************************************************************************
 * @brief Parse and dispatch incoming Spectral frame payload
 * @param data  frame payload (first byte = command id)
 * @param len   payload length
 */
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

/*******************************************************************************
 * @brief PING — respond with PONG (0x05 echo)
 * Frame: AA AA 01 05 04 BB BB
 */
static void cmd_ping(spectral_cmd_t *cmd)
{
    (void)cmd;
    uint8_t payload[] = { SPECTRAL_CMD_PING };
    spectral_respond(OS_TX_FD_UART1, payload, sizeof(payload));
}
 
/*******************************************************************************
 * @brief LED ON — turn LED on, respond with ACK (0x01)
 * Frame: AA AA 01 01 00 BB BB
 */
static void cmd_led_on(spectral_cmd_t *cmd)
{
    (void)cmd;
    GPIO_CLR(led);
    uint8_t payload[] = { SPECTRAL_CMD_LED_ON };
    spectral_respond(OS_TX_FD_UART1, payload, sizeof(payload));
}
 
/*******************************************************************************
 * @brief LED OFF — turn LED off, respond with ACK (0x02)
 * Frame: AA AA 01 02 03 BB BB
 */
static void cmd_led_off(spectral_cmd_t *cmd)
{
    (void)cmd;
    GPIO_SET(led);
    uint8_t payload[] = { SPECTRAL_CMD_LED_OFF };
    spectral_respond(OS_TX_FD_UART1, payload, sizeof(payload));
}
 
/*******************************************************************************
 * @brief STATUS — respond with build info as ASCII payload
 */
static void cmd_status(spectral_cmd_t *cmd)
{
    (void)cmd;
    uint8_t payload[128];
    uint8_t len = 0;
 
    /* Cmd ID first byte */
    payload[len++] = SPECTRAL_CMD_STATUS;
 
    /* Pack build info as null-terminated string in payload */
    len += snprintf((char *)&payload[len], sizeof(payload) - len,
                    "%s %s %s",
                    build_info.branch,
                    build_info.build_number,
                    build_info.git_sha);
 
    spectral_respond(OS_TX_FD_UART1, payload, len);
}
 
/*******************************************************************************
 * @brief Unknown command — respond with 0xFF
 */
static void cmd_unknown(spectral_cmd_t *cmd)
{
    (void)cmd;
    uint8_t payload[] = { SPECTRAL_CMD_UNKNOWN };
    spectral_respond(OS_TX_FD_UART1, payload, sizeof(payload));
}