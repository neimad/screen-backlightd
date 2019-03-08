// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright Ãƒâ€šÃ‚Â© 2019 Damien Flament
// This file is part of screen-backlightd.

#include "ScreenBacklightManager.h"

#include "BacklightDevice.h"
#include "glib-helpers/glib-object-helpers.h"

#include <libudev.h>

#define SYSFS_SUBSYSTEM "backlight"
#define STEPS_COUNT 10

struct _ScreenBacklightManager
{
  GObject parent;
  BacklightDevice *device;
};

G_DEFINE_TYPE(ScreenBacklightManager, screen_backlight_manager, G_TYPE_OBJECT)

/*
 * discover_first_device:
 *
 * Discovers the first device within the `SYSFS_SUBSYSTEM` subsystem.
 *
 * Returns: (transfer full): a #udev_device
 */
static struct udev_device *
discover_first_device(void)
{
  struct udev *udev                = NULL;
  struct udev_enumerate *enumerate = NULL;
  struct udev_list_entry *entries  = NULL;
  const char *syspath              = NULL;
  struct udev_device *device       = NULL;

  udev = udev_new();

  if (udev == NULL)
  {
    g_error("Failed to create udev context.");
  }

  enumerate = udev_enumerate_new(udev);

  if (enumerate == NULL)
  {
    g_error("Failed to create a udev enumerate object.");
  }

  udev_enumerate_add_match_subsystem(enumerate, SYSFS_SUBSYSTEM);
  udev_enumerate_scan_devices(enumerate);

  entries = udev_enumerate_get_list_entry(enumerate);

  if (entries == NULL)
  {
    g_error("No device found in the `%s` subsystem.", SYSFS_SUBSYSTEM);
  }

  syspath = udev_list_entry_get_name(entries);
  g_info("Managing device %s.", syspath);

  device = udev_device_new_from_syspath(udev, syspath);

  udev_enumerate_unref(enumerate);
  udev_unref(udev);

  return device;
}

static void
screen_backlight_manager_init(ScreenBacklightManager *self G_GNUC_UNUSED)
{
  self->device = backlight_device_new(discover_first_device());
}

static void screen_backlight_manager_dispose(ScreenBacklightManager *self)
{
  g_object_unref(self->device);

  G_OBJECT_CLASS(screen_backlight_manager_parent_class)
    ->dispose(G_OBJECT(self));
}

static void
screen_backlight_manager_class_init(ScreenBacklightManagerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->dispose = (GObjectDisposeFunc) screen_backlight_manager_dispose;
}

ScreenBacklightManager *screen_backlight_manager_new()
{
  ScreenBacklightManager *manager = NULL;

  manager = g_object_new(SCREEN_BACKLIGHT_TYPE_MANAGER, NULL);

  return manager;
}

typedef enum _BrightnessAction
{
  BRIGHTNESS_INCREASE = 1,
  BRIGHTNESS_DECREASE = -1
} BrightnessAction;

/*
 * screen_backlight_manager_set:
 * @self: the manager
 * @action: a brightness action to perform
 *
 * Performs an action on the #BacklightDevice.
 */
static void
brightness_perfom(ScreenBacklightManager *self, BrightnessAction action)
{
  gint maximum      = 0;
  gint minimum      = 0;
  gint value        = 0;
  gint target_value = 0;
  guint step_value  = 0;

  g_return_if_fail(SCREEN_BACKLIGHT_IS_MANAGER(self));

  g_object_get(self->device,
               "min_brightness",
               &minimum,
               "max-brightness",
               &maximum,
               "brightness",
               &value,
               NULL);

  step_value   = (maximum - minimum) / STEPS_COUNT;
  target_value = value + (action * step_value);
  target_value = CLAMP(target_value, minimum, maximum);

  g_object_set(self->device, "brightness", target_value, NULL);
}

void
screen_backlight_manager_increase(ScreenBacklightManager *self)
{
  brightness_perfom(self, BRIGHTNESS_INCREASE);
}

void
screen_backlight_manager_decrease(ScreenBacklightManager *self)
{
  brightness_perfom(self, BRIGHTNESS_DECREASE);
}
