/*
 * ESP8266.h
 *
 *  Created on: Oct 1, 2024
 *      Author: Shreyas Acharya, BHARATI SOFTWARE
 */

#ifndef INC_ESP8266_H_
#define INC_ESP8266_H_

#include <stdint.h>
#include "version.h"
typedef enum {
    WIFI_ERR,
    WIFI_OK
} wifi_status_t;

// Update FirmwareInfo structure
typedef struct {
    int size;
    char version[VERSION_MAX_LENGTH];  // Store version as a string
} FirmwareInfo;

wifi_status_t connect_to_wifi(char *SSID, char *PASSWD);
wifi_status_t get_firmware(uint8_t *buff);
char get_firmware_info();
int firmware_size() ;


#endif /* INC_ESP8266_H_ */
