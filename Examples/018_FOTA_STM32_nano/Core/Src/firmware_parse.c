/*
 * firmware_parse.c
 *
 *  Created on: Oct 1, 2024
 *      Author: Shreyas Acharya, BHARATI SOFTWARE
 */

#include <stdint.h>
#include <string.h>
#include "firmware_parse.h"
#include "stm32f3xx_hal.h"

// Function to calculate the length of a string (handles null-terminated strings)
int string_length(char *str) {
    int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

// Function to find a substring within a string, returning the position of the first match
char* substring_position(char *str, char *substr, int size) {
    int substrlen = string_length(substr);
    if (substrlen > size) return NULL;

    for (int i = 0; i <= size - substrlen; i++) {
        if (strncmp(&str[i], substr, substrlen) == 0) {
            return &str[i];
        }
    }
    return NULL;
}

// Main parsing function that processes the buffer
void firmware_parse(char *dst, char *src, int size) {
    uint16_t j = 0;
    char *str_pos = src;
    int cur_size = size;
    char *ipd_marker = "+IPD,";  // Marker to identify start
    char *end_marker = "0,CLOSED"; // Marker to identify end

    // Find the occurrence of the IPD marker
    char *ipd_pos = substring_position(str_pos, ipd_marker, cur_size);
    if (!ipd_pos) return;  // No IPD marker found

    // Move the position forward past the IPD marker and the next digits until the colon
    str_pos = ipd_pos;
    char *colon_pos = substring_position(str_pos, ":", cur_size);
    if (!colon_pos) return;  // No colon after +IPD found

    // Move the position past the colon to start copying the actual data
    cur_size -= (colon_pos + 1 - str_pos);
    str_pos = colon_pos + 1;

    // Find the end marker position
    char *end_pos = substring_position(str_pos, end_marker, cur_size);
    uint16_t remaining_len = end_pos ? (end_pos - str_pos) : cur_size;

    // Check for the last 12 bytes and adjust
    int total_bytes = remaining_len + 12; // Add 12 bytes to account for missing bytes

    // Copy the actual data into the destination buffer including the last 12 bytes
    for (int i = 0; i < total_bytes && i < size; i++) {
        dst[j++] = str_pos[i];
    }

    // Optionally, null-terminate the destination buffer if needed
    // dst[j] = '\0';
}

static uint8_t count = 0;

void flash_firmware(uint8_t* data_buf, uint32_t address, uint32_t length) {
    // Unlock flash memory for erasing and writing
    HAL_FLASH_Unlock();

    // Calculate the starting and ending page based on the address
    uint32_t page_start = address & ~(FLASH_PAGE_SIZE - 1);  // Align the address to the start of the page
    uint32_t page_end = (address + length - 1) & ~(FLASH_PAGE_SIZE - 1);  // Align the end address to the start of its page

    // Erase each even page in the range
    for (uint32_t page_address = page_start; page_address <= page_end; page_address += FLASH_PAGE_SIZE) {
        // Check if the page address is even
        if (count % 2 == 0) {
            // 1. Wait for no flash memory operation to be ongoing by checking the BSY bit in the FLASH_SR register
            while (FLASH->SR & FLASH_SR_BSY);

            // 2. Set the PER bit in the FLASH_CR register to enable page erase
            FLASH->CR |= FLASH_CR_PER;

            // 3. Program the FLASH_AR register with the page address to erase
            FLASH->AR = page_address;

            // 4. Set the STRT bit in the FLASH_CR register to start the erase operation
            FLASH->CR |= FLASH_CR_STRT;

            // 5. Wait for the BSY bit to be reset, indicating the operation is done
            while (FLASH->SR & FLASH_SR_BSY);

            // 6. Check the EOP flag in the FLASH_SR register (indicating success)
            if (FLASH->SR & FLASH_SR_EOP) {
                // 7. Clear the EOP flag by writing to it
                FLASH->SR |= FLASH_SR_EOP;
            } else {
                // Handle flash erase error (if EOP flag is not set)
                HAL_FLASH_Lock();  // Lock flash memory
                return;
            }

            // Clear the PER bit after the operation
            FLASH->CR &= ~FLASH_CR_PER;
        }
    }
    count++;

    // Write the firmware to flash memory
    for (uint32_t i = 0; i < length; i += 4) {
        uint32_t data = *((uint32_t*)(data_buf + i));  // Read 4 bytes at a time
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address + i, data);  // Write to flash

        // Verify the flash memory
        if (*(uint32_t*)(address + i) != data) {
            // Handle flash error
            HAL_FLASH_Lock();  // Lock flash memory
            return;
        }
    }

    // Lock flash memory after programming
    HAL_FLASH_Lock();
}

