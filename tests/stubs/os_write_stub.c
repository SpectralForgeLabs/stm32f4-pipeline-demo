/* stubs/os_write_stub.c */
#include "os_write_stub.h"
#include <string.h>

os_write_capture_t os_write_last = { 0 };

void os_write_stub_reset(void)
{
    memset(&os_write_last, 0, sizeof(os_write_last));
}

void os_write(int fd, uint8_t *buf, uint16_t len)
{
    os_write_last.called = 1;
    os_write_last.fd     = fd;
    os_write_last.len    = len < OS_WRITE_STUB_BUF ? len : OS_WRITE_STUB_BUF;
    memcpy(os_write_last.buf, buf, os_write_last.len);
}