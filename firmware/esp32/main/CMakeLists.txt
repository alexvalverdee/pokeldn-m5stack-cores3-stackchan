idf_build_get_property(idf_target IDF_TARGET)
if(idf_target STREQUAL "esp32s3" OR idf_target STREQUAL "esp32c3" OR idf_target STREQUAL "esp32c6")
    set(wire_driver esp_driver_usb_serial_jtag esp_driver_uart)   # either socket answers (wire.c)
elseif(idf_target STREQUAL "esp32s2")
    # The S2 uses USB OTG with TinyUSB, resolved by the component manifest.
elseif(idf_target STREQUAL "esp32")
    set(wire_driver esp_driver_uart)
else()
    message(FATAL_ERROR "pokeldn supports esp32, esp32s2, esp32s3, esp32c3 and esp32c6 targets only")
endif()

set(POKELDN_DISPLAY "$ENV{POKELDN_DISPLAY}")
if(idf_target STREQUAL "esp32s2" AND NOT POKELDN_DISPLAY)
    set(POKELDN_DISPLAY NONE)
elseif(NOT POKELDN_DISPLAY)
    set(POKELDN_DISPLAY OLED)
endif()
if(POKELDN_DISPLAY STREQUAL "E32R40T" AND idf_target STREQUAL "esp32")
    set(panel_source "panel_spi.c")
    set(panel_driver esp_driver_spi)
elseif(POKELDN_DISPLAY STREQUAL "SZPI" AND idf_target STREQUAL "esp32s3")
    set(panel_source "panel_spi.c" "board_szpi.c" "speaker_es8311.c" "sfx.c")
    set(panel_driver esp_driver_spi esp_driver_i2c esp_driver_i2s)
    set(speaker 1)
elseif(POKELDN_DISPLAY STREQUAL "STACKCHAN" AND idf_target STREQUAL "esp32s3")
    set(panel_source "panel_stackchan.cpp")
elseif(POKELDN_DISPLAY STREQUAL "NONE" AND idf_target STREQUAL "esp32s2")
    set(panel_source "panel_none.c")
elseif(POKELDN_DISPLAY STREQUAL "OLED" AND NOT idf_target STREQUAL "esp32s2")
    set(panel_source "panel_oled.c")
    set(panel_driver esp_driver_i2c)
else()
    message(FATAL_ERROR "POKELDN_DISPLAY=${POKELDN_DISPLAY}: OLED, E32R40T on esp32, SZPI/STACKCHAN on esp32s3, or NONE on esp32s2")
endif()

if(DEFINED ENV{POKELDN_BLE})
    if(idf_target STREQUAL "esp32s2")
        message(FATAL_ERROR "ESP32-S2 does not have Bluetooth")
    endif()
    set(ble_component bt)
endif()

idf_component_register(
    SRCS "radio.c" "advert.c" "beacon.c" "gift_ldn.c" "gift_pia.c" "gift_link.c" "gift_mg.c" "gift_zstd.c" "ble.c" "wire.c" "led.c" "usbwatch.c" "display.c" ${panel_source} "scene.c" "screen.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES esp_app_format esp_wifi wpa_supplicant ${wire_driver} esp_driver_ledc esp_driver_gpio ${panel_driver} esp_timer nvs_flash esp_event mbedtls zstd ${ble_component})

string(TOLOWER "${POKELDN_DISPLAY}" display_name)
if(POKELDN_DISPLAY STREQUAL "STACKCHAN")
    set(display_name "szpi")
endif()

file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/../version.txt" fw_version)
if(DEFINED ENV{POKELDN_FIRMWARE_VERSION})
    set(fw_version "$ENV{POKELDN_FIRMWARE_VERSION}")
endif()

target_compile_definitions(${COMPONENT_LIB} PRIVATE POKELDN_DISPLAY_${POKELDN_DISPLAY}
                           POKELDN_DISPLAY_NAME="${display_name}"
                           POKELDN_FIRMWARE_VERSION="${fw_version}")
if(speaker)
    target_compile_definitions(${COMPONENT_LIB} PRIVATE POKELDN_SPEAKER=1)
endif()

if(DEFINED ENV{POKELDN_USB_BEACON})
    target_compile_definitions(${COMPONENT_LIB} PRIVATE POKELDN_USB_BEACON=1)
endif()

if(DEFINED ENV{POKELDN_BLE})
    target_compile_definitions(${COMPONENT_LIB} PRIVATE POKELDN_BLE=1)
endif()