/*
 * Copyright (c) 2018
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
#include <stdbool.h>
#include <assert.h>
#include <signal.h>
#include <math.h>

#include "device_grove.h"
#include "grove_lcd.h"

static bool stop = false;

static void grove_inthandler (int i)
{
  stop = true;
}

static void usage (void)
{
  printf ("Options: \n");
  printf ("   -h, --help           : Show this text\n");
  printf ("   -r, --registry       : Use the registry service\n");
  printf ("   -p, --profile <name> : Set the profile name\n");
  printf ("   -c, --confdir <dir>  : Set the configuration directory\n");
}

static grove_attributes_t *get_groveattributes (edgex_nvpairs *device_attr)
{
  grove_attributes_t *grove_attr = (grove_attributes_t *) malloc (sizeof (grove_attributes_t));
  for (; device_attr != NULL; device_attr = device_attr->next)
  {
    if (strcmp (device_attr->name, "Pin_Num") == 0)
    {
      grove_attr->pin_no = device_attr->value;
    }
    else if (strcmp (device_attr->name, "Interface") == 0)
    {
      grove_attr->pin_type = device_attr->value;
    }
    else if (strcmp (device_attr->name, "Type") == 0)
    {
      grove_attr->type = device_attr->value;
    }
    else if (strcmp (device_attr->name, "normalize") == 0)
    {
      grove_attr->normalize = (strcasecmp (device_attr->value, "true") == 0);
    }
  }
  return grove_attr;
}

static grove_dev_ctxt_t * grove_device_lookup (grove_pidriver_t *impln, char *pin)
{
  for (int index = 0; index < GROVE_NO_PORTS; index++)
  {
    if (impln->dev[index] != NULL)
    {
      if (strcmp (impln->dev[index]->pin_number, pin) == 0)
      {
        return impln->dev[index];
      }
    }
    else
    {
      break;
    }
  }
  return NULL;
}

static grove_dev_ctxt_t *grove_set_devctxt
  (grove_pidriver_t *impln, void *dev, char *pin, grove_interface_type_t interface_type)
{
  int index = 0;
  grove_dev_ctxt_t *mraa_dev = NULL;

  for (; index < GROVE_NO_PORTS; index++)
  {
    if (impln->dev[index] == NULL)
    {
      mraa_dev = malloc (sizeof (grove_dev_ctxt_t));
      if (strstr (pin, "I2C"))
      {
        grove_i2c_dev_ctxt_t *i2c_dev = malloc (sizeof (grove_i2c_dev_ctxt_t));
        i2c_dev->dev = dev;
        mraa_dev->dev_ctxt = (void *) i2c_dev;
      }
      else
      {
        mraa_dev->dev_ctxt = dev;
      }
      mraa_dev->pin_number = malloc (strlen (pin));
      strcpy (mraa_dev->pin_number, pin);
      mraa_dev->pin_type = interface_type;

      impln->dev[index] = mraa_dev;
      break;
    }
  }
  return mraa_dev;
}

static mraa_result_t grove_gpio_init (grove_pidriver_t *impln, char *pin, char *type)
{
  mraa_result_t status = MRAA_SUCCESS;
  grove_interface_type_t pin_type = GROVE_GPIO;

  //int pin_number = GROVE_SUBPLATFORM_OFFSET + (pin[strlen (pin) - 1] - '0');
  int pin_number = atoi(pin);
  iot_log_error(impln->lc, "Initializing pin %d", pin_number);
  mraa_gpio_context dev = mraa_gpio_init (pin_number);
  if (dev == NULL)
  {
    status = MRAA_ERROR_UNSPECIFIED;
    iot_log_error (impln->lc, "Failed to initialize a device at GPIO %s", pin);
  }
  else
  {
    if (strcmp (type, "OUT") == 0)
    {
      status = mraa_gpio_dir (dev, MRAA_GPIO_OUT);
    }
    else if (strcmp (type, "IN") == 0)
    {
      status = mraa_gpio_dir (dev, MRAA_GPIO_IN);
    }
    else
    {
      /* IN/OUT Configuration not supported - ignore */
    }
  }

  if (status != MRAA_SUCCESS)
  {
    iot_log_error (impln->lc, "Failed to set the GPIO %s to %s mode", pin, type);
  }
  else
  {
    if ((grove_set_devctxt (impln, (void *) dev, pin, pin_type)) == NULL)
    {
      iot_log_error (impln->lc, "Unable to set the mraadev_ctxt at %s", pin, type);
      status = MRAA_ERROR_UNSPECIFIED;
    }
  }
  return status;
}

static mraa_result_t grove_aio_init (grove_pidriver_t *impln, char *pin)
{
  mraa_result_t status = MRAA_SUCCESS;
  grove_interface_type_t pin_type = GROVE_AIO;

  int pin_number = GROVE_SUBPLATFORM_OFFSET + (pin[strlen (pin) - 1] - '0');
  mraa_aio_context dev = mraa_aio_init (pin_number);

  if (dev == NULL)
  {
    status = MRAA_ERROR_UNSPECIFIED;
    iot_log_error (impln->lc, "Failed to initialize a device at AIO %s", pin);
  }
  else
  {
    if ((grove_set_devctxt (impln, (void *) dev, pin, pin_type)) == NULL)
    {
      iot_log_error (impln->lc, "Unable to set the mraadev_ctxt at %s", pin);
      status = MRAA_ERROR_UNSPECIFIED;
    }
  }
  return status;
}

static mraa_result_t grove_lcd_init (grove_pidriver_t *impln, char *pin)
{
  /* Initialize I2C bus for LCD */
  mraa_i2c_context lcd_dev = NULL;
  mraa_i2c_context rgb_dev = NULL;
  mraa_result_t status = MRAA_SUCCESS;

  if (!(lcd_dev = mraa_i2c_init (GROVE_I2C_BUS)))
  {
    status = MRAA_ERROR_UNSPECIFIED;
    iot_log_error (impln->lc, "mraa_i2c_init(LCD) connected to %s failed\n", pin);
    return status;
  }
  else
  {
    status = mraa_i2c_address (lcd_dev, GROVE_LCD_ADDR);
    if (status != MRAA_SUCCESS)
    {
      iot_log_error (impln->lc, "mraa_i2c_address(LCD) connected to %s failed, status = %d\n", pin, status);
      mraa_i2c_stop (lcd_dev);
      return status;
    }
  }

  /* Initialize I2C bus for Backlight */
  if (!(rgb_dev = mraa_i2c_init (GROVE_I2C_BUS)))
  {
    status = MRAA_ERROR_UNSPECIFIED;
    iot_log_error (impln->lc, "mraa_i2c_init(RGB) connected to %s failed\n", pin);
    return status;
  }
  else
  {
    status = mraa_i2c_address (rgb_dev, GROVE_RGB_ADDR);
    if (status != MRAA_SUCCESS)
    {
      iot_log_error (impln->lc, "mraa_i2c_address(RGB) connected to %s failed, status = %d\n", pin, status);
      mraa_i2c_stop (rgb_dev);
      return status;
    }
  }

  status = grove_lcd_hd44780_init (lcd_dev, rgb_dev);
  if (status == MRAA_SUCCESS)
  {
    /* store the lcd_dev context */
    grove_interface_type_t pin_type = GROVE_I2C;
    grove_dev_ctxt_t *lcd_ctx = grove_set_devctxt (impln, (void *) lcd_dev, pin, pin_type);
    if (lcd_ctx == NULL)
    {
      iot_log_error (impln->lc, "Unable to set the mraadev_ctxt at %s", pin);
      status = MRAA_ERROR_UNSPECIFIED;
    }
    else
    {
      grove_i2c_dev_ctxt_t *i2c_ctx = (grove_i2c_dev_ctxt_t *) lcd_ctx->dev_ctxt;
      i2c_ctx->rgb_dev = rgb_dev;
      i2c_ctx->is_lcd = true;
    }
  }
  return status;
}

static mraa_result_t grove_i2c_init (grove_pidriver_t *impln, char *pin, char *type)
{
  mraa_result_t status = MRAA_SUCCESS;

  if (strcmp (type, "LCD") == 0)
  {
    status = grove_lcd_init (impln, pin);
  }
  else
  {
    status = MRAA_ERROR_UNSPECIFIED;
    iot_log_warning (impln->lc, "Invalid Type, Ignore I2C initialization");
  }
  return status;
}

static bool grove_init (void *impl, struct iot_logging_client *lc, const edgex_nvpairs *config)
{
  mraa_result_t status = MRAA_SUCCESS;
  grove_pidriver_t *impln = (grove_pidriver_t *) impl;
  impln->lc = lc;
  pthread_mutex_init (&impln->mutex, NULL);

  iot_log_debug (lc, "driver initialization");
  {
    uint32_t nprofiles = 0;
    edgex_deviceprofile *profiles = NULL;

    status = mraa_init ();
    if (status != MRAA_SUCCESS)
    {
      iot_log_error (lc, "GrovePI driver initialization failed");
      return false;
    }

    //mraa_add_subplatform (MRAA_GROVEPI, "0");

    /* read the attributes from the device profile to initialize the driver */
    edgex_device_service_getprofiles (impln->svc, &nprofiles, &profiles);

    while (nprofiles--)
    {
      edgex_deviceobject *dev_obj = profiles[nprofiles].device_resources;
      grove_attributes_t *grove_attr = NULL;
      for (; dev_obj != NULL; dev_obj = dev_obj->next)
      {
        edgex_nvpairs *dev_attr = dev_obj->attributes;
        assert (dev_attr != NULL);

        grove_attr = get_groveattributes (dev_attr);

        grove_dev_ctxt_t *dev = grove_device_lookup (impln, grove_attr->pin_no);
        if (dev != NULL)
        {
          /* device is initialized */
          continue;
        }
        else
        {
          if (strcmp (grove_attr->pin_type, "GPIO") == 0)
          {
            status = grove_gpio_init (impln, grove_attr->pin_no, grove_attr->type);
            assert (!status);
          }
          else if (strcmp (grove_attr->pin_type, "AIO") == 0)
          {
            status = grove_aio_init (impln, grove_attr->pin_no);
            assert (!status);
          }
          else if (strcmp (grove_attr->pin_type, "I2C") == 0)
          {
            status = grove_i2c_init (impln, grove_attr->pin_no, grove_attr->type);
            assert (!status);
          }
          else
          {
            /* PWM & Serial interface support not implemented */
            status = MRAA_ERROR_FEATURE_NOT_IMPLEMENTED;
          }
        }
        free (grove_attr);
      }
    }
    free (profiles);
  }

  return (status == MRAA_SUCCESS);
}

static bool grove_gethandler
(
  void *impl,
  const edgex_addressable *devaddr,
  uint32_t nresults,
  const edgex_device_commandrequest *requests,
  edgex_device_commandresult *readings
)
{
  grove_pidriver_t *impln = (grove_pidriver_t *) impl;

  pthread_mutex_lock (&impln->mutex);
  edgex_nvpairs *dev_attr = requests[0].devobj->attributes;
  assert (dev_attr != NULL);
  grove_attributes_t *grove_attr = get_groveattributes (dev_attr);
  bool ret_status = true;

  grove_dev_ctxt_t *mraa_devctxt = grove_device_lookup (impln, grove_attr->pin_no);
  if (mraa_devctxt != NULL)
  {
    volatile int read_value;
    if (strcmp (grove_attr->pin_type, "GPIO") == 0)
    {
      mraa_gpio_context gpio_dev = (mraa_gpio_context) mraa_devctxt->dev_ctxt;
      read_value = mraa_gpio_read (gpio_dev);

      assert (nresults == 1);

      if (read_value == -1)
      {
        /* error */
        ret_status = false;       
      }
         /* Grove Button */
      else if (strcmp (requests->devobj->properties->value->type, "Uint8") == 0)
      {
        readings->value.ui8_result = (uint8_t) read_value;
        readings->type = Uint8;
      }
      else
      {
        /* No other type support available for GPIO in the profile */
        ret_status = false;
      }
    } /* GPIO */
    else if (strcmp (grove_attr->pin_type, "AIO") == 0)
    {
      mraa_aio_context aio_dev = (mraa_aio_context) mraa_devctxt->dev_ctxt;
      read_value = mraa_aio_read (aio_dev);
      if (read_value == -1)
      {
        /* error */
        ret_status = false;    
      }
      else
      {
        // get adc bit range
        int16_t range = (1 << mraa_aio_get_bit (aio_dev)) - 1;
        if (strcmp (requests->devobj->name, "SoundIntensity") == 0)
        {
          assert (nresults == 1);
          assert (!strcmp (requests->devobj->properties->value->type, "Float32"));
          readings->type = Float32;
          if (grove_attr->normalize)
          {
            readings->value.f32_result = (float) read_value * GROVE_ADC_REF / range;
          }
          else
          {
            readings->value.f32_result = (float) read_value;
          }
        }
        else if (strcmp (requests->devobj->name, "LightIntensity") == 0)
        {
          assert (nresults == 1);
          assert (!strcmp (requests->devobj->properties->value->type, "Float32"));
          /* Ref: https://github.com/intel-iot-devkit/upm/src/light/light.c */
          readings->value.f32_result = (float) (10000.0 / powf ((((float) (range) - read_value) * 10.0 / read_value) * 15.0, 4.0 / 3.0));
          readings->type = Float32;
        }
        else
        {
          assert (nresults == 2); /* For RotarySensorMeasurements */
          for (int index = 0; (requests[index].devobj != NULL && index < nresults); index++)
          {
            readings[index].type = Float32;
            if (strcmp ((requests[index].devobj)->name, "RotaryAngle") == 0)
            {
              if (grove_attr->normalize)
              {
                readings[index].value.f32_result = read_value * (float) GROVE_ROTARY_MAX_ANGLE / range;
              }
              else
              {
                readings[index].value.f32_result = read_value;
              }
            }
            else if (strcmp ((requests[index].devobj)->name, "RotaryVoltage") == 0)
            {
              if (grove_attr->normalize)
              {
                readings[index].value.f32_result = read_value * (float) GROVE_ADC_REF / range;
              }
              else
              {
                readings[index].value.f32_result = read_value;
              }
            }
          }
        }
      }
    } /* AIO */
    else
    {
      /* Only GPIO, AIO and I2C interface types are supported */
      ret_status = false;
    }
  } /* dev_ctxt != NULL */
  free (grove_attr);
  pthread_mutex_unlock (&impln->mutex);
  return ret_status;
}

static bool grove_puthandler
(
  void *impl,
  const edgex_addressable *devaddr,
  uint32_t nrequests,
  const edgex_device_commandrequest *requests,
  const edgex_device_commandresult *readings
)
{
  mraa_result_t status = MRAA_SUCCESS;
  grove_pidriver_t *impln = (grove_pidriver_t *) impl;

  pthread_mutex_lock (&impln->mutex);
  /* Get the device context */
  edgex_nvpairs *dev_attr = requests[0].devobj->attributes;
  assert (dev_attr != NULL);
  grove_attributes_t *grove_attr = get_groveattributes (dev_attr);

  grove_dev_ctxt_t *mraa_devctxt = grove_device_lookup (impln, grove_attr->pin_no);
  if (mraa_devctxt != NULL)
  {
    if (strcmp (grove_attr->pin_type, "GPIO") == 0)
    {
      mraa_gpio_context gpio_dev = (mraa_gpio_context) mraa_devctxt->dev_ctxt;

      assert (!strcmp (requests->devobj->properties->value->type, "Bool"));
      assert (nrequests == 1);

      status = mraa_gpio_write (gpio_dev, readings[--nrequests].value.bool_result);
    }
    else if (strcmp (grove_attr->pin_type, "I2C") == 0)
    {
      assert (nrequests == 3); /* for lcd */

      grove_i2c_dev_ctxt_t *i2c_dev = (grove_i2c_dev_ctxt_t *) (mraa_devctxt->dev_ctxt);
      mraa_i2c_context mraa_i2cdev = (mraa_i2c_context) (i2c_dev->dev);

      uint8_t column = 0;
      uint8_t row = 0;
      char *display_string = NULL;

      for (int index = 0; (requests[index].devobj != NULL && index < nrequests); index++)
      {
        struct edgex_deviceobject *devobj = (struct edgex_deviceobject *) requests[index].devobj;
        if (strcmp (devobj->name, "Display-String") == 0)
        {
          display_string = strdup (readings[index].value.string_result);
        }
        else if (strcmp (devobj->name, "Row") == 0)
        {
          row = (readings[index]).value.ui8_result;
        }
        else if (strcmp (devobj->name, "Column") == 0)
        {
          column = (readings[index]).value.ui8_result;
        }
      }
      status |= grove_lcd_set_cursor (mraa_i2cdev, row, column);
      status |= grove_lcd_write (mraa_i2cdev, display_string, strlen (display_string));

      free (display_string);
    }
    else
    {
      /* PWM & Serial interface support not implemented */
      status = MRAA_ERROR_FEATURE_NOT_IMPLEMENTED;
    }
  }
  free (grove_attr);
  pthread_mutex_unlock (&impln->mutex);
  return (status == MRAA_SUCCESS);
 }

static bool grove_disconnect (void *impl, edgex_addressable *device)
{
  free (impl);
  return true;
}

static void grove_stop (void *impl, bool force)
{
  grove_pidriver_t *impln = (grove_pidriver_t *) impl;
  mraa_result_t status = MRAA_SUCCESS;

  iot_log_debug (impln->lc, "grove stop call");

  /* Release the resources */
  for (int index = 0; index < GROVE_NO_PORTS; index++)
  {
    if (impln->dev[index] != NULL)
    {
      switch (impln->dev[index]->pin_type)
      {
        case GROVE_GPIO:
        {
          mraa_gpio_context gpio_dev = (mraa_gpio_context) impln->dev[index]->dev_ctxt;
          status |= mraa_gpio_close (gpio_dev);
          break;
        }
        case GROVE_AIO:
        {
          mraa_aio_context aio_dev = (mraa_aio_context) impln->dev[index]->dev_ctxt;
          status |= mraa_aio_close (aio_dev);
          break;
        }
        case GROVE_I2C:
        {
          grove_i2c_dev_ctxt_t *i2c_dev = (grove_i2c_dev_ctxt_t *) impln->dev[index]->dev_ctxt;
          if (i2c_dev->is_lcd)
          {
            grove_lcd_cleardisplay (i2c_dev->dev);
            grove_rgb_backlight_on (i2c_dev->rgb_dev, false);

            status |= mraa_i2c_stop (i2c_dev->rgb_dev);
            i2c_dev->rgb_dev = NULL;
            i2c_dev->is_lcd = false;
          }
          status |= mraa_i2c_stop (i2c_dev->dev);
          break;
        }
        default: 
        {
          status |= MRAA_ERROR_FEATURE_NOT_IMPLEMENTED;
          iot_log_error (impln->lc, "grove_stop(), interface type %d not implemented", impln->dev[index]->pin_type);
          break;
        }
      }
      free (impln->dev[index]->pin_number);
      free (impln->dev[index]);
      impln->dev[index] = NULL;

    } /* dev != NULL */
  }

  if (status != MRAA_SUCCESS)
  {
    iot_log_error (impln->lc, "grove_stop() failure");
  }
  mraa_deinit ();
  pthread_mutex_destroy (&impln->mutex);
}

int main (int argc, char *argv[])
{
  const char *profile = "";
  char *confdir = "";
  edgex_error err;
  bool useRegistry = false;

  grove_pidriver_t *implObject = malloc (sizeof (grove_pidriver_t));
  memset (implObject, 0, sizeof (grove_pidriver_t));
  implObject->lc = iot_log_default;

  int n = 1;
  while (n < argc)
  {
    if (strcmp (argv[n], "-h") == 0 || strcmp (argv[n], "--help") == 0)
    {
      usage ();
      return 0;
    }
    if (strcmp (argv[n], "-r") == 0 || strcmp (argv[n], "--registry") == 0)
    {
      useRegistry = true;
      n++;
      continue;
    }
    if (strcmp (argv[n], "-p") == 0 || strcmp (argv[n], "--profile") == 0)
    {
      profile = argv[n + 1];
      n += 2;
      continue;
    }
    if (strcmp (argv[n], "-c") == 0 || strcmp (argv[n], "--confdir") == 0)
    {
      confdir = argv[n + 1];
      n = n + 2;
      continue;
    }

    printf ("Unknown option %s\n", argv[n]);
    usage ();
    return 0;
  }

  err.code = 0;

  edgex_device_callbacks myImpls =
  {
    grove_init,
    NULL,
    grove_gethandler,
    grove_puthandler,
    grove_disconnect,
    grove_stop
  };

  edgex_device_service *grove_service = edgex_device_service_new (GROVE_SVC, "1.0", implObject, myImpls, &err);
  GROVE_ERR_CHECK (err);

  implObject->svc = grove_service;
  edgex_device_service_start (grove_service, useRegistry, NULL, 0, profile, confdir, &err);
  GROVE_ERR_CHECK (err);

  printf ("\nRunning - press ctrl-c to exit\n");
  signal (SIGTERM, grove_inthandler);
  signal (SIGINT, grove_inthandler);

  while (!stop)
  {
    sleep (1);
  }

  edgex_error e;
  e.code = 0;
  edgex_device_service_stop (grove_service, true, &e);
  GROVE_ERR_CHECK (e);

  free (implObject);
  return 0;
}
