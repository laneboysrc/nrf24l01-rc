#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include <nrf_nvmc.h>

#include <persistent_storage.h>


__attribute__ ((section(".persistent_data")))
const volatile uint8_t persistent_data[NUMBER_OF_PERSISTENT_ELEMENTS];


// ****************************e************************************************
void load_persistent_storage(uint8_t *data)
{
    memcpy(data, (void *)persistent_data, NUMBER_OF_PERSISTENT_ELEMENTS);
}


// ****************************************************************************
void save_persistent_storage(uint8_t new_data[])
{
    nrf_nvmc_page_erase((uint32_t)persistent_data);
    nrf_nvmc_write_words((uint32_t)persistent_data, (uint32_t *)new_data, NUMBER_OF_PERSISTENT_ELEMENTS/4+1);
}
