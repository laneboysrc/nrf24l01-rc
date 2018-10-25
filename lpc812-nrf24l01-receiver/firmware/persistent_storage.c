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

    // M05 test data
    // data[0] = 0xeb;
    // data[1] = 0x5d;
    // data[2] = 0xb8;
    // data[3] = 0xd5;
    // data[4] = 0x07;

    // data[5] = 5;
    // data[6] = 6;
    // data[7] = 7;
    // data[8] = 8;
    // data[9] = 9;
    // data[10] = 10;
    // data[11] = 11;
    // data[12] = 12;
    // data[13] = 13;
    // data[14] = 14;
    // data[15] = 15;
    // data[16] = 16;
    // data[17] = 17;
    // data[18] = 18;
    // data[19] = 19;
    // data[20] = 20;
    // data[21] = 21;
    // data[22] = 22;
    // data[23] = 23;
    // data[24] = 24;

    // data[25] = 0xaa;
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
