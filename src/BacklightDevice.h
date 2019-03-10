// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright © 2019 Damien Flament
// This file is part of screen-backlightd.

#pragma once

#include <glib-object.h>
#include <libudev.h>

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
 * BacklightDevice:max-brightness:
 *
 * The maximum backlight brightness value.
 */
G_DECLARE_FINAL_TYPE(
  BacklightDevice, backlight_device, BACKLIGHT, DEVICE, GObject);
#define BACKLIGHT_TYPE_DEVICE backlight_device_get_type()

/**
 * backlight_device_new: (constructor)
 * @device: a _udev_ device
 *
 * Creates a new backlight device from a _udev_ device.
 *
 * Returns: (transfer full): a new #BacklightDevice
 */
BacklightDevice *backlight_device_new(struct udev_device *device);
