#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <sdk_common.h>
#include <nrf_drv_config.h>
#include <nrf.h>
#include <nrf_drv_clock.h>
#include <nrf_drv_rtc.h>
#include <nrf_drv_uart.h>
#include <nrf_gpio.h>
#include <app_uart.h>

#include <micro_esb.h>
#include <uesb_error_codes.h>

#define SYSTICK_IN_MS (1000/RTC0_CONFIG_FREQUENCY)
volatile uint32_t milliseconds;

#define PAYLOAD_SIZE 10
#define ADDRESS_WIDTH 5
static const uint8_t BIND_CHANNEL = 0x51;


// ****************************************************************************
static void CLOCKS_init( void )
{
    nrf_drv_clock_init();
    nrf_drv_clock_lfclk_request(NULL);
}


// ****************************************************************************
static void GPIO_init( void )
{
    nrf_gpio_cfg_output(GPIO_LED);
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
// Callback for the micro ESB stack
void rf_event_handler(void)
{
    static uint32_t rf_interrupts;

    uesb_get_clear_interrupts(&rf_interrupts);

    if(rf_interrupts & UESB_INT_TX_SUCCESS_MSK) {
        printf("TX SUCCESS EVENT\n");
    }

    if(rf_interrupts & UESB_INT_TX_FAILED_MSK) {
        printf("TX FAILED EVENT\n");
    }

    if(rf_interrupts & UESB_INT_RX_DR_MSK) {
        printf("RX RECEIVED EVENT\n");
    }
}

// ****************************************************************************
static void TX_init(void)
{
    uint32_t ret;
    uesb_config_t uesb_config = {
        .bitrate               = UESB_BITRATE_250KBPS,
        .crc                   = UESB_CRC_16BIT,
        .dynamic_ack_enabled   = 0,
        .dynamic_payload_length_enabled = 0,
        .event_handler         = rf_event_handler,
        .mode                  = UESB_MODE_PTX,
        .payload_length        = PAYLOAD_SIZE,
        .protocol              = UESB_PROTOCOL_SB,
        .radio_irq_priority    = 1,
        .rf_addr_length        = ADDRESS_WIDTH,
        .rf_channel            = BIND_CHANNEL,
        .rx_address_p0         = {0x12, 0x23, 0x23, 0x45, 0x78},
        .rx_pipes_enabled      = 0x01,
        .tx_mode               = UESB_TXMODE_AUTO,                 \
    };

    ret = uesb_init(&uesb_config);
    if (ret != NRF_SUCCESS) {
        printf("uesb_config failed ret=%lu\n", ret);
    }
}

// ****************************************************************************
static void TX_send(void)
{
    uesb_payload_t payload;
    uint32_t ret;

    memcpy(payload.data, "Hello!", 6);
    ret = uesb_write_tx_payload(&payload);
    if (ret != NRF_SUCCESS) {
        printf("uesb_write_tx_payload failed ret=%lu\n", ret);
    }
}

// ****************************************************************************
int main(void)
{
    GPIO_init();
    CLOCKS_init();
    RTC_init();
    UART_init();

    TX_init();
    printf("nRF51822 transmitter running\n");

    while (true) {
        static uint32_t next = 500;

        UART_read();

        if (milliseconds >= next) {
            TX_send();
            nrf_gpio_pin_toggle(GPIO_LED);
            next += 500;
        }

        __WFE();
    }
}

