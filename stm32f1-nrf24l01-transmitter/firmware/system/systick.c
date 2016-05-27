#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencmsis/core_cm3.h>

#include <systick.h>


// ****************************************************************************
#define MAX_SYSTICK_CALLBACKS 5


typedef struct {
    systick_callback callback;
    uint32_t trigger_ms;
} systick_callback_t;


volatile uint32_t milliseconds;

static systick_callback_t callbacks[MAX_SYSTICK_CALLBACKS];
static systick_callback rf_callback = NULL;
static uint32_t rf_callback_time_ms;


// ****************************************************************************
static systick_callback_t *find_callback(systick_callback cb)
{
    for (size_t i = 0; i < MAX_SYSTICK_CALLBACKS; i++) {
        if (callbacks[i].callback == cb) {
            return &callbacks[i];
        }
    }

    return NULL;
}


// ****************************************************************************
static systick_callback_t *get_emtpy_callback_slot(void)
{
    return find_callback(NULL);
}


// ****************************************************************************
void init_systick(void)
{
    uint32_t reload_value;

    // 24 MHz / 8 => 3000000 counts per second
    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);

    // SysTick interrupt every 1 ms
    reload_value = (rcc_ahb_frequency / 8 / 1000) - 1;
    systick_set_reload(reload_value);

    systick_interrupt_enable();
    systick_counter_enable();
}


// ****************************************************************************
void systick_set_callback(systick_callback cb, uint32_t duration_ms)
{
    systick_callback_t *slot;

    if (duration_ms == 0) {
        duration_ms = 1;
    }

    slot = find_callback(cb);
    if (slot == NULL) {
        slot = get_emtpy_callback_slot();
        if (slot == NULL) {
            printf("ALERT: MAX_SYSTICK_CALLBACKS needs to be increased!\n");
            return;
        }
    }

    cm_disable_interrupts();
    slot->callback = cb;
    slot->trigger_ms = milliseconds + duration_ms;
    cm_enable_interrupts();
}


// ****************************************************************************
void systick_clear_callback(systick_callback cb)
{
    systick_callback_t *slot;

    slot = find_callback(cb);
    if (slot) {
        cm_disable_interrupts();
        slot->callback = NULL;
        cm_enable_interrupts();
    }
}


// ****************************************************************************
void systick_set_rf_callback(systick_callback cb, uint32_t repetition_time_ms)
{
    rf_callback_time_ms = repetition_time_ms;
    rf_callback = cb;
}


// ****************************************************************************
void sys_tick_handler(void)
{
    ++milliseconds;

    // The RF callback is called every rf_callback_time_ms milliseconds.
    // One millisecond before we run the mixer to prepare the latest stick
    // and switch data for output.
    if (rf_callback) {
        if ((milliseconds % rf_callback_time_ms) == 0) {
            // FIXME: run the mixer to prepare for the next RF transmission
        }
        else if ((milliseconds % rf_callback_time_ms) == 1) {
            (*rf_callback)();
        }
    }

    for (size_t i = 0; i < MAX_SYSTICK_CALLBACKS; i++) {
        if (callbacks[i].callback != NULL  &&
            callbacks[i].trigger_ms == milliseconds ) {

            systick_callback cb = callbacks[i].callback;
            callbacks[i].callback = NULL;
            (*cb)();
        }
    }
}