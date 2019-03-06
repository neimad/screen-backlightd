// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright © 2019 Damien Flament
// This file is part of screen-backlightd.

#pragma once

#include <glib-object.h>
#include <glib.h>

/**
 * BacklightDevice:
 * A backlight device.
 */
/**
 * BacklightDevice:brightness:
 *
 * The actual backlight brightness value.
 */
/**
 * BacklightDevice:min-brightness:
 *
 * The minimum backlight brightness value.
 */
/**
 * BacklightDevice:maw-brightness:
 *
 * The maximum backlight brightness value.
 */
G_DECLARE_FINAL_TYPE(
  BacklightDevice, backlight_device, BACKLIGHT, DEVICE, GObject)
#define BACKLIGHT_TYPE_DEVICE backlight_device_get_type()

/**
 * backlight_device_new: (constructor)
 * @syspath: the name of the device within _sysfs_
 *
 * Creates a new backlight device from a _sysfs_ name.
 *
 * Returns: (transfer full): a new #BacklightDevice
 */
BacklightDevice *backlight_device_new(const gchar *sysname);
