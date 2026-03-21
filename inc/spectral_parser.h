/* spectral_parser.h */
#ifndef __INCspectral_parser_h
#define __INCspectral_parser_h

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Defines */

/** Typedef, Enums, Classes */

/** 
 * SpectralForge Command Protocol v1.0 
 * Command IDs — first byte of payload
 */
typedef enum {
    SPECTRAL_CMD_LED_ON      = 0x01,
    SPECTRAL_CMD_LED_OFF     = 0x02,
    SPECTRAL_CMD_STATUS      = 0x03,
    SPECTRAL_CMD_SET_BAUD    = 0x04,
    SPECTRAL_CMD_PING        = 0x05,
    SPECTRAL_CMD_UNKNOWN     = 0xFF,
} spectral_cmd_id_t;

/** Parsed command */
typedef struct {
    spectral_cmd_id_t  id;
    uint8_t           *payload;
    uint8_t            payload_len;
    bool               valid;
} spectral_cmd_t;

/** Global/Static variables */

/** Functions */

void spectral_parse(uint8_t *data, uint8_t len);

#ifdef __cplusplus
}
#endif

#endif /* __INCspectral_parser_h */