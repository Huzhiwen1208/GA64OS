#include <stdio.h>
#include <stdarg.h>

#define CONSOLE_PRINT_BUFFER_SIZE 1024
static char print_buffer[CONSOLE_PRINT_BUFFER_SIZE];

#define CONFIG_LOG_BUF_SHIFT 17
#define LOG_BUF_LEN (1 << CONFIG_LOG_BUF_SHIFT)
static char log_buf[LOG_BUF_LEN];

enum printk_status {
    PRINTK_STATUS_DOWN,
    PRINTK_STATUS_READY,
};

static enum printk_status g_printk_status = PRINTK_STATUS_DOWN;
static char *g_record = log_buf;
static unsigned long g_record_len = 0;

#define ZEROPAD 1
#define SIGN 2
#define PLUS 4
#define SPACE 8
#define LEFT 16
#define SPECIAL 32
#define SMALL 64

#define is_digit(c) ((c) >= '0' && (c) <= '9')

#define do_div(n, base) ({ \
    unsigned long __base = (base); \
    unsigned long __rem; \
    __rem = ((unsigned long)(n)) % __base; \
    (n) = ((unsigned long)(n)) / __base; \
    __rem; \
})

static const char *scan_number(const char *string, int *number) {
    int result = 0;
    while (is_digit(*string)) {
        result = result * 10 + *string - '0';
        string++;
    }
    *number = result;
    return string;
}

static char *number(char *str, unsigned long num, int base, int size, int precision, int type) {
    char c, sign, tmp[66];
    const char *digits;
    int i;

    digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    if (type & SMALL) {
        digits = "0123456789abcdefghijklmnopqrstuvwxyz";
    }
    if (type & LEFT) {
        type &= ~ZEROPAD;
    }
    c = (type & ZEROPAD) ? '0' : ' ';
    sign = 0;
    if (type & SIGN) {
        if ((long)num < 0) {
            sign = '-';
            num = -(long)num;
        } else if (type & PLUS) {
            sign = '+';
        } else if (type & SPACE) {
            sign = ' ';
        }
    }
    if (type & SPECIAL) {
        if (base == 16) {
            size -= 2;
        } else if (base == 8) {
            size -= 1;
        }
    }
    i = 0;
    if (num == 0) {
        tmp[i++] = '0';
    } else {
        while (num != 0) {
            tmp[i++] = digits[do_div(num, base)];
        }
    }
    if (i > precision) {
        precision = i;
    }
    size -= precision;
    if (!(type & (ZEROPAD | LEFT))) {
        while (size-- > 0) {
            *str++ = ' ';
        }
    }
    if (sign) {
        *str++ = sign;
    }
    if (type & SPECIAL) {
        if (base == 8) {
            *str++ = '0';
        } else if (base == 16) {
            *str++ = '0';
            *str++ = digits[33];
        }
    }
    if (!(type & LEFT)) {
        while (size-- > 0) {
            *str++ = c;
        }
    }
    while (i < precision--) {
        *str++ = '0';
    }
    while (i-- > 0) {
        *str++ = tmp[i];
    }
    while (size-- > 0) {
        *str++ = ' ';
    }
    return str;
}

int my_printf(char *string, unsigned int size, const char *fmt, va_list arg) {
    int flags, field_width, precision, qualifier;
    char *str;
    const char *s;

    for (str = string; *fmt; fmt++) {
        if (*fmt != '%') {
            *str++ = *fmt;
            continue;
        }
        flags = 0;
    repeat:
        fmt++;
        switch (*fmt) {
        case '-':
            flags |= LEFT;
            goto repeat;
        case '+':
            flags |= PLUS;
            goto repeat;
        case ' ':
            flags |= SPACE;
            goto repeat;
        case '#':
            flags |= SPECIAL;
            goto repeat;
        case '0':
            flags |= ZEROPAD;
            goto repeat;
        }
        field_width = -1;
        if (is_digit(*fmt)) {
            fmt = scan_number(fmt, &field_width);
        } else if (*fmt == '*') {
            fmt++;
            field_width = va_arg(arg, int);
            if (field_width < 0) {
                field_width = -field_width;
                flags |= LEFT;
            }
        }
        precision = -1;
        if (*fmt == '.') {
            fmt++;
            if (is_digit(*fmt)) {
                fmt = scan_number(fmt, &precision);
            } else if (*fmt == '*') {
                fmt++;
                precision = va_arg(arg, int);
            }
        }
        qualifier = 0;
        if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L') {
            qualifier = *fmt;
            fmt++;
        }
        switch (*fmt) {
        case 'c': {
            if (!(flags & LEFT)) {
                while (--field_width > 0) {
                    *str++ = ' ';
                }
            }
            *str++ = (unsigned char)va_arg(arg, int);
            while (--field_width > 0) {
                *str++ = ' ';
            }
            break;
        }
        case 's': {
            s = va_arg(arg, char *);
            if (!s) {
                s = "<NULL>";
            }
            int len = strlen(s);
            if (precision >= 0 && len > precision) {
                len = precision;
            }
            if (!(flags & LEFT)) {
                while (len < field_width--) {
                    *str++ = ' ';
                }
            }
            for (int i = 0; i < len; i++) {
                *str++ = *s++;
            }
            while (len < field_width--) {
                *str++ = ' ';
            }
            break;
        }
        case 'p': {
            if (field_width == -1) {
                field_width = 2 * sizeof(void *);
                flags |= ZEROPAD;
            }
            str = number(str, (unsigned long)va_arg(arg, void *), 16
                , field_width, precision, flags);
            break;
        }
        case 'n': {
            if (qualifier == 'l') {
                long *ip = va_arg(arg, long *);
                *ip = (str - string);
            } else {
                int *ip = va_arg(arg, int *);
                *ip = (str - string);
            }
            break;
        }
        case '%': {
            *str++ = '%';
            break;
        }
        case 'o': {
            str = number(str, va_arg(arg, unsigned long), 8, field_width, precision, flags);
            break;
        }
        case 'X':
            flags |= SMALL;
        case 'x': {
            str = number(str, va_arg(arg, unsigned long), 16, field_width,
                precision, flags);
            break;
        }
        case 'd':
        case 'i': {
            flags |= SIGN;
            str = number(str, va_arg(arg, unsigned long), 10, field_width,
                precision, flags);
            break;
        }
        case 'u': {
            str = number(str, va_arg(arg, unsigned long), 10, field_width,
                precision, flags);
            break;
        }
        default: {
            *str++ = '%';
            if (*fmt) {
                *str++ = *fmt;
            } else {
                fmt--;
            }
            break;
        }
        }
    }
    *str = '\0';
    return str - string;
}

void init_printk_done() {
    g_printk_status = PRINTK_STATUS_READY;
    g_record = log_buf;
    g_record_len = 0;
}

int printk(const char *fmt, ...) {
    va_list arg;
    va_start(arg, fmt);
    int printed_len = my_printf(print_buffer, CONSOLE_PRINT_BUFFER_SIZE, fmt, arg);
    va_end(arg);

    if (g_printk_status == PRINTK_STATUS_READY) {
        uart_puts(print_buffer);
    }

    if (printed_len + g_record_len < LOG_BUF_LEN) {
        memcpy(g_record + g_record_len, print_buffer, printed_len);
        g_record_len += printed_len;
    }

    return printed_len;
}