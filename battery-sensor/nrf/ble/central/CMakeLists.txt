cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(central)

# Main application sources
target_sources(app PRIVATE
  src/main.c
  src/peripheral_manager.c
  src/connection_pool.c
  src/scheduler.c
)

# Include directories
zephyr_library_include_directories(
  ${ZEPHYR_BASE}/samples/bluetooth
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)