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
    $<$<COMPILE_LANGUAGE:C>:--opt-code-speed>
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

# Cameras/streaming are used by RAM-constrained games. Keep their objects small;
# each public camera function has its own archive member.
set_source_files_properties(
    src/camera/init_view.c src/camera/set.c src/camera/follow_target.c
    src/camera/scroll_layer.c src/camera/rebase.c
    src/tilemap/stream.c src/tilemap/stream_ring.c
    src/tilemap/stream_ram.c src/tilemap/stream_reader.c
    src/tilemap/stream_edit.c src/tilemap/stream_buffer.c src/tilemap/stream_rebase.c
    PROPERTIES COMPILE_OPTIONS "--opt-code-size"
)

# Preserve the compact sequencer implementation used by RAM-constrained games.
set_source_files_properties(src/sound/sfx.c src/sound/hardware.c
    PROPERTIES COMPILE_OPTIONS "--opt-code-size")
