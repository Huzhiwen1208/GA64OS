#include <stdio.h>

#define UART_BASE   0x9000000UL
#define UART_DR     (*(unsigned int *)(UART_BASE + 0x00))
#define UART_FR     (*(unsigned int *)(UART_BASE + 0x18))

static inline int uart_is_busy() {
    return UART_FR & (1 << 5);
}

void uart_putc(char c) {
    while (uart_is_busy());
    UART_DR = c;
}

void uart_puts(const char *content) {
    while (*content) {
        if (*content == '\n') {
            uart_putc('\r');
        }
        uart_putc(*content++);
    }
}