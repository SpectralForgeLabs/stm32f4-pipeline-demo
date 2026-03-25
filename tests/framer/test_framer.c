/* tests/framer/test_framer.c */
#include "unity.h"
#include "spectral_framer.h"
#include <string.h>

/** Test fixtures */
static framer_t        framer;
static spectral_frame_t frame;

/** Stub time — controlled by tests */
static uint32_t stub_time = 0;

static uint32_t get_stub_time(void)
{
    return stub_time;
}

/** Helper — feed a byte array through framer, return final state */
static framer_state_t feed_bytes(const uint8_t *data, uint16_t len)
{
    framer_state_t state = WAIT_SOF1;
    for (uint16_t i = 0; i < len; i++)
        state = framer_process(&framer, &frame, data[i]);
    return state;
}

/** Helper — build a valid frame: AA AA [len] [payload...] [crc] BB BB */
static uint16_t build_frame(uint8_t *out, const uint8_t *payload, uint8_t pay_len)
{
    uint8_t crc = pay_len;
    uint16_t idx = 0;

    for (uint8_t i = 0; i < pay_len; i++)
        crc ^= payload[i];

    out[idx++] = 0xAA;
    out[idx++] = 0xAA;
    out[idx++] = pay_len;
    memcpy(&out[idx], payload, pay_len);
    idx += pay_len;
    out[idx++] = crc;
    out[idx++] = 0xBB;
    out[idx++] = 0xBB;

    return idx;
}

/*******************************************************************************
 * setUp / tearDown — run before/after every test
 */
void setUp(void)
{
    stub_time = 0;
    framer_init(&framer, get_stub_time);
    memset(&frame, 0, sizeof(frame));
}

void tearDown(void) {}

/*******************************************************************************
 * Tests
 */

/** Happy path — single byte payload */
void test_framer_valid_single_byte_payload(void)
{
    uint8_t payload[] = { 0x05 };
    uint8_t buf[16];
    uint16_t len = build_frame(buf, payload, sizeof(payload));

    framer_state_t state = feed_bytes(buf, len);

    TEST_ASSERT_EQUAL(FRAME_DONE, state);
    TEST_ASSERT_EQUAL(1, frame.len);
    TEST_ASSERT_EQUAL_UINT8(0x05, frame.data[0]);
    TEST_ASSERT_EQUAL(FRAME_ERR_NONE, framer.last_error);
}

/** Happy path — multi byte payload */
void test_framer_valid_multi_byte_payload(void)
{
    uint8_t payload[] = { 0x04, 0x00, 0x00, 0x96, 0x00 }; /* SET_BAUD 38400 */
    uint8_t buf[16];
    uint16_t len = build_frame(buf, payload, sizeof(payload));

    framer_state_t state = feed_bytes(buf, len);

    TEST_ASSERT_EQUAL(FRAME_DONE, state);
    TEST_ASSERT_EQUAL(5, frame.len);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(payload, frame.data, sizeof(payload));
}

/** Bad second SOF byte */
void test_framer_invalid_sof2(void)
{
    uint8_t buf[] = { 0xAA, 0x00, 0x01, 0x05, 0x04, 0xBB, 0xBB };
    framer_state_t state = feed_bytes(buf, sizeof(buf));

    TEST_ASSERT_EQUAL(FRAME_ERROR, state);
    TEST_ASSERT_EQUAL(FRAME_ERR_INVALID_SOF, framer.last_error);
}

/** Zero length — rejected at WAIT_LEN */
void test_framer_zero_length(void)
{
    uint8_t buf[] = { 0xAA, 0xAA, 0x00 };
    framer_state_t state = feed_bytes(buf, sizeof(buf));

    TEST_ASSERT_EQUAL(FRAME_ERROR, state);
    TEST_ASSERT_EQUAL(FRAME_ERR_OVERFLOW, framer.last_error);
}

/** Length > FRAME_MAX_DATA — overflow */
void test_framer_overflow(void)
{
    uint8_t buf[] = { 0xAA, 0xAA, 0xFF };
    framer_state_t state = feed_bytes(buf, sizeof(buf));

    TEST_ASSERT_EQUAL(FRAME_ERROR, state);
    TEST_ASSERT_EQUAL(FRAME_ERR_OVERFLOW, framer.last_error);
}

/** Bad CRC */
void test_framer_bad_crc(void)
{
    uint8_t payload[] = { 0x05 };
    uint8_t buf[16];
    uint16_t len = build_frame(buf, payload, sizeof(payload));

    /* corrupt the CRC byte — it sits right before BB BB */
    buf[len - 3] ^= 0xFF;

    framer_state_t state = feed_bytes(buf, len);

    TEST_ASSERT_EQUAL(FRAME_ERROR, state);
    TEST_ASSERT_EQUAL(FRAME_ERR_BAD_CRC, framer.last_error);
}

/** Bad EOF1 */
void test_framer_invalid_eof1(void)
{
    uint8_t payload[] = { 0x05 };
    uint8_t buf[16];
    uint16_t len = build_frame(buf, payload, sizeof(payload));

    /* corrupt first EOF byte */
    buf[len - 2] = 0x00;

    framer_state_t state = feed_bytes(buf, len);

    TEST_ASSERT_EQUAL(FRAME_ERROR, state);
    TEST_ASSERT_EQUAL(FRAME_ERR_INVALID_EOF, framer.last_error);
}

/** Bad EOF2 */
void test_framer_invalid_eof2(void)
{
    uint8_t payload[] = { 0x05 };
    uint8_t buf[16];
    uint16_t len = build_frame(buf, payload, sizeof(payload));

    /* corrupt second EOF byte */
    buf[len - 1] = 0x00;

    framer_state_t state = feed_bytes(buf, len);

    TEST_ASSERT_EQUAL(FRAME_ERROR, state);
    TEST_ASSERT_EQUAL(FRAME_ERR_INVALID_EOF, framer.last_error);
}

/** Byte timeout — gap between bytes exceeds BYTE_TIMEOUT_MS */
void test_framer_byte_timeout(void)
{
    /* Feed SOF1 and SOF2 at time 0 */
    framer_process(&framer, &frame, 0xAA);
    framer_process(&framer, &frame, 0xAA);

    /* Advance time past byte timeout */
    stub_time = BYTE_TIMEOUT_MS + 1;

    framer_state_t state = framer_process(&framer, &frame, 0x01);

    TEST_ASSERT_EQUAL(FRAME_ERROR, state);
    TEST_ASSERT_EQUAL(FRAME_ERR_BYTE_TIMEOUT, framer.last_error);
}

/** Frame timeout — total frame time exceeds FRAME_TIMEOUT_MS */
void test_framer_frame_timeout(void)
{
    /* Feed SOF + LEN at tick 0 */
    framer_process(&framer, &frame, 0xAA);
    framer_process(&framer, &frame, 0xAA);
    framer_process(&framer, &frame, 0xFD); /* LEN = 5 */

    /* Feed data bytes one tick at a time — byte timeout never fires
       but total frame time accumulates past FRAME_TIMEOUT_MS */
    for (uint32_t i = 1; i <= FRAME_TIMEOUT_MS + 1; i++)
    {
        stub_time = i;
        framer_state_t state = framer_process(&framer, &frame, 0x00);
        if (state == FRAME_ERROR)
        {
            TEST_ASSERT_EQUAL(FRAME_ERR_FRAME_TIMEOUT, framer.last_error);
            return;
        }
    }

    TEST_FAIL_MESSAGE("Frame timeout never triggered");
}

/** Reset restores to initial state */
void test_framer_reset(void)
{
    /* Partially feed a frame */
    framer_process(&framer, &frame, 0xAA);
    framer_process(&framer, &frame, 0xAA);
    framer_process(&framer, &frame, 0x01);

    framer_reset(&framer);

    TEST_ASSERT_EQUAL(WAIT_SOF1, framer.state);
    TEST_ASSERT_EQUAL(FRAME_ERR_NONE, framer.last_error);
    TEST_ASSERT_EQUAL(0, framer.bytes_received);
}

/** framer_crc8 — known values */
void test_framer_crc8_known_value(void)
{
    /* CRC of LEN(1) ^ DATA(0x05) = 0x04 — matches PING frame spec */
    uint8_t data[] = { 0x01, 0x05 };
    TEST_ASSERT_EQUAL_UINT8(0x04, framer_crc8(data, sizeof(data)));
}

/** Noise before valid frame — framer re-syncs */
void test_framer_noise_before_valid_frame(void)
{
    uint8_t payload[] = { 0x01 };
    uint8_t frame_buf[16];
    uint16_t frame_len = build_frame(frame_buf, payload, sizeof(payload));

    /* Prepend garbage bytes */
    uint8_t noise[] = { 0x00, 0x11, 0xFF, 0x55 };
    for (uint16_t i = 0; i < sizeof(noise); i++)
        framer_process(&framer, &frame, noise[i]);

    framer_state_t state = feed_bytes(frame_buf, frame_len);

    TEST_ASSERT_EQUAL(FRAME_DONE, state);
    TEST_ASSERT_EQUAL(1, frame.len);
    TEST_ASSERT_EQUAL_UINT8(0x01, frame.data[0]);
}

/*******************************************************************************
 * main
 */
int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_framer_valid_single_byte_payload);
    RUN_TEST(test_framer_valid_multi_byte_payload);
    RUN_TEST(test_framer_invalid_sof2);
    RUN_TEST(test_framer_zero_length);
    RUN_TEST(test_framer_overflow);
    RUN_TEST(test_framer_bad_crc);
    RUN_TEST(test_framer_invalid_eof1);
    RUN_TEST(test_framer_invalid_eof2);
    RUN_TEST(test_framer_byte_timeout);
    RUN_TEST(test_framer_frame_timeout);
    RUN_TEST(test_framer_reset);
    RUN_TEST(test_framer_crc8_known_value);
    RUN_TEST(test_framer_noise_before_valid_frame);

    return UNITY_END();
}