/*
 * ESP8266.c
 *
 *  Created on: Oct 1, 2024
 *      Author: Shreyas Acharya, BHARATI SOFTWARE
 */

#include "ESP8266.h"
#include "UartRingbuffer_multi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "firmware_parse.h"
#include "stm32f3xx_hal.h"

#define FLASH_START_ADDRESS  0x08008000

extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart1;
#define wifi_uart &huart3
#define pc_uart &huart1

/*****************************************************************************************************************************************/

// Wait for specific response with timeout
static int wait_for_response(char *response, UART_HandleTypeDef *uart, uint32_t timeout_ms) {
    uint32_t start_time = HAL_GetTick();
    while (HAL_GetTick() - start_time < timeout_ms) {
        if (Wait_for(response, uart)) return 1;
    }
    return 0;
}

// Send a command and wait for an expected response
static int send_command(const char *cmd, const char *response, uint32_t timeout_ms) {
    Uart_flush(wifi_uart);
    Uart_sendstring(cmd, wifi_uart);
    return wait_for_response((char *)response, wifi_uart, timeout_ms);
}

// Global variable to hold firmware info
FirmwareInfo fw_info;

// Parse the "size" from HTTP response
int parse_size(const char *response) {
    const char *size_pos = strstr(response, "\"size\":");
    return size_pos ? atoi(size_pos + strlen("\"size\":")) : 0;
}

// Function to parse and return the version as a string
// Parse the "version" from HTTP response
void parse_version(const char *response, char *version_out) {
    const char *version_pos = strstr(response, "\"firmware_id\":\"");
    if (version_pos) {
        version_pos += strlen("\"firmware_id\":\"");
        size_t len = 0;
        while (*version_pos != '"' && len < VERSION_MAX_LENGTH - 1) {
            *version_out++ = *version_pos++;
            len++;
        }
        *version_out = '\0';  // Null-terminate the version string
    } else {
        strcpy(version_out, "0.0.0");  // Default version if not found
    }
}

// Function to retrieve and parse firmware info
char get_firmware_info() {
    char http_get_request[256];
    char local_buf[64];
    char buff[512];

    // Send the HTTP GET request
    sprintf(http_get_request, "GET /api/firmware_info HTTP/1.1\r\nHost: 192.168.137.1:5000\r\nConnection: keep-alive\r\n\r\n");
    if (!send_command("AT+CIPSTART=0,\"TCP\",\"192.168.137.1\",5000\r\n", "CONNECT\r\n", 5000))
        return WIFI_ERR;

    sprintf(local_buf, "AT+CIPSEND=0,%d\r\n", (int)strlen(http_get_request));
    if (!send_command(local_buf, ">", 5000))
        return WIFI_ERR;

    Uart_sendstring(http_get_request, wifi_uart);
    if (!wait_for_response("SEND OK\r\n", wifi_uart, 5000))
        return WIFI_ERR;
    if (!wait_for_response("+IPD,", wifi_uart, 5000))
        return WIFI_ERR;
    if (!Copy_upto("CLOSED", buff, wifi_uart))
        return WIFI_ERR;

    // Parse size and version
    fw_info.size = parse_size(buff);
    parse_version(buff, fw_info.version);

    return WIFI_OK;
}



// ESP connection to WiFi with AT commands
wifi_status_t connect_to_wifi(char *SSID, char *PASSWD) {
    char data[80];
    if (!send_command("AT\r\n", "OK\r\n", 5000))
        return WIFI_ERR;
    if (!send_command("AT+CWMODE=1\r\n", "OK\r\n", 5000))
        return WIFI_ERR;

    sprintf(data, "AT+CWJAP=\"%s\",\"%s\"\r\n", SSID, PASSWD);
    Uart_flush(wifi_uart);
    Uart_sendstring(data, wifi_uart);

    if (!(wait_for_response("OK\r\n", wifi_uart, 20000) || Wait_for("FAIL\r\n", wifi_uart)))
        return WIFI_ERR;

    // Additional AT commands
    if (!send_command("AT+CIFSR\r\n", "OK\r\n", 5000))
        return WIFI_ERR;
    if (!send_command("AT+CIPMUX=1\r\n", "OK\r\n", 5000))
        return WIFI_ERR;

    return WIFI_OK;
}

// Retrieve and flash firmware from server
wifi_status_t get_firmware(uint8_t *buff) {
    char http_get_request[500] = {0};
    char local_buf[100];
    uint32_t flash_address = FLASH_START_ADDRESS;
    uint8_t data_buf[1024];
    int received_size = 0;
    int fw_size = fw_info.size;
    int chunk_size = 1024;

    if (!send_command("AT+CIPSTART=0,\"TCP\",\"192.168.137.1\",5000\r\n", "CONNECT\r\n", 5000)) return WIFI_ERR;

    while (received_size < fw_size) {
        int end_byte = received_size + chunk_size - 1;
        if (end_byte >= fw_size) end_byte = fw_size - 1;

        sprintf(http_get_request, "GET /OTA/firmware HTTP/1.1\r\nHost: 192.168.137.1:5000\r\nRange: bytes=%d-%d\r\nConnection: keep-alive\r\n\r\n", received_size, end_byte);
        sprintf(local_buf, "AT+CIPSEND=0,%d\r\n", (int)strlen(http_get_request));

        if (!send_command(local_buf, ">", 5000)) return WIFI_ERR;
        Uart_sendstring(http_get_request, wifi_uart);

        if (!wait_for_response("SEND OK\r\n", wifi_uart, 5000)) return WIFI_ERR;
        if (!wait_for_response("+IPD,", wifi_uart, 5000)) return WIFI_ERR;
        if (!Copy_upto("CLOSED", (char *)buff, wifi_uart)) return WIFI_ERR;

        firmware_parse((char *)data_buf, (char *)buff, chunk_size);
        flash_firmware(data_buf, flash_address, chunk_size);
        flash_address += chunk_size;
        received_size += chunk_size;

        if (received_size >= fw_size) break;

       if (!send_command("AT+CIPSTART=0,\"TCP\",\"192.168.137.1\",5000\r\n", "CONNECT\r\n", 5000)) return WIFI_ERR;
       HAL_Delay(100);
    }

    return WIFI_OK;
}
