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
// We use the extended endurance area of the flash memory to store the
// persistent data.  We use page 32, starting at MCU xdata adress 0xfa00.
// A page is 256 bytes, we only use the first 25 of them.
//
// During erasing and programming CPU execution halts. Since the writing is
// self timed we do not need to check for it to finish, once the CPU resumes
// operation the write process is done.
//
// Erasing a page takes at most 22.5ms, writing a byte 46us. This means that
// erasing and writing 25 bytes takes 24ms.
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
