cmake_minimum_required(VERSION 3.16)

set(ZOS_TOOLCHAIN sdcc)
include($ENV{ZOS_PATH}/cmake/zos_init.cmake)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

project(zgdk C)

file(GLOB_RECURSE ZGDK_SRCS CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.c
)

add_library(zgdk STATIC ${ZGDK_SRCS})

target_compile_options(zgdk PRIVATE
    --opt-code-speed
)

target_include_directories(zgdk
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        $ENV{ZVB_SDK_PATH}/include
)

set_target_properties(zgdk PROPERTIES
    OUTPUT_NAME zgdk
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib
)
