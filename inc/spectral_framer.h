#ifndef __INCspectral_framer_h
#define __INCspectral_framer_h

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Defines */
#define FRAME_SOF        0xAA
#define FRAME_EOF        0xBB
#define FRAME_MAX_DATA   253
/* inter-byte timeout — one byte transfer time + delta */
#define BYTE_TIMEOUT_MS    1   /* 86us rounded up + margin */

/* full frame timeout — max frame size * byte time + delta */
#define FRAME_TIMEOUT_MS   30  /* 134 bytes * 86us + 20% margin */

/* framer doesn't know about FreeRTOS */
typedef uint32_t (*framer_get_time_fn)(void);

/** Frame error codes */
typedef enum
{
    FRAME_ERR_NONE,
    FRAME_ERR_INVALID_SOF,
    FRAME_ERR_INVALID_EOF,
    FRAME_ERR_OVERFLOW,
    FRAME_ERR_BAD_CRC,
    FRAME_ERR_BYTE_TIMEOUT,
    FRAME_ERR_FRAME_TIMEOUT,
} frame_error_t;

/** Framer state machine states */
typedef enum
{
    WAIT_SOF1,
    WAIT_SOF2,
    WAIT_LEN,
    RECV_DATA,
    WAIT_CRC,
    WAIT_EOF1,
    WAIT_EOF2,
    FRAME_DONE,
    FRAME_ERROR
} framer_state_t;

/** Caller owns this — framer writes into it */
typedef struct
{
    uint8_t data[FRAME_MAX_DATA];
    uint8_t len;
    uint8_t crc;
} spectral_frame_t;

/** Framer state machine — no data buffer */
typedef struct
{
    framer_state_t      state;
    frame_error_t       last_error;
    uint8_t             bytes_received;
    uint8_t             expected_len;
    uint8_t             crc_accum;
    uint32_t            sof_time;
    uint32_t            last_byte_time;
    framer_get_time_fn  get_time;
} framer_t;

/** API */
void           framer_init(framer_t * f, framer_get_time_fn get_time);
void           framer_reset(framer_t * f);
framer_state_t framer_process(framer_t * f, spectral_frame_t * frame, uint8_t byte);
uint8_t        framer_crc8(const uint8_t * data, uint8_t len);

#ifdef __cplusplus
}
#endif

#endif /* __INCspectral_framer_h */