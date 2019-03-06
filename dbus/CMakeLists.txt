find_package(GDBusCodegen)
include(TargetModules)

set(target ${PROJECT_NAME}-dbus)
set(interface "${CONTROLD_DBUS_BUS_NAME}.Controls.xml")
set(interface_url "https://github.com/neimad/kbdfnd/raw/master/dist/${interface}")
set(interface_path "${CMAKE_CURRENT_BINARY_DIR}/${interface}")

file(DOWNLOAD "${interface_url}" "${interface_path}"
     EXPECTED_HASH MD5=7165c3f271f0ea9a306d7fd74c28c52c
     TLS_VERIFY ON
     INACTIVITY_TIMEOUT 5
     SHOW_PROGRESS)

gdbus_generate_code("${interface_path}"
                    PREFIX ${CONTROLD_DBUS_BUS_NAME}
                    NAMESPACE DBus
                    PRAGMA_ONCE)

add_library(${target} STATIC DBusControls.c)
target_required_modules(${target} MODULES gio-unix-2.0)
target_compile_options(${target} PUBLIC -Wno-unused-parameter)
