# Copyright (c) Renetec, Inc. All rights reserved.

cmake_minimum_required(VERSION 2.8.12.1)

project(pyxis-gpio)

set(SOURCES src/pyxis_gpio.c
            src/rpi_gpio_backend.c
            ${BCM2835_SOURCE}/src/bcm2835.c)

add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})

set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY C_STANDARD 11)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${BCM2835_SOURCE}/src)

install(TARGETS ${CMAKE_PROJECT_NAME} RUNTIME DESTINATION /usr/bin)
install(FILES pyxis-gpio.service DESTINATION /lib/systemd/system)
install(FILES src/pyxis_gpio_comm.h DESTINATION /usr/include)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_FLAGS "-Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-g -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O3")

add_subdirectory(example)

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "0")

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Daemon required to control GPIO pins via bcm2835.")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Renetec, Inc.")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://renetec.io")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/postinst")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "armhf")

include(CPack)
