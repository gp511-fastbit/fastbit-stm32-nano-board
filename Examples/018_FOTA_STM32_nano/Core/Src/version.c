/*
 * version.c
 *
 *  Created on: Oct 1, 2024
 *      Author: Shreyas Acharya, BHARATI SOFTWARE
 */

#include "stm32f3xx_hal.h"
#include "version.h"
#include "ESP8266.h"
#include <string.h>
#include "firmware_parse.h"

extern FirmwareInfo fw_info;
void get_current_version(char *version, size_t max_length){
  strncpy(version, fw_info.version, max_length - 1);
}

// Read data from a specified flash memory range into a variable
void get_stored_version(char *version, size_t max_length) {
    // Ensure we read from the correct flash memory address
    const uint32_t *flash_memory = (const uint32_t *)FIRMWARE_VERSION_PAGE_ADDRESS;

    // Clear the version buffer to avoid garbage data
    memset(version, 0, max_length);

    // Read from flash memory and copy to version variable
    for (size_t i = 0; i < max_length / 4; i++) {  // Read in 4-byte chunks (32 bits)
        uint32_t data = flash_memory[i];
        memcpy(version + (i * 4), &data, sizeof(data));  // Copy each word to the version buffer
    }

    // Ensure null-termination
    version[max_length - 1] = '\0';  // Null-terminate to prevent buffer overflow
}

void flash_firmware_version(uint8_t* data_buf, uint32_t address, uint32_t length){
  // Unlock flash memory for erasing and writing
      HAL_FLASH_Unlock();

      // Calculate the starting and ending page based on the address
      uint32_t page_start = address & ~(FLASH_PAGE_SIZE - 1);  // Align the address to the start of the page
      uint32_t page_end = (address + length - 1) & ~(FLASH_PAGE_SIZE - 1);  // Align the end address to the start of its page

      // Erase each even page in the range
      for (uint32_t page_address = page_start; page_address <= page_end; page_address += FLASH_PAGE_SIZE) {
          // Check if the page address is even

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
