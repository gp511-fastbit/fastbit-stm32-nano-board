/*
 * version.h
 *
 *  Created on: Oct 1, 2024
 *      Author: Shreyas Acharya, BHARATI SOFTWARE
 */

#ifndef INC_VERSION_H_
#define INC_VERSION_H_
#include "stm32f3xx_hal.h"

#define FIRMWARE_VERSION_PAGE_ADDRESS 0x08020000
#define VERSION_MAX_LENGTH 16

void get_current_version(char *version, size_t max_length);
void get_stored_version(char *version, size_t max_length);
void flash_firmware_version(uint8_t* data_buf, uint32_t address, uint32_t length);

#endif /* INC_VERSION_H_ */
