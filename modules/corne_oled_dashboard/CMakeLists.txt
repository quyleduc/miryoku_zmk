# Small, standalone custom ZMK LVGL status screen for SSD1306 128x32.
# All ZMK status changes arrive through built-in event-driven widgets.
zephyr_library()
zephyr_library_sources_ifdef(CONFIG_ZMK_DISPLAY_STATUS_SCREEN_CUSTOM
  src/status_screen.c
)
