# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

# Find Python3 for certificate conversion
find_program(PYTHON3 python3)

# Define certificate paths
set(CERT_PEM_FILE ${CMAKE_CURRENT_SOURCE_DIR}/certs/ca.crt)
set(CERT_HEADER_FILE ${CMAKE_CURRENT_SOURCE_DIR}/src/certificates.h)
set(CERT_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate_cert_header.py)

# Add custom command to generate certificates.h from PEM file
if(PYTHON3)
    add_custom_command(
        OUTPUT ${CERT_HEADER_FILE}
        COMMAND ${PYTHON3} ${CERT_SCRIPT} ${CERT_PEM_FILE} -o ${CERT_HEADER_FILE}
        DEPENDS ${CERT_PEM_FILE} ${CERT_SCRIPT}
        COMMENT "Generating certificate header from PEM"
    )
    
    # Make sure the generated header file is built before compiling the sources
    add_custom_target(generate_cert_header ALL DEPENDS ${CERT_HEADER_FILE})
else()
    message(WARNING "Python3 not found, certificate header will not be automatically generated")
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(mqtt_publisher)

# Include all source directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sensors
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mqtt
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/src/i2c
)

# Gather source files from all subdirectories
file(GLOB app_sources 
    src/main.c
    src/sensors/*.c
    src/config/*.c
    src/utils/*.c
    src/i2c/*.c
    src/mqtt/*.c
)

# FILE(GLOB app_sources src/main_old.c)
target_sources(app PRIVATE ${app_sources})

# Add dependency to ensure certificate header is generated before building
if(PYTHON3)
    add_dependencies(app generate_cert_header)
endif()