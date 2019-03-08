// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright © 2019 Damien Flament
// This file is part of screen-backlightd.

#include "DBusControls.h"
#include "ScreenBacklightManager.h"
#include "config.h"

#include <glib-unix.h>

typedef struct _Application
{
  GMainLoop *loop;
  ScreenBacklightManager *manager;
  DBusControls *controls;

} Application;

void
init_application(Application *app)
{
  GError *error = NULL;

  g_return_if_fail(app != NULL);

  app->manager = screen_backlight_manager_new();
  g_info("Created manager.");

  app->controls =
    dbus_controls_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                         G_DBUS_PROXY_FLAGS_NONE,
                                         CONTROLD_DBUS_BUS_NAME,
                                         CONTROLD_DBUS_OBJECT_PATH,
                                         NULL,
                                         &error);
  if (app->controls == NULL)
  {
    g_error("Failed to create the D-Bus proxy: %s.", error->message);
    g_error_free(error);
  }

  g_message("Listening on screen backlight controls events on object %s in bus "
            "name %s...",
            CONTROLD_DBUS_OBJECT_PATH,
            CONTROLD_DBUS_BUS_NAME);
  g_signal_connect_swapped(app->controls,
                           "screen-backlight-increase",
                           G_CALLBACK(screen_backlight_manager_increase),
                           app->manager);
  g_signal_connect_swapped(app->controls,
                           "screen-backlight-decrease",
                           G_CALLBACK(screen_backlight_manager_decrease),
                           app->manager);
}

void
quit_application(Application *app)
{
  g_return_if_fail(app != NULL);

  g_message("Exiting...");

  g_info("Releasing resources...");

  g_clear_object(&app->controls);
  g_info("Destroyed D-Bus proxy.");

  g_clear_object(&app->manager);
  g_info("Destroyed screen backlight manager.");

  g_main_loop_unref(app->loop);
  g_main_loop_quit(app->loop);
}

int
main(void)
{
  Application app = {NULL};

  g_unix_signal_add(SIGHUP, G_SOURCE_FUNC(quit_application), &app);
  g_unix_signal_add(SIGINT, G_SOURCE_FUNC(quit_application), &app);
  g_unix_signal_add(SIGTERM, G_SOURCE_FUNC(quit_application), &app);
  g_info("Installed Unix signals handler.");

  app.loop = g_main_loop_new(NULL, FALSE);

  init_application(&app);

  g_main_loop_run(app.loop);
}
