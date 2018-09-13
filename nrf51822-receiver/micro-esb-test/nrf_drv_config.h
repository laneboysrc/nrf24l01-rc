#pragma once

#define GPIO_LED                    18


#define CLOCK_ENABLED               1
#define CLOCK_CONFIG_XTAL_FREQ      NRF_CLOCK_XTALFREQ_Default
#define CLOCK_CONFIG_LF_SRC         CLOCK_LFCLKSRC_SRC_Xtal
#define CLOCK_CONFIG_IRQ_PRIORITY   APP_IRQ_PRIORITY_LOW


#define NRF_MAXIMUM_LATENCY_US      1000


#define RTC0_ENABLED                1
#define RTC0_CONFIG_FREQUENCY       1000
#define RTC0_CONFIG_IRQ_PRIORITY    APP_IRQ_PRIORITY_LOW
#define RTC0_CONFIG_RELIABLE        false
#define RTC0_INSTANCE_INDEX         0
#define RTC_COUNT                   1


#define UART0_ENABLED               1
#define UART0_CONFIG_HWFC           NRF_UART_HWFC_DISABLED
#define UART0_CONFIG_PARITY         NRF_UART_PARITY_EXCLUDED
#define UART0_CONFIG_BAUDRATE       NRF_UART_BAUDRATE_115200
#define UART0_CONFIG_PSEL_TXD       9
#define UART0_CONFIG_PSEL_RXD       11
#define UART0_CONFIG_PSEL_CTS       10
#define UART0_CONFIG_PSEL_RTS       8
#define UART0_CONFIG_IRQ_PRIORITY   APP_IRQ_PRIORITY_HIGH
#define UART0_CONFIG_RX_BUFFER_SIZE 512
#define UART0_CONFIG_TX_BUFFER_SIZE 512


#define TIMER_COUNT                0
// NOTE: the micro ESB code uses TIMER2 internally with direct register access
