zephyr_library()
target_sources(app PRIVATE led.c)
zephyr_library_include_directories(${ZEPHYR_BASE}/drivers)
