#include <project.h>

#include "stdint.h"

#define PRINT_BUF_LEN 64
static char print_buf[PRINT_BUF_LEN];

static uint8_t __print_num(char *buf, uint32_t num, uint32_t base) {
    uint32_t m;
    char c;
    uint8_t chars = 0;

    do {
        m = num;
        num /= base;
        c = m - base * num;
        *--buf = c < 10 ? c + '0' : c + 'A' - 10;
        chars++;
    } while (num);

    return chars;
}

char *print_time(uint8_t hour, uint8_t min, uint8_t sec) {
    char *str = &print_buf[PRINT_BUF_LEN - 1];

    *str = '\0';

    str -= __print_num(str, sec, 10);
    *--str = ':';
    str -= __print_num(str, min, 10);
    *--str = ':';
    str -= __print_num(str, hour, 10);

    return str;
}

char *print_num(uint32_t num) {
    char *str = &print_buf[PRINT_BUF_LEN - 1];

    *str = '\0';
    str -= __print_num(str, num, 10);

    return str;
}

char *print_hex(uint32_t num) {
    char *str = &print_buf[PRINT_BUF_LEN - 1];

    *str = '\0';
    str -= __print_num(str, num, 16);

    return str;
}

/* Function to change integer to ascii with negative values*/
char* itoa(int value, char *result, int base) {

	char* ptr = result;
	char* ptr1 = result;
	char tmp_char;
	//*ptr = result;
	//*ptr1 = result;
	int tmp_value;
	if(base < 2 || base > 36) { *result = '\0'; return result; }
	do {
		tmp_value = value;
		value /= base;
		*ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
	} while ( value );

	//Apply negative sign
	if (tmp_value < 0) *ptr++ = '-';
	*ptr-- = '\0';
	while (ptr1 < ptr) {
		tmp_char = *ptr;
		*ptr--= *ptr1;
		*ptr1++ = tmp_char;
	}
	return result;
}
