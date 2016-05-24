#include <stdlib.h>
#include <stdint.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>

#include <spi.h>


// ****************************************************************************
void init_spi(void)
{
    rcc_periph_clock_enable(RCC_SPI1);

    // Configure GPIOs: SS=PA4, SCK=PA5, MISO=PA6 and MOSI=PA7
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
            GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO4 | GPIO5 | GPIO7 );
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO6);

    // Reset SPI, SPI_CR1 register cleared, SPI is disabled
    spi_reset(SPI1);

    // Set up SPI1 in Master mode
    spi_init_master(SPI1, SPI_CR1_BAUDRATE_FPCLK_DIV_64,
        SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE, SPI_CR1_CPHA_CLK_TRANSITION_1,
        SPI_CR1_DFF_8BIT, SPI_CR1_MSBFIRST);

    // Enable hardware NSS
    spi_enable_ss_output(SPI1);
}


// ****************************************************************************
uint8_t spi_transaction(unsigned int count, uint8_t *buffer)
{
    spi_enable(SPI1);

    for (unsigned int i = 0; i < count; i++) {
         uint8_t rx = (uint8_t)spi_xfer(SPI1, buffer[i]);
         buffer[i] = rx;
    }

    // Disable SPI1 to release NSS
    spi_clean_disable(SPI1);

    return *buffer;
}

