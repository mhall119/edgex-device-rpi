/*
 * Copyright (c) 2018
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

/* Based on code from https://github.com/intel-iot-devkit/upm/src/jhd1313m1.c */
/*
 * Copyright (c) 2016 Intel Corporation.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "grove_lcd.h"
#include "grove_hd44780.h"

#include <assert.h>
#include <unistd.h>

mraa_result_t grove_lcd_i2c_command (const mraa_i2c_context dev, uint8_t cmd)
{
  return mraa_i2c_write_byte_data (dev, cmd, GROVE_HD44780_CMD);
}

mraa_result_t grove_lcd_i2c_data (const mraa_i2c_context dev, uint8_t cmd)
{
  return mraa_i2c_write_byte_data (dev, cmd, GROVE_HD44780_DATA);
}

mraa_result_t
grove_lcd_set_cursor (const mraa_i2c_context dev, uint8_t row, uint8_t column)
{
  column = column % 16;
  uint8_t offset = column;

  // this should work for any display with two rows
  // DDRAM mapping:
  // 00 .. 27
  // 40 .. 67
  offset += row * 0x40;

  return grove_lcd_i2c_command (dev, GROVE_HD44780_CMD | offset);
}

mraa_result_t grove_lcd_cleardisplay (const mraa_i2c_context dev)
{
  mraa_result_t ret;
  ret = grove_lcd_i2c_command (dev, GROVE_HD44780_CLEARDISPLAY);
  usleep (2000);

  return ret;
}

mraa_result_t grove_lcd_display_on (const mraa_i2c_context dev, bool on)
{
  uint8_t displaycontrol = GROVE_HD44780_DISPLAYON;
  if (on)
  {
    displaycontrol |= GROVE_HD44780_DISPLAYON;
  }
  else
  {
    displaycontrol &= ~GROVE_HD44780_DISPLAYON;
  }

  return grove_lcd_i2c_command (dev,
                                GROVE_HD44780_DISPLAYCONTROL | displaycontrol);
}

mraa_result_t grove_rgb_backlight_on (const mraa_i2c_context rgb_dev, bool on)
{
  mraa_result_t rv = MRAA_SUCCESS;
  if (on)
  {
    rv = mraa_i2c_write_byte_data (rgb_dev, 0xaa, 0x08);
  }
  else
  {
    rv = mraa_i2c_write_byte_data (rgb_dev, 0x00, 0x08);
  }

  return rv;
}

mraa_result_t
grove_rgb_set_color (const mraa_i2c_context rgb_dev, uint8_t r, uint8_t g,
                     uint8_t b)
{
  mraa_result_t rv = MRAA_SUCCESS;
  rv = mraa_i2c_write_byte_data (rgb_dev, 0, 0);
  assert ((!rv));

  rv = mraa_i2c_write_byte_data (rgb_dev, 0, 1);
  assert ((!rv));

  rv = mraa_i2c_write_byte_data (rgb_dev, r, 0x04);
  assert ((!rv));

  rv = mraa_i2c_write_byte_data (rgb_dev, g, 0x03);
  assert ((!rv));

  rv = mraa_i2c_write_byte_data (rgb_dev, b, 0x02);
  assert ((!rv));

  return rv;
}

mraa_result_t grove_lcd_write (const mraa_i2c_context dev, char *buffer,
                               int len)
{
  int i;
  mraa_result_t rv = MRAA_SUCCESS;

  for (i = 0; i < len; ++i)
  {
    rv = grove_lcd_i2c_data (dev, buffer[i]);
  }
  return rv;
}

mraa_result_t grove_lcd_hd44780_init (const mraa_i2c_context lcd_dev,
                                      const mraa_i2c_context rgb_dev)
{
  /* HD44780 requires writing three times to initialize or reset
      according to the hardware errata on page 45 figure 23 of
      the Hitachi HD44780 datasheet */
  mraa_result_t status = MRAA_SUCCESS;

  /* First try */
  usleep (50000);
  status = grove_lcd_i2c_command (lcd_dev,
                                  GROVE_HD44780_FUNCTIONSET |
                                  GROVE_HD44780_8BITMODE);
  assert ((!status));

  /* Second try */
  usleep (4500);
  status = grove_lcd_i2c_command (lcd_dev,
                                  GROVE_HD44780_FUNCTIONSET |
                                  GROVE_HD44780_8BITMODE);
  assert ((!status));

  /* Third try */
  usleep (150);
  status = grove_lcd_i2c_command (lcd_dev,
                                  GROVE_HD44780_FUNCTIONSET |
                                  GROVE_HD44780_8BITMODE);
  assert ((!status));

  /* Set 2 row mode and font size */
  status = grove_lcd_i2c_command (lcd_dev,
                                  GROVE_HD44780_FUNCTIONSET |
                                  GROVE_HD44780_8BITMODE |
                                  GROVE_HD44780_2LINE |
                                  GROVE_HD44780_5x10DOTS);
  assert ((!status));
  usleep (100);

  status = grove_lcd_display_on (lcd_dev, true);
  assert ((!status));

  status = grove_lcd_cleardisplay (lcd_dev);
  usleep (2000);

  status = grove_lcd_i2c_command (lcd_dev,
                                  GROVE_HD44780_ENTRYMODESET |
                                  GROVE_HD44780_ENTRYLEFT |
                                  GROVE_HD44780_ENTRYSHIFTDECREMENT);
  assert ((!status));

  status = grove_rgb_backlight_on (rgb_dev, true);
  assert ((!status));

  status = grove_rgb_set_color (rgb_dev, 0xff, 0xff, 0xff);
  assert ((!status));

  return status;
}
