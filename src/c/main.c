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

#include "device_rpi.h"

static bool stop = false;

static void rpi_inthandler (int i)
{
  stop = true;
}

static rpi_attributes_t *get_rpiattributes (const edgex_nvpairs *device_attr)
{
  rpi_attributes_t *rpi_attr = (rpi_attributes_t *) malloc (sizeof (rpi_attributes_t));
  for (; device_attr != NULL; device_attr = device_attr->next)
  {
    if (strcmp (device_attr->name, "Pin_Num") == 0)
    {
      rpi_attr->pin_no = device_attr->value;
    }
    else if (strcmp (device_attr->name, "Interface") == 0)
    {
      rpi_attr->pin_type = device_attr->value;
    }
    else if (strcmp (device_attr->name, "Type") == 0)
    {
      rpi_attr->type = device_attr->value;
    }
    else if (strcmp (device_attr->name, "normalize") == 0)
    {
      rpi_attr->normalize = (strcasecmp (device_attr->value, "true") == 0);
    }
  }
  return rpi_attr;
}

static rpi_dev_ctxt_t * rpi_device_lookup (rpi_pidriver_t *impln, char *pin)
{
  for (int index = 0; index < RPI_NO_PORTS; index++)
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

static rpi_dev_ctxt_t *rpi_set_devctxt
  (rpi_pidriver_t *impln, void *dev, char *pin, rpi_interface_type_t interface_type)
{
  int index = 0;
  rpi_dev_ctxt_t *mraa_dev = NULL;

  for (; index < RPI_NO_PORTS; index++)
  {
    if (impln->dev[index] == NULL)
    {
      mraa_dev = malloc (sizeof (rpi_dev_ctxt_t));
      if (strstr (pin, "I2C"))
      {
        rpi_i2c_dev_ctxt_t *i2c_dev = malloc (sizeof (rpi_i2c_dev_ctxt_t));
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

static mraa_result_t rpi_gpio_init (rpi_pidriver_t *impln, char *pin, char *type)
{
  mraa_result_t status = MRAA_SUCCESS;
  rpi_interface_type_t pin_type = RPI_GPIO;

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
    if ((rpi_set_devctxt (impln, (void *) dev, pin, pin_type)) == NULL)
    {
      iot_log_error (impln->lc, "Unable to set the mraadev_ctxt at %s", pin, type);
      status = MRAA_ERROR_UNSPECIFIED;
    }
  }
  return status;
}

static bool rpi_init (void *impl, struct iot_logger_t *lc, const edgex_nvpairs *config)
{
  mraa_result_t status = MRAA_SUCCESS;
  rpi_pidriver_t *impln = (rpi_pidriver_t *) impl;
  impln->lc = lc;
  pthread_mutex_init (&impln->mutex, NULL);

  iot_log_debug (lc, "driver initialization");
  {
    edgex_deviceprofile *profiles = NULL;

    status = mraa_init ();
    if (status != MRAA_SUCCESS)
    {
      iot_log_error (lc, "RaspberryPi driver initialization failed");
      return false;
    }

    //mraa_add_subplatform (MRAA_RPIPI, "0");

    /* read the attributes from the device profile to initialize the driver */
    profiles = edgex_device_profiles (impln->svc);

    while (profiles)
    {
      edgex_deviceresource *dev_obj = profiles->device_resources;
      rpi_attributes_t *rpi_attr = NULL;
      for (; dev_obj != NULL; dev_obj = dev_obj->next)
      {
        edgex_nvpairs *dev_attr = dev_obj->attributes;
        assert (dev_attr != NULL);

        rpi_attr = get_rpiattributes (dev_attr);

        rpi_dev_ctxt_t *dev = rpi_device_lookup (impln, rpi_attr->pin_no);
        if (dev != NULL)
        {
          /* device is initialized */
          continue;
        }
        else
        {
          if (strcmp (rpi_attr->pin_type, "GPIO") == 0)
          {
            status = rpi_gpio_init (impln, rpi_attr->pin_no, rpi_attr->type);
            assert (!status);
          }
          else
          {
            /* PWM & Serial interface support not implemented */
            status = MRAA_ERROR_FEATURE_NOT_IMPLEMENTED;
          }
        }
        free (rpi_attr);
      }
      profiles = profiles->next;
    }
    free (profiles);
  }

  return (status == MRAA_SUCCESS);
}

static bool rpi_gethandler
  (
    void *impl,
    const char *devname,
    const edgex_protocols *protocols,
    uint32_t nreadings,
    const edgex_device_commandrequest *requests,
    edgex_device_commandresult *readings
  )
  {
  rpi_pidriver_t *impln = (rpi_pidriver_t *) impl;

  pthread_mutex_lock (&impln->mutex);
  const edgex_nvpairs *dev_attr = requests->attributes;
  assert (dev_attr != NULL);
  rpi_attributes_t *rpi_attr = get_rpiattributes (dev_attr);
  bool ret_status = true;

  rpi_dev_ctxt_t *mraa_devctxt = rpi_device_lookup (impln, rpi_attr->pin_no);
  if (mraa_devctxt != NULL)
  {
    volatile int read_value;
    if (strcmp (rpi_attr->pin_type, "GPIO") == 0)
    {
      mraa_gpio_context gpio_dev = (mraa_gpio_context) mraa_devctxt->dev_ctxt;
      read_value = mraa_gpio_read (gpio_dev);

      assert (nresults == 1);

      if (read_value == -1)
      {
        /* error */
        ret_status = false;       
      }
         /* RaspberryPi Button */
      else if (requests->type == Uint8)
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
    else
    {
      /* Only GPIO types are supported */
      ret_status = false;
    }
  } /* dev_ctxt != NULL */
  free (rpi_attr);
  pthread_mutex_unlock (&impln->mutex);
  return ret_status;
}

static bool rpi_puthandler
  (
    void *impl,
    const char *devname,
    const edgex_protocols *protocols,
    uint32_t nvalues,
    const edgex_device_commandrequest *requests,
    const edgex_device_commandresult *readings
  )
{
  mraa_result_t status = MRAA_SUCCESS;
  rpi_pidriver_t *impln = (rpi_pidriver_t *) impl;

  pthread_mutex_lock (&impln->mutex);
  /* Get the device context */
  const edgex_nvpairs *dev_attr = requests->attributes;
  assert (dev_attr != NULL);
  rpi_attributes_t *rpi_attr = get_rpiattributes (dev_attr);

  rpi_dev_ctxt_t *mraa_devctxt = rpi_device_lookup (impln, rpi_attr->pin_no);
  if (mraa_devctxt != NULL)
  {
    if (strcmp (rpi_attr->pin_type, "GPIO") == 0)
    {
      mraa_gpio_context gpio_dev = (mraa_gpio_context) mraa_devctxt->dev_ctxt;

      assert (!requests->type == Bool);
      assert (nvalues == 1);

      status = mraa_gpio_write (gpio_dev, readings[--nvalues].value.bool_result);
    }
    else
    {
      /* PWM & Serial interface support not implemented */
      status = MRAA_ERROR_FEATURE_NOT_IMPLEMENTED;
    }
  }
  free (rpi_attr);
  pthread_mutex_unlock (&impln->mutex);
  return (status == MRAA_SUCCESS);
 }

static bool rpi_disconnect (void *impl, edgex_protocols *device)
{
  free (impl);
  return true;
}

static void rpi_stop (void *impl, bool force)
{
  rpi_pidriver_t *impln = (rpi_pidriver_t *) impl;
  mraa_result_t status = MRAA_SUCCESS;

  iot_log_debug (impln->lc, "rpi stop call");

  /* Release the resources */
  for (int index = 0; index < RPI_NO_PORTS; index++)
  {
    if (impln->dev[index] != NULL)
    {
      switch (impln->dev[index]->pin_type)
      {
        case RPI_GPIO:
        {
          mraa_gpio_context gpio_dev = (mraa_gpio_context) impln->dev[index]->dev_ctxt;
          status |= mraa_gpio_close (gpio_dev);
          break;
        }
        default: 
        {
          status |= MRAA_ERROR_FEATURE_NOT_IMPLEMENTED;
          iot_log_error (impln->lc, "rpi_stop(), interface type %d not implemented", impln->dev[index]->pin_type);
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
    iot_log_error (impln->lc, "rpi_stop() failure");
  }
  mraa_deinit ();
  pthread_mutex_destroy (&impln->mutex);
}

static void usage (void)
{
  printf ("Options: \n");
  printf ("   -h, --help           : Show this text\n");
  printf ("   -n, --name=<name>    : Set the device service name\n");
  printf ("   -r, --registry=<url> : Use the registry service\n");
  printf ("   -p, --profile=<name> : Set the profile name\n");
  printf ("   -c, --confdir=<dir>  : Set the configuration directory\n");
}

static bool testArg (int argc, char *argv[], int *pos, const char *pshort, const char *plong, char **var)
{
  if (strcmp (argv[*pos], pshort) == 0 || strcmp (argv[*pos], plong) == 0)
  {
    if (*pos < argc - 1)
    {
      (*pos)++;
      *var = argv[*pos];
      (*pos)++;
      return true;
    }
    else
    {
      printf ("Option %s requires an argument\n", argv[*pos]);
      exit (0);
    }
  }
  char *eq = strchr (argv[*pos], '=');
  if (eq)
  {
    if (strncmp (argv[*pos], pshort, eq - argv[*pos]) == 0 || strncmp (argv[*pos], plong, eq - argv[*pos]) == 0)
    {
      if (strlen (++eq))
      {
        *var = eq;
        (*pos)++;
        return true;
      }
      else
      {
        printf ("Option %s requires an argument\n", argv[*pos]);
        exit (0);
      }
    }
  }
  return false;
}

int main (int argc, char *argv[])
{
  char *profile = "";
  char *confdir = "";
  char *svcname = "device-rpi";
  char *regURL = getenv ("EDGEX_REGISTRY");
  edgex_error err;

  rpi_pidriver_t * implObject = malloc (sizeof (rpi_pidriver_t));
  memset (implObject, 0, sizeof (rpi_pidriver_t));

  int n = 1;
  while (n < argc)
  {
    if (strcmp (argv[n], "-h") == 0 || strcmp (argv[n], "--help") == 0)
    {
      usage ();
      return 0;
    }
    if (testArg (argc, argv, &n, "-r", "--registry", &regURL))
    {
      continue;
    }
    if (testArg (argc, argv, &n, "-n", "--name", &svcname))
    {
      continue;
    }
    if (testArg (argc, argv, &n, "-p", "--profile", &profile))
    {
      continue;
    }
    if (testArg (argc, argv, &n, "-c", "--confdir", &confdir))
    {
      continue;
    }

    printf ("Unknown option %s\n", argv[n]);
    usage ();
    return 0;
  }

  err.code = 0;

  edgex_device_callbacks myImpls =
  {
    rpi_init,
    NULL,
    rpi_gethandler,
    rpi_puthandler,
    rpi_disconnect,
    rpi_stop
  };

  edgex_device_service *rpi_service = edgex_device_service_new (RPI_SVC, "1.0", implObject, myImpls, &err);
  RPI_ERR_CHECK (err);

  implObject->svc = rpi_service;
  edgex_device_service_start (rpi_service, regURL, profile, confdir, &err);
  RPI_ERR_CHECK (err);

  printf ("\nRunning - press ctrl-c to exit\n");
  signal (SIGTERM, rpi_inthandler);
  signal (SIGINT, rpi_inthandler);

  while (!stop)
  {
    sleep (1);
  }

  edgex_error e;
  e.code = 0;
  edgex_device_service_stop (rpi_service, true, &e);
  RPI_ERR_CHECK (e);

  free (implObject);
  return 0;
}
