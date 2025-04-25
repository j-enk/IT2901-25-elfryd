cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(saadc_battery_monitor)

target_sources(app PRIVATE src/main.c src/adc_driver.c)
