#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

static void gpio_setup(void)
{
	// Enable GPIOC clock.
	rcc_periph_clock_enable(RCC_GPIOC);

	// Set GPIO13 (in GPIO port C) to 'output push-pull'.
	gpio_set_mode(
		GPIOC, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);
}


int main(void)
{
	int i;

	gpio_setup();

	// Blink the LED connected to PC13
	while (1) {
		gpio_toggle(GPIOC, GPIO13);
		for (i = 0; i < 800000; i++) {
			__asm__("nop");
		}
	}

	return 0;
}
