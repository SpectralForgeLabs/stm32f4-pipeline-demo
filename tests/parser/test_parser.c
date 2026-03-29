/* tests/parser/test_parser.c */
#include "unity.h"
#include "spectral_parser.h"
#include "os_write_stub.h"
#include "gpio.h"
#include <string.h>

/* Satisfy linker — led is defined in blinky_task.c in production */
gpio_handle_t led = 0;

/*******************************************************************************
 * setUp / tearDown
 */
void setUp(void)
{
    os_write_stub_reset();
}

void tearDown(void) {}

/*******************************************************************************
 * Helpers
 */

/** Verify response frame structure: AA AA [len] [payload...] [crc] BB BB */
static void assert_valid_response_frame(uint8_t *buf, uint16_t total_len,
                                         uint8_t expected_cmd)
{
    TEST_ASSERT_GREATER_OR_EQUAL(7, total_len);         /* min frame size */
    TEST_ASSERT_EQUAL_UINT8(0xAA, buf[0]);              /* SOF1 */
    TEST_ASSERT_EQUAL_UINT8(0xAA, buf[1]);              /* SOF2 */

    uint8_t pay_len = buf[2];
    TEST_ASSERT_EQUAL_UINT8(expected_cmd, buf[3]);      /* cmd id first payload byte */

    /* Verify CRC: XOR of LEN + all payload bytes */
    uint8_t crc = pay_len;
    for (uint8_t i = 0; i < pay_len; i++)
        crc ^= buf[3 + i];
    TEST_ASSERT_EQUAL_UINT8(crc, buf[3 + pay_len]);     /* CRC byte */

    TEST_ASSERT_EQUAL_UINT8(0xBB, buf[3 + pay_len + 1]); /* EOF1 */
    TEST_ASSERT_EQUAL_UINT8(0xBB, buf[3 + pay_len + 2]); /* EOF2 */
}

/*******************************************************************************
 * Tests
 */

/** PING — dispatches, os_write called, response contains PING cmd id */
void test_parser_ping(void)
{
    uint8_t data[] = { SPECTRAL_CMD_PING };
    spectral_parse(data, sizeof(data));

    TEST_ASSERT_EQUAL(1, os_write_last.called);
    assert_valid_response_frame(os_write_last.buf, os_write_last.len,
                                 SPECTRAL_CMD_PING);
}

/** LED ON — dispatches, GPIO set, response ACK */
void test_parser_led_on(void)
{
    uint8_t data[] = { SPECTRAL_CMD_LED_ON };
    spectral_parse(data, sizeof(data));

    TEST_ASSERT_EQUAL(1, os_write_last.called);
    assert_valid_response_frame(os_write_last.buf, os_write_last.len,
                                 SPECTRAL_CMD_LED_ON);
}

/** LED OFF — dispatches, GPIO set, response ACK */
void test_parser_led_off(void)
{
    uint8_t data[] = { SPECTRAL_CMD_LED_OFF };
    spectral_parse(data, sizeof(data));

    TEST_ASSERT_EQUAL(1, os_write_last.called);
    assert_valid_response_frame(os_write_last.buf, os_write_last.len,
                                 SPECTRAL_CMD_LED_OFF);
}

/** STATUS — dispatches, response contains STATUS cmd id and build info */
void test_parser_status(void)
{
    uint8_t data[] = { SPECTRAL_CMD_STATUS };
    spectral_parse(data, sizeof(data));

    TEST_ASSERT_EQUAL(1, os_write_last.called);
    assert_valid_response_frame(os_write_last.buf, os_write_last.len,
                                 SPECTRAL_CMD_STATUS);

    /* Build info should appear in payload — stub values are known */
    char *payload_str = (char *)&os_write_last.buf[3];
    TEST_ASSERT_NOT_NULL(strstr(payload_str, "test-branch"));
    TEST_ASSERT_NOT_NULL(strstr(payload_str, "42"));
    TEST_ASSERT_NOT_NULL(strstr(payload_str, "beeffeed"));
}

/** Unknown command — responds with 0xFF */
void test_parser_unknown_cmd(void)
{
    uint8_t data[] = { 0xDE };
    spectral_parse(data, sizeof(data));

    TEST_ASSERT_EQUAL(1, os_write_last.called);
    assert_valid_response_frame(os_write_last.buf, os_write_last.len,
                                 SPECTRAL_CMD_UNKNOWN);
}

/** NULL data — os_write still called with unknown response */
void test_parser_null_data(void)
{
    spectral_parse(NULL, 1);

    TEST_ASSERT_EQUAL(1, os_write_last.called);
    assert_valid_response_frame(os_write_last.buf, os_write_last.len,
                                 SPECTRAL_CMD_UNKNOWN);
}

/** Zero length — os_write still called with unknown response */
void test_parser_zero_len(void)
{
    uint8_t data[] = { SPECTRAL_CMD_PING };
    spectral_parse(data, 0);

    TEST_ASSERT_EQUAL(1, os_write_last.called);
    assert_valid_response_frame(os_write_last.buf, os_write_last.len,
                                 SPECTRAL_CMD_UNKNOWN);
}

/** SET_BAUD — not implemented yet, falls through to unknown */
void test_parser_set_baud_unknown(void)
{
    uint8_t data[] = { SPECTRAL_CMD_SET_BAUD, 0x00, 0x00, 0x96, 0x00 };
    spectral_parse(data, sizeof(data));

    TEST_ASSERT_EQUAL(1, os_write_last.called);
    assert_valid_response_frame(os_write_last.buf, os_write_last.len,
                                 SPECTRAL_CMD_UNKNOWN);
}

/** os_write fd is correct */
void test_parser_ping_fd(void)
{
    uint8_t data[] = { SPECTRAL_CMD_PING };
    spectral_parse(data, sizeof(data));

    TEST_ASSERT_EQUAL(1, os_write_last.fd);  /* OS_TX_FD_UART1 = 1 */
}

/*******************************************************************************
 * main
 */
int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_parser_ping);
    RUN_TEST(test_parser_led_on);
    RUN_TEST(test_parser_led_off);
    RUN_TEST(test_parser_status);
    RUN_TEST(test_parser_unknown_cmd);
    RUN_TEST(test_parser_null_data);
    RUN_TEST(test_parser_zero_len);
    RUN_TEST(test_parser_set_baud_unknown);
    RUN_TEST(test_parser_ping_fd);

    return UNITY_END();
}