#include <stdlib.h>
#include <stdint.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>

#include <spi.h>


// ****************************************************************************
void init_spi(void)
{
    rcc_periph_clock_enable(RCC_SPI2);

    // Configure GPIOs: SS=PB12, SCK=PB13, MISO=PB14 and MOSI=PB15
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
            GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO12 | GPIO13 | GPIO15);
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO14);

    // Reset SPI, SPI_CR1 register cleared, SPI is disabled
    spi_reset(SPI2);

    // Set up SPI2 in Master mode
    spi_init_master(SPI2, SPI_CR1_BAUDRATE_FPCLK_DIV_64,
        SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE, SPI_CR1_CPHA_CLK_TRANSITION_1,
        SPI_CR1_DFF_8BIT, SPI_CR1_MSBFIRST);

    // Enable hardware NSS
    spi_enable_ss_output(SPI2);
}


// ****************************************************************************
uint8_t spi_transaction(unsigned int count, uint8_t *buffer)
{
    spi_enable(SPI2);

    for (unsigned int i = 0; i < count; i++) {
         uint8_t rx = (uint8_t)spi_xfer(SPI2, buffer[i]);
         buffer[i] = rx;
    }

    // Disable SPI2 to release NSS
    spi_disable(SPI2);

    return *buffer;
}

