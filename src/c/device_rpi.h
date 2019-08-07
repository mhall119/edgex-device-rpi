/*
 * Copyright (c) 2018
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef _DEVICE_RPI_H_
#define _DEVICE_RPI_H_ 1

#include <stdio.h>
#include <pthread.h>

#include <mraa/gpio.h>
#include <mraa/aio.h>
#include <mraa/i2c.h>
#include "edgex/edgex.h"
#include "edgex/devsdk.h"
#include "edgex/device-mgmt.h"

#if defined (__cplusplus)
extern "C" {
#endif


#define RPI_ERR_CHECK(x) if (x.code) { fprintf (stderr, "Error: %d: %s\n", x.code, x.reason); return x.code; }
#define RPI_NO_GPIO_PINS 7
#define RPI_NO_AIO_PINS 0
#define RPI_NO_I2C_PINS 0
#define RPI_NO_PORTS RPI_NO_GPIO_PINS + RPI_NO_AIO_PINS + RPI_NO_I2C_PINS

#define RPI_I2C_BUS 0

#define RPI_ROTARY_MAX_ANGLE 300
#define RPI_ADC_REF 5

#define RPI_SVC "Device-RPi"

typedef enum
{
  RPI_GPIO    = 0,
  RPI_PWM     = 1,
  RPI_AIO     = 2,
  RPI_I2C     = 3,
  RPI_SERIAL  = 4
} rpi_interface_type_t;

typedef struct
{
  char *pin_no;
  char *pin_type;
  char *type;
  bool normalize;
} rpi_attributes_t;

typedef struct
{
  bool is_lcd;
  mraa_i2c_context dev;
  mraa_i2c_context rgb_dev;
} rpi_i2c_dev_ctxt_t;

typedef struct
{
  rpi_interface_type_t pin_type;
  char *pin_number;
  void *dev_ctxt;
} rpi_dev_ctxt_t;

typedef struct
{
  iot_logger_t *lc;
  edgex_device_service *svc;
  rpi_dev_ctxt_t *dev[RPI_NO_PORTS];
  pthread_mutex_t mutex;
} rpi_pidriver_t;

#if defined (__cplusplus)
}
#endif
#endif
