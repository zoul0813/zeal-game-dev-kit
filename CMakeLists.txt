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

# Cameras and tilemap loading/streaming are used by RAM-constrained games. Keep their objects small;
# each public camera function has its own archive member.
set_source_files_properties(
    src/camera/init_view.c src/camera/set.c src/camera/follow_target.c
    src/camera/scroll_layer.c src/camera/rebase.c
    src/tilemap/load.c
    src/tilemap/stream.c src/tilemap/stream_ring.c
    src/tilemap/stream_ram.c src/tilemap/stream_reader.c
    src/tilemap/stream_edit.c src/tilemap/stream_buffer.c src/tilemap/stream_rebase.c
    PROPERTIES COMPILE_OPTIONS "--opt-code-size"
)

# Preserve the compact sequencer implementation used by RAM-constrained games.
set_source_files_properties(src/sound/sfx.c src/sound/hardware.c
    PROPERTIES COMPILE_OPTIONS "--opt-code-size")

# Keep physics and general tile-flag lookups compact, with separate archive members.
file(GLOB_RECURSE ZGDK_PHYSICS_SRCS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/physics/*.c")
file(GLOB ZGDK_TILEFLAGS_SRCS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/tilemap/flags/*.c")
set_source_files_properties(${ZGDK_PHYSICS_SRCS} ${ZGDK_TILEFLAGS_SRCS}
    PROPERTIES COMPILE_OPTIONS "--opt-code-size")

# Independent pool capabilities are extracted only when used.
file(GLOB_RECURSE ZGDK_ENTITY_SRCS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/entities/*.c")
set_source_files_properties(${ZGDK_ENTITY_SRCS} PROPERTIES COMPILE_OPTIONS "--opt-code-size")

# Preserve the compact game animation sequencer, one capability per member.
set_source_files_properties(src/anims/play.c src/anims/restart.c src/anims/tick.c
    PROPERTIES COMPILE_OPTIONS "--opt-code-size")

# Shared ASM source body, compiled only into requested preset capabilities.
target_include_directories(zgdk PRIVATE ${CMAKE_SOURCE_DIR}/src/entities/presets)
set_property(SOURCE src/entities/presets/physics_sync.asm
    src/entities/presets/physics_pool_sync.asm src/entities/presets/sprite_sync.asm
    APPEND PROPERTY OBJECT_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/entities/presets/prepare.inc;${CMAKE_SOURCE_DIR}/src/entities/presets/draw.inc")
