cmake_minimum_required(VERSION 3.27)

set(PICO_BOARD pico2)
# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(controller)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()


add_executable(controller
        main.c
		usb_descriptors.c
        )
target_include_directories(controller PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)
# enable uart over USB
pico_enable_stdio_usb(controller 1)

pico_generate_pio_header(controller ${CMAKE_CURRENT_LIST_DIR}/spi_read_adc.pio)

# Pull in basic dependencies
target_link_libraries(controller pico_stdlib pico_stdio hardware_spi hardware_pio pico_multicore tinyusb_device tinyusb_board)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(controller)
