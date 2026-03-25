/* stubs/os_write_stub.h */
#ifndef __INCos_write_stub_h
#define __INCos_write_stub_h

#include <stdint.h>

#define OS_WRITE_STUB_BUF 256

typedef struct
{
    int      called;
    int      fd;
    uint16_t len;
    uint8_t  buf[OS_WRITE_STUB_BUF];
} os_write_capture_t;

extern os_write_capture_t os_write_last;

void os_write_stub_reset(void);

#endif /* __INCos_write_stub_h */