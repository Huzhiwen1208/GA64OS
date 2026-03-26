#ifndef GKERNEL_STDIO_H
#define GKERNEL_STDIO_H

void uart_puts(const char *content);
void uart_putc(char c);
unsigned int strlen(const char *s);
void memcpy(void *dest, const void *src, unsigned int n);
int printk(const char *fmt, ...);
void init_printk_done();

#endif