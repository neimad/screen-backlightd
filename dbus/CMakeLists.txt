find_package(GDBusCodegen)
include(TargetModules)

set(CONTROLD_DBUS_BUS_NAME "org.neimad.control1"
    CACHE STRING "controld D-Bus well-known bus name")
set(CONTROLD_DBUS_OBJECT_PATH "/org/neimad/control1"
    CACHE STRING "controld D-Bus object path")
mark_as_advanced(CONTROLD_DBUS_BUS_NAME
                 CONTROLD_DBUS_OBJECT_PATH)

set(interface "${CONTROLD_DBUS_BUS_NAME}.Controls.xml")
set(interface_url "https://github.com/neimad/controld/raw/master/dist/${interface}")
set(interface_path "${CMAKE_CURRENT_BINARY_DIR}/${interface}")

file(DOWNLOAD "${interface_url}" "${interface_path}"
     EXPECTED_HASH MD5=fddacfcc95ed4e62994da0cecbb48eed
     TLS_VERIFY ON
     INACTIVITY_TIMEOUT 5
     SHOW_PROGRESS)

gdbus_generate_code("${interface_path}"
                    PREFIX ${CONTROLD_DBUS_BUS_NAME}
                    NAMESPACE DBus
                    PRAGMA_ONCE)

add_library(screen-backlightd-dbus STATIC DBusControls.c)
target_required_modules(screen-backlightd-dbus PUBLIC gio-unix-2.0)
target_include_directories(screen-backlightd-dbus INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
target_compile_options(screen-backlightd-dbus PRIVATE -Wno-unused-parameter)
