/*
 * firmware_parse.h
 *
 *  Created on: Oct 1, 2024
 *      Author: Shreyas Acharya, BHARATI SOFTWARE
 */

#ifndef INC_FIRMWARE_PARSE_H_
#define INC_FIRMWARE_PARSE_H_

void firmware_parse(char *dst, char *src, int size);
void flash_firmware(uint8_t* data_buf, uint32_t address, uint32_t length);

#endif /* INC_FIRMWARE_PARSE_H_ */
