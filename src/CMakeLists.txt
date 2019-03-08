include(TargetModules)
include(GNUInstallDirs)

set(target ${PROJECT_NAME})
set(units ScreenBacklightManager BacklightDevice)

configure_file(config.h.in config.h)

add_library(${target}-units STATIC ${units})
target_required_modules(${target}-units MODULES gobject-2.0)
target_include_directories(${target}-units BEFORE PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
                                                         ${CMAKE_CURRENT_BINARY_DIR}/../dbus)

add_executable(${target} main.c)
target_required_modules(${target} MODULES libudev)
target_link_libraries(${target} ${target}-units ${target}-dbus)

install(TARGETS ${target} DESTINATION ${CMAKE_INSTALL_BINDIR})
