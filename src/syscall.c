#include <sys/stat.h>
#include <errno.h>
#include <stdint.h>

extern char _end;   // from linker

static char *heap_end;

/// @brief 
/// @param incr 
/// @return 
void *_sbrk(ptrdiff_t incr)
{
    if (!heap_end) heap_end = &_end;

    char *prev = heap_end;
    char *next = heap_end + incr;

    register char *sp __asm("sp");
    if (next > sp)
    {
        errno = ENOMEM;
        return (void *)-1;
    }

    heap_end = next;
    return prev;
}

int _write(int file, char *ptr, int len) { (void)file; (void)ptr; return len; }
int _read (int file, char *ptr, int len) { (void)file; (void)ptr; (void)len; return 0; }
int _close(int file) { (void)file; return -1; }
int _lseek(int file, int ptr, int dir) { (void)file; (void)ptr; (void)dir; return 0; }
int _isatty(int file) { (void)file; return 1; }
int _fstat(int file, struct stat *st) { (void)file; st->st_mode = S_IFCHR; return 0; }

void _exit(int status) { (void)status; while (1) {} }
int _kill(int pid, int sig) { (void)pid; (void)sig; errno = EINVAL; return -1; }
int _getpid(void) { return 1; }
