#include <string.h>
#include <stdarg.h>
#include <stdio.h>

typedef void (*uart_cb)(char *, unsigned int len);

int qxwz_printf(const char *fmt, ...);

int start_uart(char* uart,char* file,uart_cb uart_rsp);

int stop_uart(void);

int write_to_uart(void* data,int len);
