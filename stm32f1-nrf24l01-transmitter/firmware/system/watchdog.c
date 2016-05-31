#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/iwdg.h>

#include <watchdog.h>


// ****************************************************************************
void WATCHDOG_start(void)
{
    iwdg_set_period_ms(100);
    iwdg_start();
}


// ****************************************************************************
void WATCHDOG_reset(void)
{
    iwdg_reset();
}
