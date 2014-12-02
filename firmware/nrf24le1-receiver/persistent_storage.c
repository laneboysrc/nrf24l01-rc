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

#include <platform.h>
#include <persistent_storage.h>
#include <uart0.h>

#define PERSISTENT_DATA_PAGE 32
__xdata __at (0xfa00) uint8_t persistent_data[256] ;


// ****************************************************************************
void load_persistent_storage(uint8_t *data)
{
	uint8_t i;

    for (i = 0; i < NUMBER_OF_PERSISTENT_ELEMENTS; i++) {
        data[i] = persistent_data[i];
    }
}


// ****************************************************************************
void save_persistent_storage(uint8_t new_data[])
{
	uint8_t i;
	__bit save_ea;

	save_ea = IEN0_all;
	IEN0_all = 0;

    for (i = 0; i < NUMBER_OF_PERSISTENT_ELEMENTS; i++) {
        if (new_data[i] != persistent_data[i]) {
            // Clear the PMW bit to work on the NV Memory area
        	PCON &= ~(1 << 4);
        	FSR_wen = 1;

        	// Clear the NV memory page the presistent data resides in
        	FCR = PERSISTENT_DATA_PAGE;

        	// Write the persistent data
		    for (i = 0; i < NUMBER_OF_PERSISTENT_ELEMENTS; i++) {
		        persistent_data[i] = new_data[i];
		    }

        	FSR_wen = 0;
		}
	}

	IEN0_all = save_ea;
}
