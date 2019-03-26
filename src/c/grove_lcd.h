/*
 * Copyright (c) 2018
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef _GROVE_LCD_H_
#define _GROVE_LCD_H_ 1

#include <mraa/i2c.h>
#include <stdbool.h>

#if defined (__cplusplus)
extern "C" {
#endif

/* LCD - Hitachi HD44780 driver */
#define GROVE_LCD_ADDR 0x3e
#define GROVE_RGB_ADDR 0x62

mraa_result_t grove_lcd_i2c_command (const mraa_i2c_context dev, uint8_t cmd);

mraa_result_t grove_lcd_i2c_data (const mraa_i2c_context dev, uint8_t cmd);

mraa_result_t
grove_lcd_set_cursor (const mraa_i2c_context dev, uint8_t row, uint8_t column);

mraa_result_t grove_lcd_display_on (const mraa_i2c_context dev, bool on);

mraa_result_t grove_lcd_cleardisplay (const mraa_i2c_context dev);

mraa_result_t grove_lcd_hd44780_init (const mraa_i2c_context lcd_dev,
                                      const mraa_i2c_context rgb_dev);

mraa_result_t grove_rgb_backlight_on (const mraa_i2c_context rgb_dev, bool on);

mraa_result_t
grove_rgb_set_color (const mraa_i2c_context rgb_dev, uint8_t r, uint8_t g,
                     uint8_t b);

mraa_result_t
grove_lcd_write (const mraa_i2c_context dev, char *buffer, int len);

#if defined (__cplusplus)
}
#endif
#endif
