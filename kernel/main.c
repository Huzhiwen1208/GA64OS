#include <stdio.h>

void start_kernel() {
    uart_puts("Hello, world\n");
    init_printk_done();
    printk("Welcome to GA64OS!\n");
}