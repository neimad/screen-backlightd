include(TargetModules)
include(GNUInstallDirs)

configure_file(config.h.in config.h)

add_library(screen-backlightd-units STATIC ScreenBacklightManager.c
                                           BacklightDevice.c)
target_required_modules(screen-backlightd-units PUBLIC gobject-2.0
                                                PRIVATE libudev)

add_executable(screen-backlightd main.c)
target_required_modules(screen-backlightd PRIVATE glib-2.0)
target_include_directories(screen-backlightd PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(screen-backlightd screen-backlightd-units screen-backlightd-dbus)

install(TARGETS screen-backlightd DESTINATION ${CMAKE_INSTALL_BINDIR})
