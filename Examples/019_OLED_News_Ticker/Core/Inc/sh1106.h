/*
 * sh1106.h
 *
 * Created on: Oct 2, 2024
 * Author: Shreyas Acharya, BHARATI SOFTWARE
 */

#ifndef SH1106_H_
#define SH1106_H_

#include "stm32f3xx_hal.h"
#include <stdlib.h>
#include <string.h>
#include "fonts.h"

// SH1106 display dimensions
#define SH1106_WIDTH  128
#define SH1106_HEIGHT 64

// Enum for SH1106 status
typedef enum {
  SH1106_OK = 0x00,
  SH1106_ERR = 0x01,
  SH1106_I2C_ERR = 0x02
} sh1106_t;

// Enum for color selection
typedef enum {
  SH1106_COLOR_BLACK = 0x00,
  SH1106_COLOR_WHITE = 0x01
} sh1106_color_t;

// SH1106 structure to store display state
typedef struct {
  uint16_t CurrentX;
  uint16_t CurrentY;
  uint8_t Inverted;
  uint8_t Initialized;
  uint8_t Buffer[SH1106_WIDTH * SH1106_HEIGHT / 8];
} SH1106_t;

// Function prototypes

/**
 * @brief Initialize the SH1106 display
 * @param sh_hi2c I2C handle
 * @return Status of initialization
 */
sh1106_t sh1106_init(I2C_HandleTypeDef* sh_hi2c);

/**
 * @brief Write a command to the SH1106
 * @param sh_hi2c I2C handle
 * @param cmd Command to send
 * @return Status of I2C write operation
 */
sh1106_t sh1106_write_command(I2C_HandleTypeDef* sh_hi2c, uint8_t cmd);

/**
 * @brief Clear the display buffer
 * @return Status of operation
 */
sh1106_t sh1106_clear(void);

/**
 * @brief Update the display with the contents of the buffer
 * @param sh_hi2c I2C handle
 * @return Status of I2C operation
 */
sh1106_t sh1106_update_screen(I2C_HandleTypeDef* sh_hi2c);

/**
 * @brief Draw a pixel on the buffer
 * @param x X coordinate
 * @param y Y coordinate
 * @param color Pixel color (black or white)
 * @return Status of operation
 */
sh1106_t sh1106_draw_pixel(uint8_t x, uint8_t y, sh1106_color_t color);

/**
 * @brief Set the cursor position
 * @param x X coordinate
 * @param y Y coordinate
 * @return Status of operation
 */
sh1106_t sh1106_gotoXY(uint8_t x, uint8_t y);

/**
 * @brief Draw a line using Bresenham's algorithm
 * @param sh_hi2c I2C handle
 * @param x1 Starting X coordinate
 * @param y1 Starting Y coordinate
 * @param x2 Ending X coordinate
 * @param y2 Ending Y coordinate
 * @param color Line color (black or white)
 * @return Status of operation
 */
sh1106_t sh1106_draw_line(I2C_HandleTypeDef* sh_hi2c, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, sh1106_color_t color);

/**
 * @brief Draw a rectangle
 * @param sh_hi2c I2C handle
 * @param x Starting X coordinate
 * @param y Starting Y coordinate
 * @param w Width of the rectangle
 * @param h Height of the rectangle
 * @param color Rectangle color (black or white)
 * @return Status of operation
 */
sh1106_t sh1106_draw_rect(I2C_HandleTypeDef* sh_hi2c, uint8_t x, uint8_t y, uint8_t w, uint8_t h, sh1106_color_t color);

/**
 * @brief Draw a circle using the midpoint circle algorithm
 * @param sh_hi2c I2C handle
 * @param x0 Center X coordinate
 * @param y0 Center Y coordinate
 * @param r Radius of the circle
 * @param color Circle color (black or white)
 * @return Status of operation
 */
sh1106_t sh1106_draw_circle(I2C_HandleTypeDef* sh_hi2c, uint8_t x0, uint8_t y0, uint8_t r, sh1106_color_t color);

/**
 * @brief Draw a character on the display buffer using a specified font.
 * @param ch    The character to be drawn
 * @param Font  Pointer to the font structure containing font data
 * @param color Color of the character (black or white)
 * @return The drawn character if successful, 0 if there's an error (e.g., character out of range)
 */
char sh1106_putc(char ch, FontDef_t* Font, sh1106_color_t color);

/**
 * @brief Draw a string of characters on the display buffer.
 * @param str   The string to be drawn on the display
 * @param Font  Pointer to the font structure containing font data
 * @param color Color of the text (black or white)
 * @return The last character drawn if successful, 0 if there’s an error (e.g., unsupported characters)
 */
char sh1106_puts(char* str, FontDef_t* Font, sh1106_color_t color);

#endif /* SH1106_H_ */
