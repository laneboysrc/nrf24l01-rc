// This file has been ported from Deviation.

// This code will write a stack-trace to disk should a fault happen
// Since when it runs, memory may be corrupted, we want to use as
// few functions or static memory addresses as possible.


#include <stdint.h>

#include <libopencm3/stm32/iwdg.h>
#include <libopencm3/stm32/usart.h>
#include <libopencmsis/core_cm3.h>


#define NONE      0
#define BACKTRACE 1
#define DUMP      2

#define MEMORY_DUMP NONE
// #define MEMORY_DUMP BACKTRACE
// #define MEMORY_DUMP DUMP


#define MEMORY_START  ((unsigned int *)0x20000000)
#define MEMORY_END    (&_stack)

// A random value unlikely to appear in data we want to dump
#define PRINTLN_NO_VALUE 0xFABACEAE


extern unsigned _stack;     // Defined in stm32f103c8t6.ld

void fault_handler_c(unsigned int * hardfault_args, unsigned int fault_type);


// ****************************************************************************
void hard_fault_handler(void)
{
    __asm(
"    MRS   R0, MSP\n"
"    MOVS  R1, #0\n"
"    B fault_handler_c\n");
}


// ****************************************************************************
void exti2_isr(void)
{
    __asm(
"    MRS   R0, MSP\n"
"    MOVS  R1, #1\n"
"    B fault_handler_c\n");
}


// ****************************************************************************
static void write_byte(uint8_t x)
{
    usart_send_blocking(USART1, x);
}


// ****************************************************************************
static void write_long(unsigned int val)
{
    for (int i = 7; i >= 0; i--) {
        uint8_t v = 0x0f & (val >> (4 * i));
        if (v < 10)
            write_byte('0' + v);
        else
            write_byte('a' + v - 10);
    }
}


// ****************************************************************************
static void fault_println(const char *str, unsigned int val)
{
    while (*str) {
        write_byte(*str);
        str++;
    }

    if (val != PRINTLN_NO_VALUE) {    // Magic value unlikely to be used
        write_long(val);
    }

    write_byte('\n');
}


// ****************************************************************************
#if MEMORY_DUMP == BACKTRACE
static void backtrace(unsigned int *hardfault_args)
{
    int count = 0;
    unsigned int *ptr = hardfault_args;
    fault_println("Backtrace:", PRINTLN_NO_VALUE);
    while (ptr < (&_stack)  &&  count < 20) {
        if ((*ptr & 0xFFF00001) == 0x08000001) {
            // This looks like it may be a return address
            write_long((unsigned int)ptr);
            fault_println(": ", *ptr);
            count++;
        }
        ptr = (unsigned int *)((unsigned int)ptr + 1);
        iwdg_reset();
    }
    fault_println("Done", PRINTLN_NO_VALUE);
}
#endif


// ****************************************************************************
#if MEMORY_DUMP == DUMP
static void write_hex_byte(uint8_t val)
{
    for (int i = 1; i >= 0; i--) {
        uint8_t v = 0x0f & (val >> (4 * i));
        if (v < 10)
            write_byte('0' + v);
        else
            write_byte('a' + v - 10);
    }

    write_byte(' ');
}

static void memory_dump(void)
{
    fault_println("Memory Dump:", PRINTLN_NO_VALUE);
    for (unsigned int *i = MEMORY_START; i < MEMORY_END; i++) {
        write_hex_byte(*(const uint8_t *)i);
        iwdg_reset();
    }
}
#endif


// ****************************************************************************
// From Joseph Yiu, minor edits by FVH
// Hard fault handler in C, with stack frame location as input parameter
void fault_handler_c(unsigned int * hardfault_args, unsigned int fault_type)
{
    unsigned int stacked_r0;
    unsigned int stacked_r1;
    unsigned int stacked_r2;
    unsigned int stacked_r3;
    unsigned int stacked_r12;
    unsigned int stacked_lr;
    unsigned int stacked_pc;
    unsigned int stacked_psr;

    stacked_r0 = ((unsigned long) hardfault_args[0]);
    stacked_r1 = ((unsigned long) hardfault_args[1]);
    stacked_r2 = ((unsigned long) hardfault_args[2]);
    stacked_r3 = ((unsigned long) hardfault_args[3]);

    stacked_r12 = ((unsigned long) hardfault_args[4]);
    stacked_lr = ((unsigned long) hardfault_args[5]);
    stacked_pc = ((unsigned long) hardfault_args[6]);
    stacked_psr = ((unsigned long) hardfault_args[7]);

    if (fault_type) {
        fault_println("\n\n[Watchdog]", PRINTLN_NO_VALUE);
    } else {
        fault_println("\n\n[Hard fault]", PRINTLN_NO_VALUE);
    }

    fault_println("R0 = ", stacked_r0);
    fault_println("R1 = ", stacked_r1);
    fault_println("R2 = ", stacked_r2);
    fault_println("R3 = ", stacked_r3);
    fault_println("R12 = ", stacked_r12);
    fault_println("LR [R14] (subroutine call return address) = ", stacked_lr);
    fault_println("PC [R15] (program counter) = ", stacked_pc);
    fault_println("PSR = ", stacked_psr);
    fault_println("BFAR = ", (*((volatile unsigned long *)(0xE000ED38))));
    fault_println("CFSR = ", (*((volatile unsigned long *)(0xE000ED28))));
    fault_println("HFSR = ", (*((volatile unsigned long *)(0xE000ED2C))));
    fault_println("DFSR = ", (*((volatile unsigned long *)(0xE000ED30))));
    fault_println("AFSR = ", (*((volatile unsigned long *)(0xE000ED3C))));
    fault_println("SCB_SHCSR = ", SCB_SHCSR);
    fault_println("Top of Stack:", (unsigned int)(&_stack));
    fault_println("Stack Detect:", (unsigned int)hardfault_args);

#if MEMORY_DUMP == BACKTRACE
    backtrace(hardfault_args);
#elif MEMORY_DUMP == DUMP
    memory_dump();
#endif

    // NVIC_SystemReset() is not in libopencm3. This code snippet originates
    // from ARMs core_cm3.h
#define NVIC_AIRCR_VECTKEY    (0x5FA << 16)
#define NVIC_SYSRESETREQ       2
    SCB->AIRCR  = (NVIC_AIRCR_VECTKEY | (SCB->AIRCR & (0x700)) | (1<<NVIC_SYSRESETREQ));
    // Ensure completion of memory access
    __asm("dsb");
    while(1);
}
