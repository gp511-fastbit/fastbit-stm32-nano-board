/*
 * app.c
 *
 *  Created on: Oct 19, 2024
 *      Author: Shreyas Acharya, BHRATI SOFTWARE
 */

#include "parser.h"
#include "sh1106.h"
#include "esp8266.h"
#include "esp8266_io.h"
#include "app.h"
#include "fonts.h"
#include "main.h"
#include <stdio.h>
#include <string.h>
#include "stm32f3xx_hal.h"

/****************************************
 * Definitions and Constants
 ****************************************/
#define DISPLAY_WIDTH 132
#define FONT_WIDTH 11
#define LONG_PRESS_DURATION 1000

extern UART_HandleTypeDef huart1;
extern I2C_HandleTypeDef hi2c1;
extern ring_buffer_t wifi_rx_buffer;

#define PC_UART &huart1

MenuState menu_bar = home;
uint8_t buttonPressed = 0;
uint32_t buttonPressStart = 0;

char recv_buf[2048 * 6] = {0};
char output_buffer[1024 * 3] = {0};

/**
 * @brief Updates the home page display.
 */
void update_home_page(void) {
    sh1106_clear();
    sh1106_update_screen(&hi2c1);

#ifdef Start
    sh1106_gotoXY(28, 16);
    sh1106_puts("FastBit", &Font_11x18, 1);
    sh1106_gotoXY(38, 39);
    sh1106_puts("Embedded", &Font_7x10, 1);
#else
    sh1106_gotoXY(7, 16);
    sh1106_puts("Today's", &Font_11x18, 1);
    sh1106_gotoXY(63, 39);
    sh1106_puts("News", &Font_11x18, 1);
#endif

    sh1106_update_screen(&hi2c1);
}

/**
 * @brief Scrolls text left on the SH1106 display.
 *
 * @param text The text to scroll.
 * @param delay_ms The delay in milliseconds between scroll steps.
 */
void sh1106_scroll_text_left(const char* text, uint16_t delay_ms) {
    int16_t x_pos = DISPLAY_WIDTH;
    int16_t text_length = strlen(text);
    int16_t text_pixel_width = text_length * FONT_WIDTH;
    int16_t reset_pos = -text_pixel_width;
    sh1106_clear();
    while (1) {
        int16_t start_idx = (x_pos < 0) ? (-x_pos) / FONT_WIDTH : 0;
        int16_t end_idx = (x_pos + text_pixel_width > DISPLAY_WIDTH) ? (DISPLAY_WIDTH - x_pos) / FONT_WIDTH : text_length;

        for (int16_t i = start_idx; i < end_idx; i++) {
            int16_t char_x_pos = x_pos + i * FONT_WIDTH;
            if (char_x_pos >= -FONT_WIDTH && char_x_pos < DISPLAY_WIDTH) {
                sh1106_gotoXY(char_x_pos, 24);
                sh1106_putc(text[i], &Font_11x18, 1);
            }
        }

        sh1106_update_screen(&hi2c1);
        x_pos--;
        if (x_pos < reset_pos) {
            x_pos = DISPLAY_WIDTH;
        }
    }
}

/**
 * @brief Sends a string over UART.
 *
 * @param s The string to send.
 * @param uart The UART handle.
 */
void uart_send_string(const char *s, UART_HandleTypeDef *uart) {
    HAL_UART_Transmit(uart, (const uint8_t *)s, strlen(s), HAL_MAX_DELAY);
}

/**
 * @brief Displays a menu section.
 *
 * @param selected The index of the selected section.
 */
void display_section(uint8_t selected) {
    const char *sections[] = {"HOME", "US", "SCIENCE", "ARTS", "WORLD"};
    sh1106_clear();

    for (uint8_t i = 0; i < 5; i++) {
        sh1106_gotoXY(0, i * 11);
        sh1106_puts((char *)sections[i], &Font_7x10, (selected == i) ? 0 : 1);
    }

    sh1106_update_screen(&hi2c1);
}

/**
 * @brief Establishes a server connection using ESP8266.
 *
 * @return 0 if successful, -1 otherwise.
 */
static int8_t connect_server(void) {
    esp8266_connection_info_t connection_info = {
        .connection_type = ESP8266_TCP_CONNECTION,
        .ip_address = (uint8_t *)API_SERVER,
        .is_server = ESP8266_FALSE,
        .port = (uint32_t)API_PORT
    };

    return (esp8266_establish_connection(&connection_info) == ESP8266_OK) ? 0 : -1;
}

/**
 * @brief Retrieves news from a specified URL.
 *
 * @param url The URL to fetch news from.
 */
void get_news_from(const char *url) {
    //uint32_t data_size = 0;
    esp8266_send_data((uint8_t *)url, strlen(url));
    //esp8266_recv_data((uint8_t *)recv_buf, sizeof(recv_buf), &data_size);
    parse_JSON((char*)wifi_rx_buffer.data, output_buffer, sizeof(output_buffer));
}

/**
 * @brief Handles button press events for a menu section.
 *
 * @param section The name of the section.
 * @param api_url The API URL for the section.
 */
void handle_press_event(const char *section, const char *api_url) {
    sh1106_clear();
    sh1106_gotoXY(10, 10);
    sh1106_puts((char *)section, &Font_11x18, 1);
    sh1106_update_screen(&hi2c1);

    if (connect_server() < 0) {
        printf("Error: Unable to connect to server\n");
        return;
    }

    get_news_from(api_url);
    sh1106_scroll_text_left(output_buffer, 50);
}

/**
 * @brief Checks for button events and handles short/long presses.
 *
 * @param short_press_next_state The next state for a short press.
 * @param section The name of the section.
 * @param api_url The API URL for the section.
 */
void check_button_event(MenuState short_press_next_state, const char *section, const char *api_url) {
    if (HAL_GPIO_ReadPin(OK_GPIO_Port, OK_Pin) == GPIO_PIN_SET) {
        if (!buttonPressed) {
            buttonPressed = 1;
            buttonPressStart = HAL_GetTick();
        }
    } else if (buttonPressed) {
        buttonPressed = 0;
        uint32_t press_duration = HAL_GetTick() - buttonPressStart;
        if (press_duration >= LONG_PRESS_DURATION) {
            handle_press_event(section, api_url);
        } else {
            menu_bar = short_press_next_state;
        }
    }
}

/**
 * @brief Constructs an API request string.
 *
 * @param buffer The buffer to store the request.
 * @param buffer_size The size of the buffer.
 * @param endpoint The API endpoint.
 */
static void construct_api_request(char *buffer, size_t buffer_size, const char *endpoint) {
    snprintf(buffer, buffer_size,
             "GET /svc/topstories/v2/%s.json?api-key=%s HTTP/1.1\r\n"
             "Host: api.nytimes.com\r\n"
             "Connection: close\r\n"
             "User-Agent: MyNewsApp/1.0\r\n"
             "Accept: application/json\r\n"
             "Range: bytes=0-20000\r\n\r\n",
             endpoint, API_KEY);
}

/**
 * @brief Main application loop.
 */
void main_app(void) {
  char request_buffer[512];
  switch (menu_bar) {
    case home:
      display_section(home);
      construct_api_request(request_buffer, sizeof(request_buffer), "home");
      check_button_event(us, "Home", request_buffer);
      break;

    case us:
      display_section(us);
      construct_api_request(request_buffer, sizeof(request_buffer), "us");
      check_button_event(science, "US", request_buffer);
      break;

    case science:
      display_section(science);
      construct_api_request(request_buffer, sizeof(request_buffer), "science");
      check_button_event(arts, "Science", request_buffer);
      break;

    case arts:
      display_section(arts);
      construct_api_request(request_buffer, sizeof(request_buffer), "arts");
      check_button_event(world, "Arts", request_buffer);
      break;

    case world:
      display_section(world);
      construct_api_request(request_buffer, sizeof(request_buffer), "world");
      check_button_event(home, "World", request_buffer);
      break;
  }
}
