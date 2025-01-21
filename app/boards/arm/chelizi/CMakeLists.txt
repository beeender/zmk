zephyr_library()
zephyr_library_sources(pinmux.c)
target_sources(app PRIVATE led.c)
zephyr_library_include_directories(${ZEPHYR_BASE}/drivers)
