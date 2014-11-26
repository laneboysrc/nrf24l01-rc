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
#include <preprocessor_output.h>

#include <LPC8xx_ROM_API.h>


#ifndef BAUDRATE
    #define BAUDRATE 115200
#endif


void SysTick_handler(void);
void PININT0_irq_handler(void);
void SCT_irq_handler(void);



// Global flag that is true for one mainloop every __SYSTICK_IN_MS
bool systick;

static uint32_t volatile systick_count;



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
    // Turn on brown-out detection and reset
    LPC_SYSCON->BODCTRL = (1 << 4) | (1 << 2) | (2 << 0);


    // ------------------------
    // IO configuration

    // Enable reset, all other special functions disabled
    LPC_SWM->PINENABLE0 = 0xffffffbf;

    // Enable hardware inputs and outputs
    LPC_SWM->PINASSIGN0 = (0xff << 24) |
                          (0xff << 16) |
                          (0xff << 8) |                 // UART0_RX
                          (GPIO_BIT_UART_TX << 0);      // UART0_TX

    LPC_SWM->PINASSIGN3 = (GPIO_BIT_NRF_SCK << 24) |    // SPI0_SCK
                          (0xff << 16) |
                          (0xff << 8) |
                          (0xff << 0);

    LPC_SWM->PINASSIGN4 = (0xff << 24) |
                          (GPIO_BIT_NRF_CSN << 16) |    // SPI0_SSEL
                          (GPIO_BIT_NRF_MISO << 8) |    // SPI0_MISO
                          (GPIO_BIT_NRF_MOSI << 0);     // SPI0_MOSI

    LPC_SWM->PINASSIGN6 = (GPIO_BIT_CH1 << 24) |        // CTOUT_0
                          (0xff << 16) |
                          (0xff << 8) |
                          (0xff << 0);

    LPC_SWM->PINASSIGN7 = (0xff << 24) |
                          (0xff << 16) |
                          (GPIO_BIT_CH3 << 8) |         // CTOUT_2
                          (GPIO_BIT_CH2 << 0);          // CTOUT_1

    // Configure outputs
    LPC_GPIO_PORT->DIR0 |= (1 << GPIO_BIT_NRF_SCK) |
                           (1 << GPIO_BIT_NRF_MOSI) |
                           (1 << GPIO_BIT_NRF_CSN) |
                           (1 << GPIO_BIT_NRF_CE) |
                           (1 << GPIO_BIT_LED);
    GPIO_NRF_CE = 0;
    GPIO_LED = 0;


    // ------------------------
    // Configure SCTimer globally for two 16-bit counters
    //
    // Timer H is used for the servo outputs, setting the servo pins
    // on timer reload and clearing them when a match condition occurs.
    // The timer is running at 1 MHz clock (1us resolution).
    // The repeat frequency is 15ms (a multiple of the on-air packet repeat
    // rate).
    // The 3 servo pulses are generated with MATCH registers 1..3. and
    // corresponding timer outputs 0..2.
    // MATCH register 0 is used for auto-reload of the timer period.
    //
    // Timer L is used for frequency hopping. It is configured as a simple
    // auto-reload that fires an interrupt in regular intervals, using event[4].
    // The rc_receiver.c takes care of setting the counter and limit values.
    LPC_SCT->CONFIG = 0;

    LPC_SCT->CONFIG |= (1 << 18) |                  // Auto-limit on counter H
                       (1 << 17);                   // Auto-limit on counter L
    LPC_SCT->CTRL_H |= (1 << 3) |                   // Clear the counter H
        (((__SYSTEM_CLOCK / 1333333) - 1) << 5);    // PRE_H[12:5] = divide for 750ns clock
    LPC_SCT->MATCHREL[0].H = 16000 - 1;             // 16 ms servo pulse repeat time
    LPC_SCT->MATCHREL[1].H = SERVO_PULSE_CENTER;    // Servo pulse 1.5 ms intially
    LPC_SCT->MATCHREL[2].H = SERVO_PULSE_CENTER;
    LPC_SCT->MATCHREL[3].H = SERVO_PULSE_CENTER;

    // All 4 events are setup in the same way:
    // Event happens in all states; Match register of the same number;
    // Match counter H, Match condition only.
    for (i = 0; i < 4; i++) {
        LPC_SCT->EVENT[i].STATE = 0xFFFF;           // Event happens in all states
        LPC_SCT->EVENT[i].CTRL = (i << 0) |         // Match register
                                 (1 << 4) |         // Select H counter
                                 (0x1 << 12);       // Match condition only
    }

    // All servo outputs will be set with timer reload event 0
    LPC_SCT->OUT[0].SET = (1u << 0);
    LPC_SCT->OUT[1].SET = (1u << 0);
    LPC_SCT->OUT[2].SET = (1u << 0);

    LPC_SCT->OUT[0].CLR = (1u << 1);                // Event 1 will clear CTOUT_0
    LPC_SCT->OUT[1].CLR = (1u << 2);                // Event 2 will clear CTOUT_1
    LPC_SCT->OUT[2].CLR = (1u << 3);                // Event 3 will clear CTOUT_2

    // We don't start the timer here but only after receiving the first
    // valid stick data package.

    LPC_SCT->CTRL_L |= (1 << 3) |                   // Clear the counter L
        (((__SYSTEM_CLOCK / 1000000) - 1) << 5);    // PRE_L[12:5] = divide for 1 MHz
    LPC_SCT->EVENT[4].STATE = 0xFFFF;               // Event happens in all states
    LPC_SCT->EVENT[4].CTRL = (0 << 0) |             // Match register
                             (0 << 4) |             // Select counter L
                             (0x1 << 12);           // Match condition only
    LPC_SCT->EVEN |= (1 << 4);                      // Event 4 generates an interrupt
    NVIC_EnableIRQ(SCT_IRQn);


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
void SCT_irq_handler(void)
{
    // Clear the Event 4 flag. It is the only one we've set up to trigger
    // an interrupt so no need to check other flags.
    LPC_SCT->EVFLAG = (1 << 4);

    hop_timer_handler();
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
#ifndef NO_DEBUG
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
#endif
}


// ****************************************************************************
void invoke_ISP(void)
{
    unsigned int param[5];

    param[0] = 57;  // Reinvoke ISP
    __disable_irq();
    iap_entry(param, param);

#ifndef NO_DEBUG
    // This should never execute ...
    __enable_irq();
    uart0_send_cstring("ERROR: Reinvoke ISP failed\n");
    while(1);
#endif
}


// ****************************************************************************
int main(void)
{
    init_hardware();
    init_uart0(BAUDRATE);
    init_spi();
    init_hardware_final();

    // Wait a for a short time after power up before talking to the nRF24
    while (systick_count < (50 / __SYSTICK_IN_MS)) {
        ;
    }
    systick_count = 0;

    init_receiver();

#ifndef NO_DEBUG
    uart0_send_cstring("Receiver initialized\n");
#endif

    while (1) {
        service_systick();
        process_receiver();
        output_preprocessor();
        stack_check();
    }
}
