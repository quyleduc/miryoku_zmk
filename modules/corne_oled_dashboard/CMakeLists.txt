# Small, standalone custom ZMK LVGL status screen for SSD1306 128x32.
# All ZMK status changes arrive through built-in event-driven widgets.
zephyr_library()
# ZMK's app headers are PRIVATE to its own app target; expose them to
# this module's Zephyr library explicitly.
zephyr_library_include_directories(${APPLICATION_SOURCE_DIR}/include)
zephyr_library_sources_ifdef(CONFIG_ZMK_DISPLAY_STATUS_SCREEN_CUSTOM
  src/status_screen.c
)
