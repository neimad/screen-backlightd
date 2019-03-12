// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright © 2019 Damien Flament
// This file is part of screen-backlightd.

#pragma once

#include <glib-object.h>

/**
 * ScreenBacklightManager:
 * A manager to set the screen backlight brightness.
 */
G_DECLARE_FINAL_TYPE(ScreenBacklightManager,
                     screen_backlight_manager,
                     SCREEN_BACKLIGHT,
                     MANAGER,
                     GObject);
#define SCREEN_BACKLIGHT_TYPE_MANAGER screen_backlight_manager_get_type()

/**
 * screen_backlight_manager_new: (constructor)
 *
 * Creates a screen backlight manager.
 *
 * Returns: (transfer full): a new #ScreenBacklightManager
 */
ScreenBacklightManager *screen_backlight_manager_new(void);

/**
 * screen_backlight_manager_increase: (method)
 * @manager: the manager
 *
 * Increases the screen backlight brightness by one step.
 */
void screen_backlight_manager_increase(ScreenBacklightManager *manager);

/**
 * screen_backlight_manager_decrease: (method)
 * @manager: the manager
 *
 * Decreases the screen backlight brightness by one step.
 */
void screen_backlight_manager_decrease(ScreenBacklightManager *manager);
