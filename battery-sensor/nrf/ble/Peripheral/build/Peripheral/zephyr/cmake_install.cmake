# Install script for directory: C:/ncs/v2.9.0/zephyr

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files/Zephyr-Kernel")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "MinSizeRel")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "C:/ncs/toolchains/b620d30767/opt/zephyr-sdk/arm-zephyr-eabi/bin/arm-zephyr-eabi-objdump.exe")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/zephyr/arch/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/zephyr/lib/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/zephyr/soc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/zephyr/boards/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/zephyr/subsys/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/zephyr/drivers/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/modules/nrf/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/modules/mcuboot/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/modules/mbedtls/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/modules/trusted-firmware-m/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/modules/cjson/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/modules/azure-sdk-for-c/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/modules/cirrus-logic/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/modules/openthread/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/modules/suit-processor/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/modules/memfault-firmware-sdk/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/modules/canopennode/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/modules/chre/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/modules/lz4/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/modules/nanopb/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/modules/zscilib/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/modules/cmsis/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/modules/cmsis-dsp/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/modules/cmsis-nn/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/modules/fatfs/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/modules/hal_nordic/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/modules/hal_st/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/modules/hal_wurthelektronik/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/modules/hostap/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/modules/libmetal/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/modules/liblc3/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/modules/littlefs/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/modules/loramac-node/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/modules/lvgl/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/modules/mipi-sys-t/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/modules/nrf_wifi/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/modules/open-amp/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/modules/picolibc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/modules/segger/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/modules/tinycrypt/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/modules/uoscore-uedhoc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/modules/zcbor/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/modules/nrfxlib/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/modules/nrf_hw_models/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/modules/connectedhomeip/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/zephyr/kernel/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/zephyr/cmake/flash/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/zephyr/cmake/usage/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/david/ZephyrProjects/bt-fund/v2.9.0-v2.7.0/l3/Peripheral/build/Peripheral/zephyr/cmake/reports/cmake_install.cmake")
endif()

