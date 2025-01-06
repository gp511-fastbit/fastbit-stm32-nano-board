/*
 * sh1106.c
 *
 * Created on: Oct 2, 2024
 * Author: Shreyas Acharya, BHARATI SOFTWARE
 */

#include <sh1106.h>
#include "stm32f3xx_hal.h"

// SH1106 object
static SH1106_t SH1106;

// Helper function to send multiple commands and check for errors
static sh1106_t sh1106_send_commands(I2C_HandleTypeDef* sh_hi2c, const uint8_t* cmds, uint8_t cmd_count) {
  for (uint8_t i = 0; i < cmd_count; i++) {
    if (sh1106_write_command(sh_hi2c, cmds[i]) != SH1106_OK) {
        return SH1106_ERR;
    }
  }
  return SH1106_OK;
}

// Function to write a command to the SH1106
sh1106_t sh1106_write_command(I2C_HandleTypeDef* sh_hi2c, uint8_t cmd) {
    uint8_t buffer[2];
    buffer[0] = 0x00;  // Command mode
    buffer[1] = cmd;

    // I2C transmit and check for error
    if (HAL_I2C_Master_Transmit(sh_hi2c, (uint16_t)(0x3C << 1), buffer, 2, HAL_MAX_DELAY) != HAL_OK) {
        return SH1106_I2C_ERR;
    }
    return SH1106_OK;
}

// Function to initialize the SH1106 display
sh1106_t sh1106_init(I2C_HandleTypeDef* sh_hi2c) {
  HAL_Delay(100);
    // Initialize SH1106 structure
    SH1106.CurrentX = 0;
    SH1106.CurrentY = 0;
    SH1106.Inverted = 0;
    SH1106.Initialized = 1;

    // Array of initialization commands
    const uint8_t init_cmds[] = {
        0xAE,  // Display off
        0xD5, 0x80,  // Set display clock divide ratio, suggested value 0x80
        0xA8, 0x3F,  // Set multiplex, 1/64 duty
        0xD3, 0x00,  // Set display offset, no offset
        0x40,  // Set start line at 0
        0x8D, 0x14,  // Charge pump, enable charge pump
        0x20, 0x00,  // Memory mode, horizontal addressing mode
        0xA1,  // Set segment remap
        0xC8,  // Set COM output scan direction
        0xDA, 0x12,  // Set COM pins hardware configuration
        0x81, 0x7F,  // Set contrast control
        0xD9, 0xF1,  // Set pre-charge period
        0xDB, 0x40,  // Set VCOMH deselect level
        0xA4,  // Enable display all on
        0xA6,  // Normal display
        0xAF   // Display on
    };

    // Send all initialization commands
    return sh1106_send_commands(sh_hi2c, init_cmds, sizeof(init_cmds));
}

// Function to clear the display buffer
sh1106_t sh1106_clear(void) {
    memset(SH1106.Buffer, 0, sizeof(SH1106.Buffer));
    SH1106.CurrentX = 0;
    SH1106.CurrentY = 0;
    return SH1106_OK;
}

// Function to update the display with the buffer
sh1106_t sh1106_update_screen(I2C_HandleTypeDef* sh_hi2c) {
    for (uint8_t page = 0; page < 8; page++) {
        if (sh1106_write_command(sh_hi2c, 0xB0 + page) != SH1106_OK) return SH1106_ERR;
        if (sh1106_write_command(sh_hi2c, 0x02) != SH1106_OK) return SH1106_ERR;  // Set column address lower nibble
        if (sh1106_write_command(sh_hi2c, 0x10) != SH1106_OK) return SH1106_ERR;  // Set column address upper nibble

        // Write buffer to display, check for I2C errors
        if (HAL_I2C_Mem_Write(sh_hi2c, (uint16_t)(0x3C << 1), 0x40, 1, &SH1106.Buffer[SH1106_WIDTH * page], SH1106_WIDTH, HAL_MAX_DELAY) != HAL_OK) {
            return SH1106_I2C_ERR;
        }
    }
    return SH1106_OK;
}

// Draw a pixel on the buffer
sh1106_t sh1106_draw_pixel(uint8_t x, uint8_t y, sh1106_color_t color) {
    if (x >= SH1106_WIDTH || y >= SH1106_HEIGHT) {
        return SH1106_ERR;  // Out of bounds
    }

    if (color == SH1106_COLOR_WHITE) {
        SH1106.Buffer[x + (y / 8) * SH1106_WIDTH] |= (1 << (y % 8));
    } else {
        SH1106.Buffer[x + (y / 8) * SH1106_WIDTH] &= ~(1 << (y % 8));
    }
    return SH1106_OK;
}

// Set cursor position
sh1106_t sh1106_gotoXY(uint8_t x, uint8_t y) {
    if (x >= SH1106_WIDTH || y >= SH1106_HEIGHT) {
        return SH1106_ERR;  // Out of bounds
    }

    SH1106.CurrentX = x;
    SH1106.CurrentY = y;
    return SH1106_OK;
}

// Draw a line (Bresenham's algorithm)
sh1106_t sh1106_draw_line(I2C_HandleTypeDef* sh_hi2c, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, sh1106_color_t color) {
    int16_t dx = abs(x2 - x1);
    int16_t dy = -abs(y2 - y1);
    int16_t sx = x1 < x2 ? 1 : -1;
    int16_t sy = y1 < y2 ? 1 : -1;
    int16_t err = dx + dy;

    while (1) {
        sh1106_draw_pixel(x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        int16_t e2 = 2 * err;
        if (e2 >= dy) { err += dy; x1 += sx; }
        if (e2 <= dx) { err += dx; y1 += sy; }
    }
    return SH1106_OK;
}

// Draw a rectangle
sh1106_t sh1106_draw_rect(I2C_HandleTypeDef* sh_hi2c, uint8_t x, uint8_t y, uint8_t w, uint8_t h, sh1106_color_t color) {
    sh1106_draw_line(sh_hi2c, x, y, x + w, y, color);
    sh1106_draw_line(sh_hi2c, x + w, y, x + w, y + h, color);
    sh1106_draw_line(sh_hi2c, x + w, y + h, x, y + h, color);
    sh1106_draw_line(sh_hi2c, x, y + h, x, y, color);
    return SH1106_OK;
}

// Function to draw a circle (midpoint circle algorithm)
sh1106_t sh1106_draw_circle(I2C_HandleTypeDef* sh_hi2c, uint8_t x0, uint8_t y0, uint8_t r, sh1106_color_t color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    sh1106_draw_pixel(x0, y0 + r, color);
    sh1106_draw_pixel(x0, y0 - r, color);
    sh1106_draw_pixel(x0 + r, y0, color);
    sh1106_draw_pixel(x0 - r, y0, color);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        sh1106_draw_pixel(x0 + x, y0 + y, color);
        sh1106_draw_pixel(x0 - x, y0 + y, color);
        sh1106_draw_pixel(x0 + x, y0 - y, color);
        sh1106_draw_pixel(x0 - x, y0 - y, color);
        sh1106_draw_pixel(x0 + y, y0 + x, color);
        sh1106_draw_pixel(x0 - y, y0 + x, color);
        sh1106_draw_pixel(x0 + y, y0 - x, color);
        sh1106_draw_pixel(x0 - y, y0 - x, color);
    }
    return SH1106_OK;
}

char sh1106_putc(char ch, FontDef_t* Font, sh1106_color_t color) {
  uint32_t i, b, j;

  /* Check available space in LCD */
  if (
    SH1106_WIDTH <= (SH1106.CurrentX + Font->FontWidth) ||
    SH1106_HEIGHT <= (SH1106.CurrentY + Font->FontHeight)
  ) {
    /* Error */
    return 0;
  }

  /* Go through font */
  for (i = 0; i < Font->FontHeight; i++) {
    b = Font->data[(ch - 32) * Font->FontHeight + i];
    for (j = 0; j < Font->FontWidth; j++) {
      if ((b << j) & 0x8000) {
        sh1106_draw_pixel(SH1106.CurrentX + j, (SH1106.CurrentY + i), (sh1106_color_t) color);
      } else {
        sh1106_draw_pixel(SH1106.CurrentX + j, (SH1106.CurrentY + i), (sh1106_color_t)!color);
      }
    }
  }

  /* Increase pointer */
  SH1106.CurrentX += Font->FontWidth;

  /* Return character written */
  return ch;
}

char sh1106_puts(char* str, FontDef_t* Font, sh1106_color_t color) {
  /* Write characters */
  while (*str) {
    /* Write character by character */
    if (sh1106_putc(*str, Font, color) != *str) {
      /* Return error */
      return *str;
    }

    /* Increase string pointer */
    str++;
  }

  /* Everything OK, zero should be returned */
  return *str;
}
