/*
 * Copyright (c) 2018
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

/* Based on code from https://github.com/intel-iot-devkit/upm/src/lcm1602/hd44780_bits.h */
/*
 * Copyright (c) 2015 Intel Corporation.
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

#ifndef _GROVE_HD44780_H_
#define _GROVE_HD44780_H_ 1


#if defined (__cplusplus)
extern "C" {
#endif

// This file contains bit definitions for the HD44780 and compatible
// LCD controllers.  It is used primarily by the lcm1602 driver and
// it's derivatives.
//

// commands
const uint8_t GROVE_HD44780_CLEARDISPLAY = 0x01;
const uint8_t GROVE_HD44780_ENTRYMODESET = 0x04;
const uint8_t GROVE_HD44780_DISPLAYCONTROL = 0x08;
const uint8_t GROVE_HD44780_FUNCTIONSET = 0x20;

// flags for display entry mode
const uint8_t GROVE_HD44780_ENTRYLEFT = 0x02;
const uint8_t GROVE_HD44780_ENTRYSHIFTDECREMENT = 0x00;

// flags for display on/off control
const uint8_t GROVE_HD44780_DISPLAYON = 0x04;
const uint8_t GROVE_HD44780_DISPLAYOFF = 0x00;

// flags for function set
const uint8_t GROVE_HD44780_8BITMODE = 0x10;
const uint8_t GROVE_HD44780_2LINE = 0x08;
const uint8_t GROVE_HD44780_5x10DOTS = 0x04;

// impln specific
const uint8_t GROVE_HD44780_DATA = 0x40;
const uint8_t GROVE_HD44780_CMD = 0x80;

#if defined (__cplusplus)
}
#endif
#endif

