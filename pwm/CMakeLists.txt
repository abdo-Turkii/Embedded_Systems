project(freq)

add_executable(${PROJECT_NAME})

# enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# must match with executable name and source file names
target_sources(${PROJECT_NAME} PRIVATE freq.c)
# pull in common dependencies
target_link_libraries(${PROJECT_NAME} pico_stdlib hardware_pwm hardware_dma)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(${PROJECT_NAME})
