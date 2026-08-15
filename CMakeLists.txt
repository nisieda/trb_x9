zephyr_library_sources(src/input_processor_rotate.c)
zephyr_library_sources(src/behavior_rotate_step.c)
zephyr_library_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dts/bindings)

# drivers/behavior.h, drivers/input_processor.h, and zmk/behavior.h live in
# ZMK's own app/include, which is only exposed PRIVATE to the "app" CMake
# target. Out-of-tree modules need to add it explicitly.
zephyr_library_include_directories(${APPLICATION_SOURCE_DIR}/include)
