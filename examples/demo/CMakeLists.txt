cmake_minimum_required(VERSION 3.16)

##
# Project specific
##
set(ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets" CACHE PATH "Directory containing assets")
set(target_name demo)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

set(ZOS_TOOLCHAIN sdcc)
include($ENV{ZOS_PATH}/cmake/zos_init.cmake)

zos_use(zvb)
include($ENV{ZGDK_PATH}/cmake/zgdk_init.cmake)

project(zgdk_app_template C ASM)

# Define your output name and sources directly here.
add_executable(${target_name}
    src/main.c
    src/player.c
    src/assets.c
)

##
# Feature toggles (template.mk equivalents)
##
# Add compile definitions as needed, for example:
# target_compile_definitions(${target_name} PRIVATE EMULATOR=1)
# target_compile_definitions(${target_name} PRIVATE FRAMELOCK=1)
# target_compile_definitions(${target_name} PRIVATE DEBUG=1)

##
# Dependencies
##
find_package(ZVB_SDK REQUIRED CONFIG PATHS "$ENV{ZVB_SDK_PATH}/cmake" NO_DEFAULT_PATH)
find_package(ZGDK REQUIRED CONFIG PATHS "$ENV{ZGDK_PATH}/cmake" NO_DEFAULT_PATH)

target_link_libraries(${target_name} PUBLIC zgdk zvb_gfx)
# Add optional libraries as needed, for example:
# target_link_libraries(${target_name} PUBLIC zvb_sound)
# target_link_libraries(${target_name} PUBLIC zvb_crc)

# GIF asset helpers (`gif2zeal`)
# Put `.gif` files in `ASSETS_DIR`.
file(GLOB GIF_SRCS CONFIGURE_DEPENDS "${ASSETS_DIR}/*.gif")
if(GIF_SRCS)
    set(GIF2ZEAL_ARGS FILES ${GIF_SRCS})
    list(APPEND GIF2ZEAL_ARGS COMPRESSED)
    # list(APPEND GIF2ZEAL_ARGS UNIQUE)
    # list(APPEND GIF2ZEAL_ARGS TILEMAP)
    # list(APPEND GIF2ZEAL_ARGS VERBOSE)
    # list(APPEND GIF2ZEAL_ARGS DEBUG)
    # list(APPEND GIF2ZEAL_ARGS BIT 4)      # 1, 4 or 8
    # list(APPEND GIF2ZEAL_ARGS COLORS 16)
    # list(APPEND GIF2ZEAL_ARGS STRIP 8)
    gif2zeal(${target_name} ${GIF2ZEAL_ARGS})
endif()

# TMX asset helpers (`tiled2zeal`)
# Put `.tmx` files in `ASSETS_DIR`.
file(GLOB TMX_SRCS CONFIGURE_DEPENDS "${ASSETS_DIR}/*.tmx")
if(TMX_SRCS)
    set(TILED2ZEAL_ARGS FILES ${TMX_SRCS})
    # list(APPEND TILED2ZEAL_ARGS COMPRESSED)
    # list(APPEND TILED2ZEAL_ARGS VERBOSE)
    # list(APPEND TILED2ZEAL_ARGS DEBUG)
    # list(APPEND TILED2ZEAL_ARGS LAYER 1)
    # list(APPEND TILED2ZEAL_ARGS SIZE 16x16)
    tiled2zeal(${target_name} ${TILED2ZEAL_ARGS})
endif()

# Embed static + generated assets.
file(GLOB STATIC_ZOS_ASSETS CONFIGURE_DEPENDS
    "${ASSETS_DIR}/*.zts"
    "${ASSETS_DIR}/*.ztp"
    "${ASSETS_DIR}/*.ztm"
)
set(EMBED_SRCS ${STATIC_ZOS_ASSETS} ${GIF_SRCS} ${TMX_SRCS})
embed_assets(${target_name} FILES ${EMBED_SRCS})

# Generate <bin>.bin output in the build directory.
zos_add_outputs(${target_name})
