#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <sdk_common.h>
#include <nrf_drv_config.h>
#include <nrf.h>
#include <nrf_drv_clock.h>
#include <nrf_drv_rtc.h>
#include <nrf_drv_uart.h>
#include <nrf_gpio.h>
#include <app_uart.h>

#include <servo.h>

#define SYSTICK_IN_MS 1
volatile uint32_t milliseconds;
static volatile uint8_t systick_count;


// ****************************************************************************
static void CLOCKS_init( void )
{
    nrf_drv_clock_init();
    nrf_drv_clock_lfclk_request(NULL);
}


// ****************************************************************************
static void GPIO_init( void )
{
    nrf_gpio_cfg_input(GPIO_SELECT_MODE, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_output(LED_0);
    nrf_gpio_cfg_output(LED_1);
}


// ****************************************************************************
static void uart_error_handler(app_uart_evt_t * p_app_uart_event)
{
    // Nothing we can do when the UART fails
}


// ****************************************************************************
static void UART_init(void)
{
    uint32_t err_code;
    const app_uart_comm_params_t comm_params = {
        .rx_pin_no = UART0_CONFIG_PSEL_RXD,
        .tx_pin_no = UART0_CONFIG_PSEL_TXD,
        .rts_pin_no = UART0_CONFIG_PSEL_RTS,
        .cts_pin_no = UART0_CONFIG_PSEL_CTS,
        .flow_control = UART0_CONFIG_HWFC,
        .use_parity = UART0_CONFIG_PARITY,
        .baud_rate = UART0_CONFIG_BAUDRATE
    };

    APP_UART_FIFO_INIT(&comm_params, UART0_CONFIG_RX_BUFFER_SIZE,
        UART0_CONFIG_TX_BUFFER_SIZE, uart_error_handler, APP_IRQ_PRIORITY_LOW,
        err_code);

    (void) err_code;
}


// ****************************************************************************
static void UART_read()
{
    uint8_t byte;
    static uint8_t stream[5];

    while (app_uart_get(&byte) == NRF_SUCCESS) {
        uint8_t last_index = sizeof(stream) - 1;

        for (int i = 0; i < last_index; i++) {
            stream[i] = stream[i+1];
        }
        stream[last_index] = byte;

        if (strncmp((char *)stream, "PING\n", sizeof(stream)) == 0) {
            printf("PONG\n");
        }
    }
}


// ****************************************************************************
static void rtc_callback(nrf_drv_rtc_int_type_t int_type)
{
    if (int_type == NRF_DRV_RTC_INT_TICK) {
        ++systick_count;
        milliseconds += SYSTICK_IN_MS;
    }
}

// ****************************************************************************
// Setup the RTC to provide a TICK
static void RTC_init(void)
{
    static const nrf_drv_rtc_t rtc = NRF_DRV_RTC_INSTANCE(0);

    static const nrf_drv_rtc_config_t rtc_config = {
        .prescaler = (uint16_t)(RTC_INPUT_FREQ / RTC0_CONFIG_FREQUENCY)-1,
        .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
        .tick_latency = RTC_US_TO_TICKS(NRF_MAXIMUM_LATENCY_US, RTC0_CONFIG_FREQUENCY),
        .reliable = false
    };

    //Initialize RTC instance
    nrf_drv_rtc_init(&rtc, &rtc_config, rtc_callback);

    //Enable tick event & interrupt
    nrf_drv_rtc_tick_enable(&rtc, true);

    //Power on RTC instance
    nrf_drv_rtc_enable(&rtc);
}


// ****************************************************************************
void LED_blink(void)
{
    static uint32_t next = 500;

    if (milliseconds >= next) {
        next += 500;
        nrf_gpio_pin_toggle(LED_0);
    }
}


// ****************************************************************************
int main(void)
{
    GPIO_init();
    CLOCKS_init();
    RTC_init();
    UART_init();

    SERVO_init();

    printf("nRF51822 receiver running\n");

    while (true) {
        UART_read();
        LED_blink();
        SERVO_process();

        __WFE();
    }
}

