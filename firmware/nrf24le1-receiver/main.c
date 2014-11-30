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



#ifndef BAUDRATE
    #define BAUDRATE 115200
#endif


#define TIMER_VALUE_US(x) (0xffff - ((uint32_t)(__SYSTEM_CLOCK / 1000) / 12 * x / 1000))

#define TIMER_16_MS TIMER_VALUE_US(16000)
#define TIMER_150_US TIMER_VALUE_US(150)


extern bool successful_stick_data;


// Global flag that is true for one mainloop every __SYSTICK_IN_MS
bool systick;

static volatile uint8_t systick_count;


// ****************************************************************************
// Prototypes for the interrupt handlers located in other files
void rf_interrupt_handler(void) __interrupt ((0x004b - 3) / 8);
void hop_timer_handler(void) __interrupt ((0x002b - 3) / 8);
void servo_pulse_timer_handler(void) __interrupt ((0x001b - 3) / 8) __using (1);



// ****************************************************************************
// Timer 0 interrupt handler at vector 1 (0x000b)
void timer0_isr(void) __interrupt (1)
{
    ++systick_count;
    if (successful_stick_data) {
        // Start timer1 with a very short interval to kick off one set of
        // servo pulses
        TH1 = TIMER_150_US >> 8;
        TL1 = TIMER_150_US;
        TCON_tr1 = 1;
    }
}


// ****************************************************************************
static void service_systick(void)
{
    if (!systick_count) {
        systick = false;
        return;
    }

    --systick_count;
    systick = true;
}


// ****************************************************************************
static void init_hardware(void)
{
    TMOD = 0x11;            // Set Timer0 and Timer1 as 16 bit timer
    TCON = 0;               // Clear Timer0 and Timer1 flags

    TH0 = TIMER_16_MS >> 8;
    TL0 = TIMER_16_MS;

    IEN0_tf0 = 1;           // Enable Timer0 interrupt
    IEN0_tf1 = 1;           // Enable Timer1 interrupt
    IEN1_rfirq = 1;

    IP0 = (1 << 3);         // Timer 1 interrupt has higher priority than the rest
}


// ****************************************************************************
static void init_hardware_final(void)
{
    IEN0_all = 1;
}


// ****************************************************************************
void delay_us(uint16_t microseconds)
{
    (void)microseconds;
}


// ****************************************************************************
int main(void)
{
    init_hardware();
    init_uart0(BAUDRATE);
    init_spi();
    init_hardware_final();

#ifndef NO_DEBUG
    uart0_send_cstring("Hardware initialized\n");
#endif

    // Wait a for a short time after power up before talking to the nRF24
    delay_us(20000);
    init_receiver();

#ifndef NO_DEBUG
    uart0_send_cstring("Receiver initialized\n");
#endif

    while (1) {
        service_systick();
        process_receiver();

#ifdef ENABLE_PREPROCESSOR_OUTPUT
        output_preprocessor();
#endif
    }
}
