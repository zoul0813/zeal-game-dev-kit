set(_ZGDK_TOOLS_DIR "${CMAKE_CURRENT_LIST_DIR}/../tools")

# tileset_chunks(target GIF <gif>
#                [FORMAT lz|rle|none|4bit|2bit|1bit] [MAX <bytes>]
#                [TILESET <tch>] [PALETTE <ztp>] [BUILD_DIR <dir>]
#                [UNIQUE] [COLORS <n>] [STRIP <n>])
# Converts a GIF with the Video Board SDK's gif2zeal.py and splits the tileset into a chunked .tch
# (tools/zts_chunks.py), to stream with tileset_load_chunks() (zgdk/tileset.h) through a MAX byte buffer.
# The .tch and .ztp are build outputs: pack them into an archive, copy them next to the program, or embed them.
# Arguments:
# - target: target that depends on the generated files.
# - GIF: source GIF.
# - FORMAT: tile data, and the loader to pass to tileset_load_chunks(): lz (default, gfx_tileset_load_lz),
#   rle (gfx_tileset_load_rle), none (gfx_tileset_load_none), 4bit/2bit/1bit (gfx_tileset_load_4bit, ...).
#   lz and rle need an 8-bit video mode.
# - MAX: largest chunk in bytes, the game's load buffer (default 2048).
# - TILESET / PALETTE: output paths (default BUILD_DIR/<gif name>.tch and .ztp).
# - BUILD_DIR: directory for generated files (default ${CMAKE_BINARY_DIR}/assets).
# - UNIQUE, COLORS, STRIP: passed to gif2zeal (-u, -c, -s).
# Example:
# tileset_chunks(my_game GIF assets/tiles.gif FORMAT lz MAX 2048)
function(tileset_chunks target)
    cmake_parse_arguments(TCH "UNIQUE" "GIF;FORMAT;MAX;TILESET;PALETTE;BUILD_DIR;COLORS;STRIP" "" ${ARGN})

    if(NOT TCH_GIF)
        message(FATAL_ERROR "tileset_chunks: GIF is required")
    endif()
    if(NOT TCH_FORMAT)
        set(TCH_FORMAT lz)
    endif()
    if(NOT TCH_MAX)
        set(TCH_MAX 2048)
    endif()
    if(NOT TCH_BUILD_DIR)
        set(TCH_BUILD_DIR "${CMAKE_BINARY_DIR}/assets")
    endif()

    if(TCH_FORMAT STREQUAL "lz")
        set(gif_args -z lz)
    elseif(TCH_FORMAT STREQUAL "rle")
        set(gif_args -z rle)
    elseif(TCH_FORMAT STREQUAL "none")
        set(gif_args "")
    elseif(TCH_FORMAT STREQUAL "4bit")
        set(gif_args -b 4)
    elseif(TCH_FORMAT STREQUAL "2bit")
        set(gif_args -b 2)
    elseif(TCH_FORMAT STREQUAL "1bit")
        set(gif_args -b 1)
    else()
        message(FATAL_ERROR "tileset_chunks: FORMAT must be lz, rle, none, 4bit, 2bit or 1bit, not '${TCH_FORMAT}'")
    endif()
    if(TCH_UNIQUE)
        list(APPEND gif_args -u)
    endif()
    if(TCH_COLORS)
        list(APPEND gif_args -c ${TCH_COLORS})
    endif()
    if(TCH_STRIP)
        list(APPEND gif_args -s ${TCH_STRIP})
    endif()

    if(NOT Python3_EXECUTABLE)
        find_package(Python3 REQUIRED COMPONENTS Interpreter)
    endif()

    get_filename_component(gif_abs "${TCH_GIF}" ABSOLUTE BASE_DIR "${CMAKE_SOURCE_DIR}")
    get_filename_component(stem "${TCH_GIF}" NAME_WE)
    if(NOT TCH_TILESET)
        set(TCH_TILESET "${TCH_BUILD_DIR}/${stem}.tch")
    endif()
    if(NOT TCH_PALETTE)
        set(TCH_PALETTE "${TCH_BUILD_DIR}/${stem}.ztp")
    endif()
    get_filename_component(tch "${TCH_TILESET}" ABSOLUTE BASE_DIR "${CMAKE_BINARY_DIR}")
    get_filename_component(ztp "${TCH_PALETTE}" ABSOLUTE BASE_DIR "${CMAKE_BINARY_DIR}")
    get_filename_component(tch_dir "${tch}" DIRECTORY)
    get_filename_component(ztp_dir "${ztp}" DIRECTORY)
    # the .zts from gif2zeal, kept next to the .tch it becomes
    set(zts "${tch}.zts")

    set(gif2zeal "$ENV{ZVB_SDK_PATH}/tools/zeal2gif/gif2zeal.py")
    set(zts_chunks "${_ZGDK_TOOLS_DIR}/zts_chunks.py")
    add_custom_command(
        OUTPUT ${tch} ${ztp}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${TCH_BUILD_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${tch_dir} ${ztp_dir}
        COMMAND ${Python3_EXECUTABLE} ${gif2zeal} -i ${gif_abs} -t ${zts} -p ${ztp} -o ${TCH_BUILD_DIR} ${gif_args}
        COMMAND ${Python3_EXECUTABLE} ${zts_chunks} ${zts} ${tch} --format ${TCH_FORMAT} --max ${TCH_MAX}
                --zvb $ENV{ZVB_SDK_PATH}
        DEPENDS ${gif_abs} ${gif2zeal} ${zts_chunks}
        COMMENT "Chunking tileset ${stem}.gif (${TCH_FORMAT}, max ${TCH_MAX})"
        VERBATIM
    )
    file(RELATIVE_PATH tch_rel "${CMAKE_BINARY_DIR}" "${tch}")
    string(MAKE_C_IDENTIFIER "${target}_${tch_rel}" custom_target)
    add_custom_target(${custom_target} ALL DEPENDS ${tch} ${ztp})
    add_dependencies(${target} ${custom_target})
endfunction()
