#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/iwdg.h>

#include <watchdog.h>


// ****************************************************************************
void WATCHDOG_start(void)
{
    iwdg_set_period_ms(100);
    iwdg_start();

    nvic_clear_pending_irq(NVIC_EXTI2_IRQ);

    nvic_set_priority(NVIC_EXTI2_IRQ, 0x01); //Highest priority
    nvic_enable_irq(NVIC_EXTI2_IRQ);
}


// ****************************************************************************
void WATCHDOG_reset(void)
{
    iwdg_reset();
}
