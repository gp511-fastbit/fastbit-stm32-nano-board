/*
 * GC9A01A.c
 *
 *  Created on: Jul 9, 2025
 *      Author: bhara
 */

#include "main.h"
#include "GC9A01A.h"
#include <stdlib.h>

/**
  * @brief  Hardware reset for GC9A01A LCD
  * @param  None
  * @retval None
  */
static void gc9a01a_hw_reset(void) {
    GC9A01A_RST_LOW;
    HAL_Delay(50);
    GC9A01A_RST_HIGH;
    HAL_Delay(120);
}

/**
  * @brief  Sends a command to the GC9A01A LCD display via SPI.
  * @param  cmd: The command byte to be sent to the display.
  * @retval None
  */
static void gc9a01a_write_cmd(uint8_t cmd) {
    //GC9A01A_CS_LOW;
    GC9A01A_DC_CMD;
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    //GC9A01A_CS_HIGH;
}

/**
  * @brief  Sends a buffer of data to the GC9A01A LCD display via SPI.
  * @param  buff: Pointer to the buffer containing the data to be sent.
  * @param  buff_size: Size of the buffer in bytes.
  * @retval None
  */
static void gc9a01a_write_data(uint8_t* buff, size_t buff_size) {
    //GC9A01A_CS_LOW;
    GC9A01A_DC_DATA;
    while (buff_size > 0) {
        uint16_t chunk_size = buff_size > 32768 ? 32768 : buff_size;
        HAL_SPI_Transmit(&hspi1, buff, chunk_size, HAL_MAX_DELAY);
        buff += chunk_size;
        buff_size -= chunk_size;
    }
    //GC9A01A_CS_HIGH;
}

/**
  * @brief  Set an area for drawing on the display with start row, end row and start col, end col.
  * @param  x0: Start column address.
  * @param  x1: End column address.
  * @param  y0: Start row address.
  * @param  y1: End row address.
  * @retval None
  */
void gc9a01a_set_address_window(uint16_t x0, uint16_t x1, uint16_t y0, uint16_t y1) {
    //GC9A01A_CS_LOW;

    gc9a01a_write_cmd(GC9A01A_CASET);
    {
        uint8_t data[] = { (x0 >> 8) & 0xFF, x0 & 0xFF, (x1 >> 8) & 0xFF, x1 & 0xFF };
        gc9a01a_write_data(data, sizeof(data));
    }

    gc9a01a_write_cmd(GC9A01A_RASET);
    {
        uint8_t data[] = { (y0 >> 8) & 0xFF, y0 & 0xFF, (y1 >> 8) & 0xFF, y1 & 0xFF };
        gc9a01a_write_data(data, sizeof(data));
    }

    gc9a01a_write_cmd(GC9A01A_RAMWR);

    //GC9A01A_CS_HIGH;
}

/**
  * @brief  Initialize the GC9A01A LCD
  * @param  None
  * @retval None
  */
static void gc9a01a_init(void) {
    uint8_t params[15];
    //GC9A01A_CS_LOW;

    gc9a01a_write_cmd(GC9A01A_INREGEN1); // Inter register enable 1
    gc9a01a_write_cmd(GC9A01A_INREGEN2); // Inter register enable 2

    params[0] = 0x08;
    params[1] = 0x09;
    params[2] = 0x14;
    params[3] = 0x08;
    gc9a01a_write_cmd(0xB5);
    gc9a01a_write_data(params, 4);

    params[0] = 0x00;
    params[1] = 0x00;
    gc9a01a_write_cmd(GC9A01A_DISP_CTRL); // Display Function Control
    gc9a01a_write_data(params, 2);

    params[0] = 0x48;
    gc9a01a_write_cmd(GC9A01A_MADCTL); // Memory Access Control
    gc9a01a_write_data(params, 1);

    params[0] = 0x05;
    gc9a01a_write_cmd(GC9A01A_COLMOD); // COLMOD: Pixel Format Set
    gc9a01a_write_data(params, 1);

    params[0] = 0x01;
    gc9a01a_write_cmd(0xBA); // TE width
    gc9a01a_write_data(params, 1);

    params[0] = 0x1A;
    gc9a01a_write_cmd(GC9A01A1_POWER2); // Power Control 2
    gc9a01a_write_data(params, 1);

    params[0] = 0x1A;
    gc9a01a_write_cmd(GC9A01A1_POWER3); // Power Control 3
    gc9a01a_write_data(params, 1);

    params[0] = 0x25;
    gc9a01a_write_cmd(GC9A01A1_POWER4); // Power Control 4
    gc9a01a_write_data(params, 1);

    params[0] = 0x45;
    params[1] = 0x09;
    params[2] = 0x08;
    params[3] = 0x08;
    params[4] = 0x26;
    params[5] = 0x2A;
    gc9a01a_write_cmd(GC9A01A_GAMMA1); // Set gamma 1
    gc9a01a_write_data(params, 6);

    params[0] = 0x43;
    params[1] = 0x70;
    params[2] = 0x72;
    params[3] = 0x36;
    params[4] = 0x37;
    params[5] = 0x6F;
    gc9a01a_write_cmd(GC9A01A_GAMMA2); // Set gamma 2
    gc9a01a_write_data(params, 6);

    params[0] = 0x45;
    params[1] = 0x09;
    params[2] = 0x08;
    params[3] = 0x08;
    params[4] = 0x26;
    params[5] = 0x2A;
    gc9a01a_write_cmd(GC9A01A_GAMMA3); // Set gamma 3
    gc9a01a_write_data(params, 6);

    params[0] = 0x43;
    params[1] = 0x70;
    params[2] = 0x72;
    params[3] = 0x36;
    params[4] = 0x37;
    params[5] = 0x6F;
    gc9a01a_write_cmd(GC9A01A_GAMMA4); // Set gamma 4
    gc9a01a_write_data(params, 6);

    params[0] = 0x34;
    gc9a01a_write_cmd(GC9A01A_FRAMERATE); // Frame rate control
    gc9a01a_write_data(params, 1);

    params[0] = 0x38;
    params[1] = 0x0B;
    params[2] = 0x6D;
    params[3] = 0x6D;
    params[4] = 0x39;
    params[5] = 0xF0;
    params[6] = 0x6D;
    params[7] = 0x6D;
    gc9a01a_write_cmd(0x60);
    gc9a01a_write_data(params, 8);

    params[0] = 0x38;
    params[1] = 0xF4;
    params[2] = 0x6D;
    params[3] = 0x6D;
    params[4] = 0x38;
    params[5] = 0xF7;
    params[6] = 0xF7;
    params[7] = 0x6D;
    params[8] = 0x6D;
    gc9a01a_write_cmd(0x61);
    gc9a01a_write_data(params, 9);

    params[0] = 0x38;
    params[1] = 0x0D;
    params[2] = 0x71;
    params[3] = 0xED;
    params[4] = 0x70;
    params[5] = 0x70;
    params[6] = 0x38;
    params[7] = 0x0F;
    params[8] = 0x71;
    params[9] = 0xEF;
    params[10] = 0x70;
    params[11] = 0x70;
    gc9a01a_write_cmd(0x62);
    gc9a01a_write_data(params, 12);

    params[0] = 0x38;
    params[1] = 0x11;
    params[2] = 0x71;
    params[3] = 0xF1;
    params[4] = 0x70;
    params[5] = 0x70;
    params[6] = 0x38;
    params[7] = 0x13;
    params[8] = 0x71;
    params[9] = 0xF3;
    params[10] = 0x70;
    params[11] = 0x70;
    gc9a01a_write_cmd(0x63);
    gc9a01a_write_data(params, 12);

    params[0] = 0x28;
    params[1] = 0x29;
    params[2] = 0xF1;
    params[3] = 0x01;
    params[4] = 0xF1;
    params[5] = 0x00;
    params[6] = 0x07;
    gc9a01a_write_cmd(0x64);
    gc9a01a_write_data(params, 7);

    params[0] = 0x3C;
    params[1] = 0x00;
    params[2] = 0xCD;
    params[3] = 0x67;
    params[4] = 0x45;
    params[5] = 0x45;
    params[6] = 0x10;
    params[7] = 0x00;
    params[8] = 0x00;
    params[9] = 0x00;
    gc9a01a_write_cmd(0x66);
    gc9a01a_write_data(params, 10);

    params[0] = 0x00;
    params[1] = 0x3C;
    params[2] = 0x00;
    params[3] = 0x00;
    params[4] = 0x00;
    params[5] = 0x01;
    params[6] = 0x54;
    params[7] = 0x10;
    params[8] = 0x32;
    params[9] = 0x98;
    gc9a01a_write_cmd(0x67);
    gc9a01a_write_data(params, 10);

    params[0] = 0x10;
    params[1] = 0x80;
    params[2] = 0x80;
    params[3] = 0x00;
    params[4] = 0x00;
    params[5] = 0x4E;
    params[6] = 0x00;
    gc9a01a_write_cmd(0x74);
    gc9a01a_write_data(params, 7);

    params[0] = 0x3E;
    params[1] = 0x07;
    gc9a01a_write_cmd(0x98);
    gc9a01a_write_data(params, 2);

    params[0] = 0x3E;
    params[1] = 0x07;
    gc9a01a_write_cmd(0x99);
    gc9a01a_write_data(params, 2);

    params[0] = 0x00;
    gc9a01a_write_cmd(GC9A01A_TEON);
    gc9a01a_write_data(params, 1);

    gc9a01a_write_cmd(GC9A01A_INVON);
    HAL_Delay(120);
    gc9a01a_write_cmd(GC9A01A_SLPOUT);
    HAL_Delay(120);
    gc9a01a_write_cmd(GC9A01A_DISPON);
    HAL_Delay(20);

    gc9a01a_write_cmd(GC9A01A_RAMWR);

    //GC9A01A_CS_HIGH;
}

/**
  * @brief  Initialize the LCD and its GPIO/SPI interface
  * @param  None
  * @retval None
  */
void bsp_lcd_init(void) {
	GC9A01A_CS_LOW;
    gc9a01a_hw_reset();
    gc9a01a_init();
    gc9a01a_set_orientation(LANDSCAPE);
}

/**
  * @brief  Draw a single pixel on the GC9A01A LCD
  * @param  x: X-coordinate of the pixel
  * @param  y: Y-coordinate of the pixel
  * @param  color: Color of the pixel in RGB565 format
  * @retval None
  */
void gc9a01a_draw_pixel(uint16_t x, uint16_t y, uint16_t color) {
    if ((x >= BSP_LCD_ACTIVE_WIDTH) || (y >= BSP_LCD_ACTIVE_HEIGHT))
        return;

    //GC9A01A_CS_LOW;
    gc9a01a_set_address_window(x, x, y, y);
    uint8_t data[] = { color >> 8, color & 0xFF };
    gc9a01a_write_data(data, sizeof(data));
    //GC9A01A_CS_HIGH;
}

/**
  * @brief  Write a character on the GC9A01A LCD
  * @param  x: Start column address
  * @param  y: Start row address
  * @param  ch: Character to be displayed
  * @param  font: Font definition structure
  * @param  color: Color of the character (RGB565 format)
  * @param  bgcolor: Background color of the character (RGB565 format)
  * @retval None
  */
static void gc9a01a_write_char(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor) {
    uint32_t i, b, j;

    gc9a01a_set_address_window(x, x + font.width - 1, y, y + font.height - 1);

    for (i = 0; i < font.height; i++) {
        b = font.data[(ch - 32) * font.height + i];
        for (j = 0; j < font.width; j++) {
            if ((b << j) & 0x8000) {
                uint8_t data[] = { color >> 8, color & 0xFF };
                gc9a01a_write_data(data, sizeof(data));
            } else {
                uint8_t data[] = { bgcolor >> 8, bgcolor & 0xFF };
                gc9a01a_write_data(data, sizeof(data));
            }
        }
    }
}

/**
  * @brief  Write a string on the GC9A01A LCD
  * @param  x: Start column address
  * @param  y: Start row address
  * @param  str: Pointer to the string to be displayed
  * @param  font: Font definition structure
  * @param  color: Text color (RGB565 format)
  * @param  bgcolor: Background color (RGB565 format)
  * @retval None
  */
void gc9a01a_write_string(uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor) {
    //GC9A01A_CS_LOW;

    while (*str) {
        if (x + font.width >= BSP_LCD_ACTIVE_WIDTH) {
            x = 0;
            y += font.height;
            if (y + font.height >= BSP_LCD_ACTIVE_HEIGHT) {
                break;
            }

            if (*str == ' ') {
                str++;
                continue;
            }
        }

        gc9a01a_write_char(x, y, *str, font, color, bgcolor);
        x += font.width;
        str++;
    }

    //GC9A01A_CS_HIGH;
}

/**
  * @brief  Fills a rectangle on the GC9A01A LCD display with a specified color.
  * @param  x: X-coordinate of the top-left corner of the rectangle.
  * @param  y: Y-coordinate of the top-left corner of the rectangle.
  * @param  w: Width of the rectangle.
  * @param  h: Height of the rectangle.
  * @param  color: Color to fill the rectangle with.
  * @retval None
  */
void gc9a01a_fill_rect(uint16_t x, uint16_t w, uint16_t y, uint16_t h, uint16_t color) {
	if((x >= BSP_LCD_ACTIVE_WIDTH) || (y >= BSP_LCD_ACTIVE_HEIGHT)) return;
	if((x + w - 1) >= BSP_LCD_ACTIVE_WIDTH) w = BSP_LCD_ACTIVE_WIDTH - x;
	if((y + h - 1) >= BSP_LCD_ACTIVE_HEIGHT) h = BSP_LCD_ACTIVE_HEIGHT - y;
    //GC9A01A_CS_LOW;
    gc9a01a_set_address_window(x, x + w - 1, y, y + h - 1);

    uint8_t data[] = { color >> 8, color & 0xFF };
    GC9A01A_DC_DATA;
    for (uint32_t i = 0; i < h; i++) {
        for (uint32_t j = 0; j < w; j++) {
            HAL_SPI_Transmit(&hspi1, data, sizeof(data), HAL_MAX_DELAY);
        }
    }
    //GC9A01A_CS_HIGH;
}

/**
  * @brief  Set the orientation of the GC9A01A LCD display
  * @param  orientation: Orientation mode (LANDSCAPE or PORTRAIT)
  * @retval None
  */
void gc9a01a_set_orientation(uint8_t orientation) {
  uint8_t params[4];
  //GC9A01A_CS_LOW;
  if (orientation == LANDSCAPE) {
    // Set column address for landscape orientation
    gc9a01a_write_cmd(GC9A01A_CASET);
    params[0] = 0x00;
    params[1] = 0x00;
    params[2] = 0x00;
    params[3] = 0xf0;
    gc9a01a_write_data(params, 4);

    // Set row address for landscape orientation
    gc9a01a_write_cmd(GC9A01A_RASET);
    gc9a01a_write_data(params, 4);

    // Set orientation parameters for landscape mode
    params[0] = MADCTL_MV | MADCTL_BGR;
    } else if (orientation == PORTRAIT) {
    // Set column address for portrait orientation
    gc9a01a_write_cmd(GC9A01A_CASET);
    params[0] = 0x00;
    params[1] = 0x00;
    params[2] = 0x00;
    params[3] = 0xf0;
    gc9a01a_write_data(params, 4);

    // Set row address for portrait orientation
    gc9a01a_write_cmd(GC9A01A_RASET);
    gc9a01a_write_data(params, 4);

    // Set orientation parameters for portrait mode
    params[0] =  MADCTL_MX | MADCTL_BGR;
    }

  // Set MADCTL register with the configured parameters
  gc9a01a_write_cmd(GC9A01A_MADCTL);
  gc9a01a_write_data(params, 1);
  //GC9A01A_CS_HIGH;
}

/**
  * @brief  Fill the entire background of the GC9A01A LCD with a specified color
  * @param  color: Color to fill the background (RGB565 format)
  * @retval None
  */
void gc9a01a_fill_screen(uint16_t color) {
    gc9a01a_fill_rect(0, BSP_LCD_ACTIVE_WIDTH, 0, BSP_LCD_ACTIVE_HEIGHT, color);
}

/**
  * @brief  Draw an image on the GC9A01A LCD
  * @param  x: Start column address
  * @param  y: Start row address
  * @param  w: Width of the image
  * @param  h: Height of the image
  * @param  data: Pointer to the image data (RGB565 format)
  * @retval None
  */
//void gc9a01a_draw_image(uint16_t x, uint16_t w, uint16_t y, uint16_t h, const uint16_t* data) {
//    if ((x >= GC9A01A_WIDTH) || (y >= GC9A01A_HEIGHT)) return;
//    if ((x + w - 1) >= GC9A01A_WIDTH) w = GC9A01A_HEIGHT - x;
//    if ((y + h - 1) >= GC9A01A_WIDTH) h = GC9A01A_HEIGHT - y;
//    //GC9A01A_CS_LOW;
//    gc9a01a_set_address_window(x, (x+w-1), y, (y+h-1));
//    GC9A01A_DC_DATA;
//
//    for (uint32_t i = 0; i < w * h; i++) {
//        uint8_t color_high = (data[i] >> 8) & 0xFF;
//        uint8_t color_low = data[i] & 0xFF;
//
//        HAL_SPI_Transmit(&hspi1, &color_high, 1, HAL_MAX_DELAY);
//        HAL_SPI_Transmit(&hspi1, &color_low, 1, HAL_MAX_DELAY);
//
//    }
//    //GC9A01A_CS_HIGH;
//}

void gc9a01a_draw_image(uint16_t x, uint16_t w, uint16_t y, uint16_t h, const uint16_t* data) {
    if ((x >= GC9A01A_WIDTH) || (y >= GC9A01A_HEIGHT)) return;
    if ((x + w - 1) >= GC9A01A_WIDTH) w = GC9A01A_WIDTH - x;
    if ((y + h - 1) >= GC9A01A_HEIGHT) h = GC9A01A_HEIGHT - y;

    // Create byte-swapped buffer for MSB-first order
    uint32_t size = w * h;
    uint8_t* buffer = (uint8_t*)malloc(size * 2);
    if (!buffer) return; // Handle allocation failure
    for (uint32_t i = 0; i < size; i++) {
        buffer[i * 2] = (data[i] >> 8) & 0xFF; // High byte
        buffer[i * 2 + 1] = data[i] & 0xFF;    // Low byte
    }

    //GC9A01A_CS_LOW;
    gc9a01a_set_address_window(x, x + w - 1, y, y + h - 1);
    gc9a01a_write_data(buffer, size * 2);
    //GC9A01A_CS_HIGH;

    free(buffer);
}

/**
  * @brief  Inverts the colors on the GC9A01A LCD display.
  * @param  invert: Boolean flag to determine whether to invert colors.
  * @retval None
  */
void gc9a01a_invert_colors(bool invert) {
    //GC9A01A_CS_LOW;
    uint8_t invert_colors = invert ? GC9A01A_INVON : GC9A01A_INVOFF;
    gc9a01a_write_cmd(invert_colors);
    //GC9A01A_CS_HIGH;
}

/**
  * @brief  Draw a full-scale image on the GC9A01A LCD (240 x 240)
  * @param  data: Pointer to the image data (RGB565 format)
  * @retval None
  */
void gc9a01a_lcd_fill_image(const uint16_t* data) {
    gc9a01a_draw_image(0, BSP_LCD_ACTIVE_WIDTH, 0,  BSP_LCD_ACTIVE_HEIGHT, data);
}
