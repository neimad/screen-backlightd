cmake_minimum_required(VERSION 3.12)

project(screen-backlightd
        DESCRIPTION "A screen backlight daemon"
        HOMEPAGE_URL "https://github.com/neimad/screen-backlightd"
        VERSION 0.1
        LANGUAGES C)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake-modules")
include(TargetModules)

require_module(glib-2.0 MINIMUM 2.58)
require_module(gobject-2.0 MINIMUM 2.58)
require_module(gio-unix-2.0 MINIMUM 2.58)
require_module(libudev MINIMUM 240)

add_subdirectory(dbus)
add_subdirectory(src)
add_subdirectory(systemd)
