cmake_minimum_required(VERSION 3.16)

set(ZOS_TOOLCHAIN sdcc)
include($ENV{ZOS_PATH}/cmake/zos_init.cmake)

zos_use(coreutils)
project(zgdk C ASM)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

find_package(COREUTILS REQUIRED)

file(GLOB_RECURSE ZGDK_SRCS CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.c
    ${CMAKE_SOURCE_DIR}/src/*.asm
)

add_library(zgdk STATIC ${ZGDK_SRCS})

target_link_libraries(zgdk PUBLIC core)

target_compile_options(zgdk PRIVATE
    $<$<COMPILE_LANGUAGE:C>:--opt-code-size>
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

# Shared ASM source body, compiled only into requested preset capabilities.
target_include_directories(zgdk PRIVATE ${CMAKE_SOURCE_DIR}/src/entities/presets)
set_property(SOURCE src/entities/presets/physics_sync.asm
    src/entities/presets/physics_pool_sync.asm src/entities/presets/sprite_sync.asm
    APPEND PROPERTY OBJECT_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/entities/presets/prepare.inc;${CMAKE_SOURCE_DIR}/src/entities/presets/draw.inc")
