# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(kystlaget_peripheral)

FILE(GLOB app_sources src/*.c)
FILE(GLOB app_headers include/*.h)
# Conditionally include sensor source
if(NOT CONFIG_USE_SENSOR_ADC)
  list(REMOVE_ITEM app_sources ${CMAKE_CURRENT_SOURCE_DIR}/src/sensor_adc.c)
elseif(NOT CONFIG_USE_SENSOR_BME)
  list(REMOVE_ITEM app_sources ${CMAKE_CURRENT_SOURCE_DIR}/src/sensor_bme.c)
elseif(NOT CONFIG_USE_SENSOR_MPU)
  list(REMOVE_ITEM app_sources ${CMAKE_CURRENT_SOURCE_DIR}/src/sensor_mpu.c)
endif()

target_sources(app PRIVATE
  ${app_sources}
  )