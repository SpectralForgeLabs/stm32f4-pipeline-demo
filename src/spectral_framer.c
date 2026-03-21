#include "spectral_framer.h"
#include <string.h>

/** Defines */

/** Typedef, Enums, Classes */

/** Global/Static variables */

/** Functions */
uint32_t default_get_time(void)
{
    return 0;
}
/*******************************************************************************
 * @brief CRC8 XOR accumulation
 */
uint8_t framer_crc8(const uint8_t * data, uint8_t len)
{
    uint8_t crc = 0x00;
    for (uint8_t i = 0; i < len; i++)
    {
        crc ^= data[i];
    }
    return crc;
}

/*******************************************************************************
 * @brief Initialize framer
 */
void framer_init(framer_t * f, framer_get_time_fn get_time)
{
    memset(f, 0, sizeof(framer_t));
    f->state      = WAIT_SOF1;
    f->last_error = FRAME_ERR_NONE;
    f->get_time   = (get_time != NULL) ? get_time : default_get_time;
}

/*******************************************************************************
 * @brief Reset framer back to initial state
 */
void framer_reset(framer_t *f)
{
    framer_init(f, f->get_time);
}

/*******************************************************************************
 * @brief Process one byte through state machine
 *        Writes into caller owned frame buffer
 * @return current state
 */
framer_state_t framer_process(framer_t * f, spectral_frame_t * frame, uint8_t byte)
{

    uint32_t now = f->get_time();

    /* check timeouts first */
    if (f->state != WAIT_SOF1)
    {
        if ((now - f->last_byte_time) > BYTE_TIMEOUT_MS)
        {
            f->last_error = FRAME_ERR_BYTE_TIMEOUT;
            f->state      = FRAME_ERROR;
            return f->state;
        }

        if ((now - f->sof_time) > FRAME_TIMEOUT_MS)
        {
            f->last_error = FRAME_ERR_FRAME_TIMEOUT;
            f->state      = FRAME_ERROR;
            return f->state;
        }
    }

    f->last_byte_time = now;

    switch (f->state)
    {
        case WAIT_SOF1:
            if (byte == FRAME_SOF)
            {
                f->state = WAIT_SOF2;
                f->sof_time = now;
            }
            /* ignore noise — stay hunting */
            break;

        case WAIT_SOF2:
            if (byte == FRAME_SOF)
            {
                f->state = WAIT_LEN;
            }
            else 
            {
                f->last_error = FRAME_ERR_INVALID_SOF;
                f->state      = FRAME_ERROR;
            }
            break;

        case WAIT_LEN:
            if (byte == 0 || byte > FRAME_MAX_DATA)
            {
                f->last_error = FRAME_ERR_OVERFLOW;
                f->state      = FRAME_ERROR;
            }
            else 
            {
                frame->len        = byte;
                f->expected_len   = byte;
                f->bytes_received = 0;
                f->crc_accum      = 0;
                f->state          = RECV_DATA;
            }
            break;

        case RECV_DATA:
            frame->data[f->bytes_received++] = byte;
            f->crc_accum ^= byte;

            if (f->bytes_received >= f->expected_len)
            {
                f->state = WAIT_CRC;
            }
            break;

        case WAIT_CRC:
            frame->crc = byte;
            if (byte != f->crc_accum)
            {
                f->last_error = FRAME_ERR_BAD_CRC;
                f->state      = FRAME_ERROR;
            }
            else
            {
                f->last_error = FRAME_ERR_NONE;
                f->state      = WAIT_EOF1;
            }
            break;

        case WAIT_EOF1:
            if (byte == FRAME_EOF)
            {
                f->state = WAIT_EOF2;
            }
            else 
            {
                f->last_error = FRAME_ERR_INVALID_EOF;
                f->state      = FRAME_ERROR;
            }
            break;

        case WAIT_EOF2:
            if (byte == FRAME_EOF)
            {
                f->last_error = FRAME_ERR_NONE;
                f->state      = FRAME_DONE;
            } 
            else
            {
                f->last_error = FRAME_ERR_INVALID_EOF;
                f->state      = FRAME_ERROR;
            }
            break;

        case FRAME_DONE:
        case FRAME_ERROR:
            break;
    }

    return f->state;
}