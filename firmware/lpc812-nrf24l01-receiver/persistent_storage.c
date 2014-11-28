/******************************************************************************

	Use IAP to program the flash
	A single page of 64 bytes should be sufficient
	Top 32 bytes of RAM needed
	RAM buffer with data needs to be on word boundary
	Uses 148 bytes of stack space
	Use compare function to only write changes
	Interrupts must be disabled during erase and write operations
	Q: How long does erase and write take?

******************************************************************************/
#include <stdint.h>
#include <stdbool.h>

#include <LPC8xx.h>
#include <persistent_storage.h>
#include <uart0.h>


typedef void (* IAP)(unsigned int [], unsigned int[]);
extern IAP iap_entry;


__attribute__ ((section(".persistent_data")))
const volatile uint8_t persistent_data[NUMBER_OF_PERSISTENT_ELEMENTS];


// ****************************************************************************
void load_persistent_storage(uint8_t *data)
{
    int i;

    for (i = 0; i < NUMBER_OF_PERSISTENT_ELEMENTS; i++) {
        data[i] = persistent_data[i];
    }
}


// ****************************************************************************
void save_persistent_storage(uint8_t new_data[])
{
    unsigned int param[5];
    int i;

    for (i = 0; i < NUMBER_OF_PERSISTENT_ELEMENTS; i++) {
        if (new_data[i] != persistent_data[i]) {

            param[0] = 50;
            param[1] = ((unsigned int)persistent_data) >> 10;
            param[2] = ((unsigned int)persistent_data) >> 10;
            __disable_irq();
            iap_entry(param, param);
            __enable_irq();
            if (param[0] != 0) {
#ifndef NO_DEBUG
                uart0_send_cstring("ERROR: prepare sector failed\n");
#endif
                return;
            }

            param[0] = 59;  // Erase page command
            param[1] = ((unsigned int)persistent_data) >> 6;
            param[2] = ((unsigned int)persistent_data) >> 6;
            param[3] = __SYSTEM_CLOCK / 1000;
            __disable_irq();
            iap_entry(param, param);
            __enable_irq();
            if (param[0] != 0) {
#ifndef NO_DEBUG
                uart0_send_cstring("ERROR: erase page failed\n");
#endif
                return;
            }

            param[0] = 50;
            param[1] = ((unsigned int)persistent_data) >> 10;
            param[2] = ((unsigned int)persistent_data) >> 10;
            __disable_irq();
            iap_entry(param, param);
            __enable_irq();
            if (param[0] != 0) {
#ifndef NO_DEBUG
                uart0_send_cstring("ERROR: prepare sector failed\n");
#endif
                return;
            }

            param[0] = 51;  // Copy RAM to Flash command
            param[1] = (unsigned int)persistent_data;
            param[2] = (unsigned int)new_data;
            param[3] = 64;
            param[4] = __SYSTEM_CLOCK / 1000;
            __disable_irq();
            iap_entry(param, param);
            __enable_irq();
            if (param[0] != 0) {
#ifndef NO_DEBUG
                uart0_send_cstring("ERROR: copy RAM to flash failed: ");
                uart0_send_uint32_hex(param[0]);
                uart0_send_linefeed();
#endif
                return;
            }

            return;
        }
    }
}
