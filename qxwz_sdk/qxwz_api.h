#include <string.h>
#include <stdarg.h>
#include <stdio.h>

typedef void (*uart_cb)(char *, unsigned int len);
typedef void (*lcd_cb)(char *);

int qxwz_printf(const char *fmt, ...);

int start_uart(char* uart,char* file, uart_cb uart_rsp, lcd_cb lcd_rsp);

int stop_uart(void);

int write_to_uart(void* data,int len);
