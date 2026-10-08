/* Minimal newlib retarget: printf() -> USART1 (ST-LINK virtual COM port). */
#include <sys/stat.h>
#include <errno.h>
#include "main.h"

int _write(int fd, char *ptr, int len)
{
    (void)fd;
    if (HAL_UART_Transmit(&huart1, (uint8_t *)ptr, (uint16_t)len, HAL_MAX_DELAY) != HAL_OK) {
        errno = EIO;
        return -1;
    }
    return len;
}

int _read(int fd, char *ptr, int len)  { (void)fd; (void)ptr; (void)len; errno = ENOSYS; return -1; }
int _close(int fd)                     { (void)fd; return -1; }
int _isatty(int fd)                    { (void)fd; return 1; }
int _lseek(int fd, int off, int whence){ (void)fd; (void)off; (void)whence; return 0; }
int _fstat(int fd, struct stat *st)    { (void)fd; st->st_mode = S_IFCHR; return 0; }
int _getpid(void)                      { return 1; }
int _kill(int pid, int sig)            { (void)pid; (void)sig; errno = EINVAL; return -1; }
