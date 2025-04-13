zephyr_library()

if (CONFIG_BOARD_CHELIZI_EINK)
    #zephyr_library_sources_ifdef(CONFIG_ZMK_DISPLAY custom_status_screen.c)
else ()
    target_sources(app PRIVATE led.c)
endif()
target_sources(app PRIVATE patch.c)
zephyr_library_include_directories(${ZEPHYR_BASE}/drivers)
