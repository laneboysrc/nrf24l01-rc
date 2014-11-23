/******************************************************************************

    Application entry point.

    Contains the main loop and the hardware initialization.

******************************************************************************/
#include <stdio.h>
#include <stdbool.h>

#include <platform.h>
#include <uart0.h>
#include <spi.h>
#include <rc_receiver.h>

#include <LPC8xx_ROM_API.h>


void SysTick_handler(void);
void PININT0_irq_handler(void);



// Global flag that is true for one mainloop every __SYSTICK_IN_MS
bool systick;

static uint32_t systick_count;



// ****************************************************************************
static void init_hardware(void)
{
    int i;

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

    // Enable hardware inputs and outputs
    LPC_SWM->PINASSIGN0 = (0xff << 24) |
                          (0xff << 16) |
                          (GPIO_BIT_UART_RX << 8) |
                          (GPIO_BIT_UART_TX << 0);

    LPC_SWM->PINASSIGN3 = (GPIO_BIT_NRF_SCK << 24) |
                          (0xff << 16) |
                          (0xff << 8) |
                          (0xff << 0);

    LPC_SWM->PINASSIGN4 = (0xff << 24) |
                          (GPIO_BIT_NRF_CSN << 16) |
                          (GPIO_BIT_NRF_MISO << 8) |
                          (GPIO_BIT_NRF_MOSI << 0);

    // CTOUT_0 = PIO0_7 (CH1) CTOUT_1 = PIO0_6 (CH2)
    LPC_SWM->PINASSIGN6 = 0x07ffffff;
    LPC_SWM->PINASSIGN7 = 0xffffff06;


    // Configure outputs
    LPC_GPIO_PORT->DIR0 |= (1 << GPIO_BIT_NRF_SCK) |
                           (1 << GPIO_BIT_NRF_MOSI) |
                           (1 << GPIO_BIT_NRF_CSN) |
                           (1 << GPIO_BIT_NRF_CE) |
                           (1 << GPIO_BIT_LED);
    GPIO_NRF_CE = 0;


    // Enable glitch filtering on the IOs
    // GOTCHA: ICONCLKDIV0 is actually the last register in the array!
    LPC_SYSCON->IOCONCLKDIV[6] = 255;       // Glitch filter 0: Main clock divided by 255
    LPC_SYSCON->IOCONCLKDIV[5] = 1;         // Glitch filter 0: Main clock divided by 1

    // NOTE: for some reason it is absolutely necessary to enable glitch
    // filtering on the IOs used for the capture timer. One clock cytle of the
    // main clock is enough, but with none weird things happen.


    // ------------------------
    // Configure SCTimer globally for two 16-bit counters
    //
    // Timer H is used for the servo outputs, setting the servo pins
    // on timer reload and clearing them when a match condition occurs.
    // The timer is running at 1 MHz clock (1us resolution).
    // The repeat frequency is 15ms (a multiple of the on-air packet repeat
    // rate).
    // The 4 servo pulses are generated with MATCH registers 1..4. and
    // corresponding timer outputs 0..3.
    // MATCH register 0 is used for auto-reload of the timer period.
    LPC_SCT->CONFIG = 0;

    LPC_SCT->CONFIG |= (1 << 18);           // Auto-limit on counter H
    LPC_SCT->CTRL_H |= (1 << 3) |           // Clear the counter H
        (((__SYSTEM_CLOCK / 1000000) - 1) << 5); // PRE_H[12:5] = divide for 1 MHz
    LPC_SCT->MATCHREL[0].H = 15000 - 1;     // 15 ms per overflow
    LPC_SCT->MATCHREL[1].H = 1500;          // Servo pulse 1.5 ms intially
    LPC_SCT->MATCHREL[2].H = 1500;
    LPC_SCT->MATCHREL[3].H = 1500;
    LPC_SCT->MATCHREL[4].H = 1500;

    // All 5 events are setup in the same way:
    // Event happens in all states; Match register of the same number;
    // Match counter H, Match condition only.
    for (i = 0; i < 5; i++) {
        LPC_SCT->EVENT[i].STATE = 0xFFFF;       // Event happens in all states
        LPC_SCT->EVENT[i].CTRL = (i << 0) |     // Match register
                                 (1 << 4) |     // Select H counter
                                 (0x1 << 12);   // Match condition only
    }

    // All servo outputs will be set with timer reload event 0
    LPC_SCT->OUT[0].SET = (1u << 0);
    LPC_SCT->OUT[1].SET = (1u << 0);
    LPC_SCT->OUT[2].SET = (1u << 0);
    LPC_SCT->OUT[3].SET = (1u << 0);

    LPC_SCT->OUT[0].CLR = (1u << 1);        // Event 1 will clear CTOUT_0
    LPC_SCT->OUT[1].CLR = (1u << 2);        // Event 2 will clear CTOUT_1
    LPC_SCT->OUT[2].CLR = (1u << 3);        // Event 3 will clear CTOUT_2
    LPC_SCT->OUT[3].CLR = (1u << 4);        // Event 4 will clear CTOUT_3

    // We don't start the timer here but only after receiving the first
    // valid stick data package.
    // LPC_SCT->CTRL_H &= ~(1u << 2);          // Start the SCTimer H


    // ------------------------
    // Configure the exernal interrupt from the NRF chip
    LPC_SYSCON->PINTSEL[0] = 11;            // PIO0_11 (NRF_IRQ) on PININT0
    LPC_PIN_INT->IENF = (1 << 0);           // Enable falling edge on PININT0
    NVIC_EnableIRQ(PININT0_IRQn);


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
        systick = false;
        return;
    }

    systick = true;

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
void invoke_ISP(void)
{
    unsigned int param[5];

    param[0] = 57;  // Reinvoke ISP
    __disable_irq();
    iap_entry(param, param);

    // This should never execute ...
    __enable_irq();
    uart0_send_cstring("ERROR: Reinvoke ISP failed\n");
    while(1);
}


// ****************************************************************************
int main(void)
{
    init_hardware();
    init_uart0(115200);
    init_spi();
    init_hardware_final();

    init_receiver();

    uart0_send_cstring("Receiver initialized\n");

    while (1) {
        service_systick();
        process_receiver();
        stack_check();
    }
}
