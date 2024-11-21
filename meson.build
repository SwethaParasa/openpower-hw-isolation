# SPDX-License-Identifier: Apache-2.0

project('hardware_isolation', 'cpp', 'c',
    default_options: [
      'buildtype=debugoptimized',
      'warning_level=3',
      'cpp_std=c++23',
      'werror=true',
    ],
    meson_version: '>=1.1.1',
    version: '1.0.0',
)

# openbmc dependency packages list

cpp = meson.get_compiler('cpp')

libdtapi = dependency('libdt-api')
libguard = cpp.find_library('libguard')
libpdbg = meson.get_compiler('c').find_library('libpdbg')
phosphor_dbus_interfaces = dependency('phosphor-dbus-interfaces')
phosphor_logging = dependency('phosphor-logging')
sdbusplus = dependency('sdbusplus')
sdeventplus = dependency('sdeventplus')
cereal = dependency('cereal', required: false)
has_cereal = cpp.has_header_symbol(
    'cereal/cereal.hpp',
    'cereal::specialize',
    dependencies: cereal,
    required: false)
if not has_cereal
    cereal_opts = import('cmake').subproject_options()
    cereal_opts.add_cmake_defines({'BUILD_TESTS': 'OFF'})
    cereal_proj = import('cmake').subproject(
        'cereal',
        options: cereal_opts,
        required: false)
    assert(cereal_proj.found(), 'cereal is required')
    cereal = cereal_proj.dependency('cereal')
endif

# Configuration header file(config.h) generation

conf_data = configuration_data()

conf_data.set_quoted('HW_ISOLATION_BUSNAME', get_option('HW_ISOLATION_BUSNAME'),
                      description : 'The D-Bus busname to own for hardware isolation'
                    )

conf_data.set_quoted('HW_ISOLATION_OBJPATH', get_option('HW_ISOLATION_OBJPATH'),
                      description : 'The hardware isolation D-Bus root'
                    )

conf_data.set_quoted('PHAL_DEVTREE', get_option('PHAL_DEVTREE'),
                     description : 'The PHAL CEC device tree to get hardware details'
                    )

conf_data.set_quoted('HW_ISOLATION_ENTRY_OBJPATH', get_option('HW_ISOLATION_ENTRY_OBJPATH'),
                      description : 'The hardware isolation dbus entry object path'
                    )

configure_file(configuration : conf_data,
               output : 'config.h'
              )

hardware_isolation_sources = [
        'src/hardware_isolation_main.cpp',
        'src/common/error_log.cpp',
        'src/common/isolatable_hardwares.cpp',
        'src/common/phal_devtree_utils.cpp',
        'src/common/utils.cpp',
        'src/common/watch.cpp',
        'src/hw_isolation_event/event.cpp',
        'src/hw_isolation_event/hw_status_manager.cpp',
        'src/hw_isolation_event/openpower_hw_status.cpp',
        'src/hw_isolation_record/entry.cpp',
        'src/hw_isolation_record/manager.cpp',
        'src/hw_isolation_record/openpower_guard_interface.cpp'
    ]

hardware_isolation_dependencies = [
        libdtapi,
        libguard,
        libpdbg,
        phosphor_dbus_interfaces,
        phosphor_logging,
        sdbusplus,
        sdeventplus,
        cereal
    ]

root_inc_dir = include_directories('include')

executable('openpower-hw-isolation',
           hardware_isolation_sources,
           dependencies: hardware_isolation_dependencies,
           include_directories: root_inc_dir,
           install : true
          )

systemd_system_unit_dir = dependency('systemd').get_variable(
    pkgconfig: 'systemdsystemunitdir')

hw_isolation_service_unit = 'service_files/org.open_power.HardwareIsolation.service'
configure_file(
    configuration: {
        'BUSNAME' : get_option('HW_ISOLATION_BUSNAME'),
    },
    input: hw_isolation_service_unit,
    install : true,
    install_dir: systemd_system_unit_dir,
    output: '@PLAINNAME@'
    )

subdir('src')
subdir('isolation')
