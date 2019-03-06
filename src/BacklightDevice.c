// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright © 2019 Damien Flament
// This file is part of screen-backlightd.

#include "BacklightDevice.h"

#include "glib-helpers/glib-object-helpers.h"

#include <errno.h>
#include <libudev.h>

#define SYSFS_SUBSYSTEM "backlight"
#define SYSATTR_MAX_BRIGHTNESS_GET "max_brightness"
#define SYSATTR_BRIGHTNESS "brightness"
#define BRIGHTNESS_MIN 0

struct _BacklightDevice
{
  GObject parent;
  struct udev_device *udev_device;
  guint maximum;
};

G_DEFINE_TYPE(BacklightDevice, backlight_device, G_TYPE_OBJECT)

enum {
  PROPERTY_SYSNAME = 1,
  PROPERTY_BRIGHTNESS,
  PROPERTY_MIN_BRIGHTNESS,
  PROPERTY_MAX_BRIGHTNESS,
  N_PROPERTIES
};

static GParamSpec *backlight_device_properties[N_PROPERTIES] = {
  NULL,
};

/*
 * transform_sysattr_uint:
 * @source: the source value
 * @target: the target value
 *
 * Transforms a #gchararray value read from a _sysattr_ to a #guint.
 */
static void transform_sysattr_uint(const GValue *source, GValue *target)
{
  guint64 value     = 0;
  const gchar *nptr = NULL;
  gchar *endptr     = NULL;

  g_return_if_fail(G_VALUE_HOLDS_STRING(source));
  g_return_if_fail(G_VALUE_HOLDS_UINT(target));

  nptr = g_value_get_string(source);

  value = g_ascii_strtoull(nptr, &endptr, 10);
  g_assert(value <= G_MAXUINT);

  if (endptr == nptr)
  {
    g_error("Failed to converts string '%s' to unsigned integer.", nptr);
  }

  g_value_set_uint(target, (guint) value);
}

/*
 * get_udev_device:
 * @sysname: the _udev_ sysname
 *
 * Gives the _udev_ device with the specified @sysname.
 *
 * Returns: (transfer full): a new instance of #udev_device
 */
static struct udev_device *get_udev_device(const gchar *sysname)
{
  struct udev *udev          = NULL;
  struct udev_device *device = NULL;

  g_return_val_if_fail(sysname != NULL, NULL);

  udev = udev_new();

  if (udev == NULL)
  {
    g_error("Failed to create udev context.");
  }

  device =
    udev_device_new_from_subsystem_sysname(udev, SYSFS_SUBSYSTEM, sysname);

  if (device == NULL)
  {
    g_error("Failed to create udev device on subsystem '%s' with sysname '%s'.",
            SYSFS_SUBSYSTEM,
            sysname);
  }

  udev_unref(udev);

  return device;
}

/*
 * get_sysattr_uint_value:
 * @device: a _udev_ device
 * @sysattr: the name of the _sysfs_ attribute to get the value
 * @value: (out caller-allocates): the location to store the value
 *
 * Gets the @value of an _udev_ @device _sysfs_ attribute.
 */
static void get_sysattr_value(struct udev_device *device,
                              const gchar *sysattr,
                              GValue *value)
{
  GValue read_value = G_VALUE_INIT;

  g_return_if_fail(device != NULL);
  g_return_if_fail(sysattr != NULL);
  g_return_if_fail(value != NULL);

  g_value_init(&read_value, G_TYPE_STRING);
  g_value_set_string(&read_value,
                     udev_device_get_sysattr_value(device, sysattr));

  if (g_value_get_string(&read_value) == NULL)
  {
    g_error("Failed to read sysfs attribue '%s' on device '%s'.",
            sysattr,
            udev_device_get_syspath(device));
  }

  g_assert(g_value_type_transformable(G_TYPE_STRING, G_VALUE_TYPE(value)));
  g_value_transform(&read_value, value);
}

/*
 * set_sysattr_uint_value:
 * @device: a _udev_ device
 * @sysattr: the name of the _sysfs_ attribute to set the value
 * @value: the value to set
 *
 * Sets the @value an _udev_ @device _sysfs_ attribute.
 */
static void set_sysattr_value(struct udev_device *device,
                              const gchar *sysattr,
                              const GValue *value)
{
  GValue value_to_write = G_VALUE_INIT;
  gint error_code       = 0;

  g_return_if_fail(device != NULL);
  g_return_if_fail(sysattr != NULL);
  g_return_if_fail(G_VALUE_HOLDS_UINT(value));

  g_value_init(&value_to_write, G_TYPE_STRING);

  g_assert(g_value_type_transformable(G_VALUE_TYPE(value), G_TYPE_STRING));
  g_value_transform(value, &value_to_write);

  error_code =
    udev_device_set_sysattr_value(device,
                                  sysattr,
                                  g_value_get_string(&value_to_write));

  if (error_code < 0)
  {
    g_error("Failed to write value '%s' to attribute '%s' on device '%s': %s.",
            g_value_get_string(&value_to_write),
            sysattr,
            udev_device_get_syspath(device),
            g_strerror(-error_code));
  }
}

static void backlight_device_init(BacklightDevice *self G_GNUC_UNUSED)
{
  self->maximum = 0;

  g_value_register_transform_func(G_TYPE_STRING,
                                  G_TYPE_UINT,
                                  transform_sysattr_uint);
  g_assert(g_value_type_transformable(G_TYPE_STRING, G_TYPE_UINT));
}

static void backlight_device_constructed(BacklightDevice *self)
{
  GParamSpecUInt *brightness_property = NULL;

  brightness_property =
    G_PARAM_SPEC_UINT(backlight_device_properties[PROPERTY_BRIGHTNESS]);
  g_assert(G_IS_PARAM_SPEC_UINT(brightness_property));

  // Initialize the brightness allowed range and default value
  g_object_get(self,
               "min-brightness",
               &brightness_property->minimum,
               "max-brightness",
               &brightness_property->maximum,
               "brightness",
               &brightness_property->default_value,
               NULL);

  G_OBJECT_CLASS(backlight_device_parent_class)->constructed(G_OBJECT(self));
}

static void backlight_device_dispose(BacklightDevice *self)
{
  g_clear_pointer(&self->udev_device, udev_device_unref);

  G_OBJECT_CLASS(backlight_device_parent_class)->dispose(G_OBJECT(self));
}

static void backlight_device_get_property(BacklightDevice *self,
                                          guint property_id,
                                          GValue *value,
                                          GParamSpec *spec)
{
  switch (property_id)
  {
    case PROPERTY_BRIGHTNESS:
      g_assert(self->udev_device != NULL);

      get_sysattr_value(self->udev_device, SYSATTR_BRIGHTNESS, value);
      break;

    case PROPERTY_MIN_BRIGHTNESS:
      g_value_set_uint(value, BRIGHTNESS_MIN);
      break;

    case PROPERTY_MAX_BRIGHTNESS:
      g_assert(self->udev_device != NULL);

      if (self->maximum == 0)
      {
        get_sysattr_value(self->udev_device, SYSATTR_MAX_BRIGHTNESS_GET, value);
        self->maximum = g_value_get_uint(value);
      }
      else
      {
        g_value_set_uint(value, self->maximum);
      }
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(self, property_id, spec);
  }
}

static void backlight_device_set_property(BacklightDevice *self,
                                          guint property_id,
                                          const GValue *value,
                                          GParamSpec *spec)
{
  switch (property_id)
  {
    case PROPERTY_SYSNAME:
      g_assert(self->udev_device == NULL);

      self->udev_device = get_udev_device(g_value_get_string(value));
      break;

    case PROPERTY_BRIGHTNESS:
      g_assert(self->udev_device != NULL);

      set_sysattr_value(self->udev_device, SYSATTR_BRIGHTNESS, value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(self, property_id, spec);
  }
}

static void backlight_device_class_init(BacklightDeviceClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->constructed =
    (GObjectConstructedFunc) backlight_device_constructed;
  object_class->finalize = (GObjectFinalizeFunc) backlight_device_dispose;
  object_class->set_property =
    (GObjectSetPropertyFunc) backlight_device_set_property;
  object_class->get_property =
    (GObjectGetPropertyFunc) backlight_device_get_property;

  backlight_device_properties[PROPERTY_SYSNAME] =
    g_param_spec_string("sysname",
                        "Sysfs name",
                        "Device sysfs name",
                        NULL,
                        G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE);
  backlight_device_properties[PROPERTY_BRIGHTNESS] =
    g_param_spec_uint("brightness",
                      "Actual brightness",
                      "Actual backlight brightness value",
                      0,
                      G_MAXUINT,
                      0,
                      G_PARAM_READWRITE);
  backlight_device_properties[PROPERTY_MIN_BRIGHTNESS] =
    g_param_spec_uint("min-brightness",
                      "Minimum brightness",
                      "Minimum backlight brightness value",
                      0,
                      G_MAXUINT,
                      0,
                      G_PARAM_READABLE);
  backlight_device_properties[PROPERTY_MAX_BRIGHTNESS] =
    g_param_spec_uint("max-brightness",
                      "Maximum brightness",
                      "Maximum backlight brightness value",
                      0,
                      G_MAXUINT,
                      0,
                      G_PARAM_READABLE);

  g_object_class_install_properties(object_class,
                                    N_PROPERTIES,
                                    backlight_device_properties);
}

BacklightDevice *backlight_device_new(const gchar *sysname)
{
  g_return_val_if_fail(sysname != NULL, NULL);

  return g_object_new(BACKLIGHT_TYPE_DEVICE, "sysname", sysname, NULL);
}
