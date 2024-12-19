project(ADS1115_emulation)

add_executable(${PROJECT_NAME})
# must match with executable name and source file names
target_sources(${PROJECT_NAME} PRIVATE ads1115.c)
# pull in common dependencies
target_link_libraries(${PROJECT_NAME} pico_stdlib hardware_gpio hardware_i2c pico_i2c_slave hardware_adc)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(${PROJECT_NAME})

# enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(${PROJECT_NAME})