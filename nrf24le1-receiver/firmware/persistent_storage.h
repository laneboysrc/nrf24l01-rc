#ifndef __PERSISTENT_STORAGE_H__
#define __PERSISTENT_STORAGE_H__

#define NUMBER_OF_PERSISTENT_ELEMENTS 25

void load_persistent_storage(uint8_t *data);
void save_persistent_storage(uint8_t *new_data);

#endif
