/******************************************************************************

    Application entry point.

    Contains the main loop and the hardware initialization.

******************************************************************************/
#include <stdio.h>
#include <stdbool.h>
#include <LPC8xx.h>

#include <uart0.h>
#include <spi.h>
#include <rc_receiver.h>

// ****************************************************************************
// IO pins: (LPC812 in TSSOP16 package)
//
// PIO0_0   (16, TDO, ISP-Rx)   Rx
// PIO0_1   (9,  TDI)           MISO
// PIO0_2   (6,  TMS, SWDIO)    SSEL
// PIO0_3   (5,  TCK, SWCLK)    SCK
// PIO0_4   (4,  TRST, ISP-Tx)  Tx
// PIO0_5   (3,  RESET)         RESET
// PIO0_6   (15)                RF interrupt
// PIO0_7   (14)                MOSI
// PIO0_8   (11, XTALIN)
// PIO0_9   (10, XTALOUT)
// PIO0_10  (8,  Open drain)
// PIO0_11  (7,  Open drain)
// PIO0_12  (2,  ISP-entry)     ISP
// PIO0_13  (1)                 RFCE
//
// GND      (13)
// 3.3V     (12)
// ****************************************************************************



#define __SYSTICK_IN_MS 10

void SysTick_handler(void);
void PININT0_irq_handler(void);


uint32_t systick_count;


// ****************************************************************************
static void init_hardware(void)
{

#if __SYSTEM_CLOCK != 12000000
#error Clock initialization code expexts __SYSTEM_CLOCK to be set to 1200000
#endif
    // Set flash wait-states to 1 system clock
    LPC_FLASHCTRL->FLASHCFG = 0;

    // ------------------------
    // Turn on peripheral clocks for SCTimer, IOCON, SPI0
    // (GPIO, SWM alrady enabled after reset)
    LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 8) | (1 << 18) | (1 << 11);


    // ------------------------
    // IO configuration

    // Enable reset, all other special functions disabled
    LPC_SWM->PINENABLE0 = 0xffffffbf;

    // U0_TXT_O = PIO0_4, U0_RXD_I = PIO0_0
    // SCK = PIO0_3, MOSI = PIO0_7, MISO = PIO0_1, SSEL = PIO0_2
    LPC_SWM->PINASSIGN0 = 0xffff0004;
    LPC_SWM->PINASSIGN3 = 0x03ffffff;
    LPC_SWM->PINASSIGN4 = 0xff020107;


    // Make SCK, MOSI, SSEL and RFCE outputs
    LPC_GPIO_PORT->DIR0 |= (1 << 2) | (1 << 3) | (1 << 7) | (1 << 13);
    LPC_GPIO_PORT->W0[13] = 0;

    // Make the open drain ports PIO0_10, PIO0_11 outputs and pull to ground
    // to prevent them from floating.
    LPC_GPIO_PORT->W0[10] = 0;
    LPC_GPIO_PORT->W0[11] = 0;
    LPC_GPIO_PORT->DIR0 |= (1 << 10) | (1 << 11);


    // Enable glitch filtering on the IOs
    // GOTCHA: ICONCLKDIV0 is actually the last register in the array!
    LPC_SYSCON->IOCONCLKDIV[6] = 255;       // Glitch filter 0: Main clock divided by 255
    LPC_SYSCON->IOCONCLKDIV[5] = 1;         // Glitch filter 0: Main clock divided by 1

    // NOTE: for some reason it is absolutely necessary to enable glitch
    // filtering on the IOs used for the capture timer. One clock cytle of the
    // main clock is enough, but with none weird things happen.


    // ------------------------
    // Configure SCTimer globally for two 16-bit counters
    LPC_SCT->CONFIG = 0;


    // ------------------------
    // Configure the exernal interrupt from the NRF chip
    LPC_SYSCON->PINTSEL[0] = 6;             // PIO0_6 (RF Int) on PININT0
    LPC_PIN_INT->IENF = (1 << 0);           // Enable falling edge on PININT0


    // ------------------------
    // SysTick configuration
    SysTick->LOAD = __SYSTEM_CLOCK * __SYSTICK_IN_MS / 1000;
    SysTick->VAL = __SYSTEM_CLOCK * __SYSTICK_IN_MS / 1000;
    SysTick->CTRL = (1 << 0) |              // Enable System Tick counter
                    (1 << 1) |              // System Tick interrupt enable
                    (1 << 2);               // Use system clock
}


// ****************************************************************************
static void init_hardware_final(void)
{
    // Turn off peripheral clock for IOCON and SWM to preserve power
    LPC_SYSCON->SYSAHBCLKCTRL &= ~((1 << 18) | (1 << 7));
}


// ****************************************************************************
void PININT0_irq_handler(void)
{
    LPC_PIN_INT->IST = (1 << 0);          // Clear the interrupt status flag
    rf_interrupt_handler();
}


// ****************************************************************************
void SysTick_handler(void)
{
    if (SysTick->CTRL & (1 << 16)) {       // Read and clear Countflag
        ++systick_count;
    }
}


// ****************************************************************************
static void service_systick(void)
{
    if (!systick_count) {
        return;
    }

    // Disable the SysTick interrupt. Use memory barriers to ensure that no
    // interrupt is pending in the pipeline.
    // More info:
    // http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dai0321a/BIHHFHJD.html
    SysTick->CTRL &= ~(1 << 1);
    __DSB();
    __ISB();
    --systick_count;
    SysTick->CTRL |= (1 << 1);      // Re-enable the system tick interrupt
}


// ****************************************************************************
static void stack_check(void)
{
    #define CANARY 0xcafebabe

    static uint32_t *last_found = (uint32_t *)(0x10001000 - 48);
    uint32_t *now;

    if (last_found == (uint32_t *)0x10000000) {
        return;
    }

    now = last_found;
    while (*now != CANARY && now > (uint32_t *)0x10000000) {
        --now;
    }

    if (now != last_found) {
        last_found = now;
        uart0_send_cstring("Stack down to 0x");
        uart0_send_uint32_hex((uint32_t)now);
        uart0_send_linefeed();
    }
}


// ****************************************************************************
int main(void)
{
    init_hardware();
    init_uart0(115200);
    init_hardware_final();

    init_receiver();

    uart0_send_cstring("Receiver initialized\n");

    while (1) {
        service_systick();
        process_receiver();
        stack_check();
    }
}
