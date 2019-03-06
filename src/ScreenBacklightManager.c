// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright © 2019 Damien Flament
// This file is part of screen-backlightd.

#include "ScreenBacklightManager.h"

#include "BacklightDevice.h"
#include "glib-helpers/glib-object-helpers.h"

#define DEVICE_SYSNAME "intel_backlight"
#define STEPS_COUNT 10

struct _ScreenBacklightManager
{
  GObject parent;
  BacklightDevice *device;
};

G_DEFINE_TYPE(ScreenBacklightManager, screen_backlight_manager, G_TYPE_OBJECT)

static void
screen_backlight_manager_init(ScreenBacklightManager *self G_GNUC_UNUSED)
{
  self->device = backlight_device_new(DEVICE_SYSNAME);
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

static void screen_backlight_manager_set(ScreenBacklightManager *self,
                                         gint8 direction)
{
  gint maximum      = 0;
  gint minimum      = 0;
  gint value        = 0;
  gint target_value = 0;
  guint step_value  = 0;

  g_return_if_fail(SCREEN_BACKLIGHT_IS_MANAGER(self));

  direction = CLAMP(direction, -1, 1);

  g_object_get(self->device,
               "min_brightness",
               &minimum,
               "max-brightness",
               &maximum,
               "brightness",
               &value,
               NULL);

  step_value   = (maximum - minimum) / STEPS_COUNT;
  target_value = value + (direction * step_value);
  target_value = CLAMP(target_value, minimum, maximum);

  g_object_set(self->device, "brightness", target_value, NULL);
}

void screen_backlight_manager_increase(ScreenBacklightManager *self)
{
  screen_backlight_manager_set(self, +1);
}

void screen_backlight_manager_decrease(ScreenBacklightManager *self)
{
  screen_backlight_manager_set(self, -1);
}
