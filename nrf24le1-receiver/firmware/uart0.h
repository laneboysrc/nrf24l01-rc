#ifndef __UART0_H
#define __UART0_H

#include <stdint.h>
#include <stdbool.h>

#ifdef ENABLE_UART

void init_uart0(void);
void disable_uart0(void);

bool uart0_send_is_ready(void);
void uart0_send_char(const char c);
void uart0_send_cstring(const char *cstring);
void uart0_send_int32(int32_t number);
void uart0_send_uint32(uint32_t number);
void uart0_send_uint32_hex(uint32_t number);
void uart0_send_uint16_hex(uint16_t number);
void uart0_send_uint8_hex(uint8_t number);
void uart0_send_uint8_binary(uint8_t number);
void uart0_send_linefeed(void);

// Read routines are not implemented
// bool uart0_read_is_byte_pending(void);
// uint8_t uart0_read_byte(void);

#else /* ENABLE_UART */

#define init_uart0()
#define disable_uart0()
#define uart0_send_cstring(x)
#define uart0_send_char(x)
#define uart0_send_int32(x)
#define uart0_send_uint32(x)
#define uart0_send_uint32_hex(x)
#define uart0_send_uint16_hex(x)
#define uart0_send_uint8_hex(x)
#define uart0_send_uint8_binary(x)
#define uart0_send_linefeed()


#endif /* ENABLE_UART */


#endif /* __UART0_H */
