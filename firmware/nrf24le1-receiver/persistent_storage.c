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

#include <persistent_storage.h>
#include <uart0.h>




// ****************************************************************************
void load_persistent_storage(uint8_t *data)
{
    (void)data;
}


// ****************************************************************************
void save_persistent_storage(uint8_t new_data[])
{
    (void)new_data;
}
