cxx = meson.get_compiler('cpp')

sdbusplus = dependency(
    'sdbusplus',
    fallback: [
        'sdbusplus',
        'sdbusplus_dep'
    ],  
)

libdtapi = dependency('libdt-api')
libguard = cpp.find_library('libguard')
libpdbg = meson.get_compiler('c').find_library('libpdbg')
phosphor_logging = dependency(
    'phosphor-logging',
    fallback: ['phosphor-logging', 'phosphor_logging_dep'],
    )

executable(
        'sysguard',
        'create_pel.cpp',
	'systemguard.cpp',
    dependencies: [ sdbusplus, phosphor_logging, libdtapi, libguard, libpdbg],
	install:true,
)
