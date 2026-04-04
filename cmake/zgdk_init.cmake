if (NOT DEFINED ZGDK_DIR)
    set(ZGDK_DIR "${CMAKE_CURRENT_LIST_DIR}")
endif()

include("${ZGDK_DIR}/helpers.cmake")

# add_asset(target FILES <file> [...])
# Copies source assets into ${CMAKE_BINARY_DIR}/assets at build time.
# Arguments:
# - target: build target that will depend on the copy step.
# - FILES: list of asset paths (absolute or relative).
# Example:
# add_asset(my_game
#     FILES
#         assets/player.gif
#         assets/tileset.tmx
# )
function(add_asset target)
    set(target_name ${ARGV0})
    # Remove target name for parsing
    list(REMOVE_AT ARGV 0)

    # Parse arguments
    cmake_parse_arguments(
        COPY_ASSETS
        ""
        ""
        FILES
        ${ARGV}
    )

    set(build_out_dir ${CMAKE_BINARY_DIR}/assets)

    foreach(asset_file ${COPY_ASSETS_FILES})
        get_filename_component(fname ${asset_file} NAME)
        get_filename_component(asset_abs ${asset_file} ABSOLUTE)

        set(output_file ${build_out_dir}/${fname})

        # Make a unique target name based on filename
        string(REPLACE "." "_" fname_safe ${fname})
        set(custom_target_name "${target}_${fname_safe}_copy")

        add_custom_command(
            OUTPUT ${output_file}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${build_out_dir}
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${asset_abs} ${output_file}
            DEPENDS ${asset_abs}
            COMMENT "Copying asset ${fname}"
            VERBATIM
        )

        # Create a per-file custom target
        add_custom_target(${custom_target_name} ALL DEPENDS ${output_file})
        add_dependencies(${target_name} ${custom_target_name})
    endforeach()
endfunction()

# zgdk_add_outputs(target [KEEP_BIN] [...])
# Wrapper around zos_add_outputs() that also moves debug artifacts to
# ${CMAKE_SOURCE_DIR}/debug and optionally renames <target>.bin -> <target>.
# Arguments:
# - target: target name passed to zos_add_outputs.
# - KEEP_BIN: optional flag; when present, keeps the .bin extension.
# - ...: any additional arguments are forwarded to zos_add_outputs.
# Example:
# zgdk_add_outputs(my_game)
# zgdk_add_outputs(my_game KEEP_BIN)
function(zgdk_add_outputs target)
    if(NOT COMMAND zos_add_outputs)
        message(FATAL_ERROR "zgdk_add_outputs requires zos_add_outputs (include zos_init.cmake first).")
    endif()

    set(target_name ${ARGV0})
    # Remove target name for parsing.
    list(REMOVE_AT ARGV 0)

    cmake_parse_arguments(
        ZGDK_ADD_OUTPUTS
        "KEEP_BIN"
        ""
        ""
        ${ARGV}
    )

    zos_add_outputs(${target_name} ${ZGDK_ADD_OUTPUTS_UNPARSED_ARGUMENTS})

    _zgdk_runtime_out_dir(runtime_out_dir ${target_name})
    set(debug_out_dir "${CMAKE_SOURCE_DIR}/debug")

    # zos_add_outputs creates the ${target}_bin output target.
    add_custom_command(TARGET ${target_name}_bin POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${debug_out_dir}"
        COMMAND ${CMAKE_COMMAND} -E rename
            "${runtime_out_dir}/${target_name}.cdb"
            "${debug_out_dir}/${target_name}.cdb"
        COMMAND ${CMAKE_COMMAND} -E rename
            "${runtime_out_dir}/${target_name}.map"
            "${debug_out_dir}/${target_name}.map"
        COMMAND ${CMAKE_COMMAND} -E rename
            "${runtime_out_dir}/${target_name}.ihx"
            "${debug_out_dir}/${target_name}.ihx"
        COMMENT "Moving debug files to debug directory"
        VERBATIM
    )

    if(NOT ZGDK_ADD_OUTPUTS_KEEP_BIN)
        add_custom_command(TARGET ${target_name}_bin POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E rename
                "${runtime_out_dir}/${target_name}.bin"
                "${runtime_out_dir}/${target_name}"
            COMMENT "Renaming ${target_name}.bin to ${target_name}"
            VERBATIM
        )
    endif()
endfunction()

# copy_asset(target asset_file)
# Copies one built asset from ${CMAKE_BINARY_DIR}/assets/<name> to
# <runtime output dir>/assets/<name>.
# Arguments:
# - target: build target that will depend on the copy step.
# - asset_file: asset path (only basename is used for lookup/copy name).
# Example:
# copy_asset(my_game assets/player.zts)
function(copy_asset target asset_file)
    set(target_name ${target})
    get_filename_component(fname ${asset_file} NAME)
    set(build_asset_file ${CMAKE_BINARY_DIR}/assets/${fname})

    _zgdk_runtime_assets_out_dir(runtime_assets_out_dir ${target_name})
    set(output_file ${runtime_assets_out_dir}/${fname})

    # Make a unique target name based on filename.
    string(REPLACE "." "_" fname_safe ${fname})
    set(custom_target_name "${target}_${fname_safe}_runtime_copy")

    add_custom_command(
        OUTPUT ${output_file}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${runtime_assets_out_dir}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${build_asset_file} ${output_file}
        DEPENDS ${build_asset_file}
        COMMENT "Copying built asset ${fname} to runtime output"
        VERBATIM
    )

    add_custom_target(${custom_target_name} ALL DEPENDS ${output_file})
    add_dependencies(${target_name} ${custom_target_name})
endfunction()

# copy_assets(target FILES <file> [...])
# Batch wrapper for copy_asset().
# Arguments:
# - target: build target that will depend on all copy steps.
# - FILES: list of files to copy from build assets to runtime assets.
# Notes:
# - Duplicate FILES entries are removed.
# Example:
# copy_assets(my_game
#     FILES
#         build/assets/player.zts
#         build/assets/player.ztp
# )
function(copy_assets target)
    set(target_name ${ARGV0})
    # Remove target name for parsing.
    list(REMOVE_AT ARGV 0)

    cmake_parse_arguments(
        COPY
        ""
        ""
        "FILES"
        ${ARGV}
    )

    if(COPY_FILES)
        list(REMOVE_DUPLICATES COPY_FILES)
        foreach(asset_file ${COPY_FILES})
            copy_asset(${target_name} ${asset_file})
        endforeach()
    endif()
endfunction()

# embed_asset(target asset_file [BUILD_DIR <dir>])
# Adds one asset to zos_add_assets() with extension-aware behavior.
# Supported input extensions:
# - .gif -> emits <stem>.zts and <stem>.ztp from BUILD_DIR
# - .tmx -> emits <stem>.ztm from BUILD_DIR
# - .zts/.ztp/.ztm -> passed through directly
# Arguments:
# - target: target that receives embedded assets.
# - asset_file: source asset path.
# - BUILD_DIR: optional directory used for generated outputs (.gif/.tmx);
#   default is "build/assets".
# Example:
# embed_asset(my_game assets/player.gif BUILD_DIR build/assets)
# embed_asset(my_game build/assets/level1.ztm)
function(embed_asset target asset_file)
    if(NOT COMMAND zos_add_assets)
        message(FATAL_ERROR "embed_asset requires zos_add_assets (include zos_init.cmake first).")
    endif()

    cmake_parse_arguments(
        EMBED_ASSET
        ""
        "BUILD_DIR"
        ""
        ${ARGN}
    )

    if(NOT EMBED_ASSET_BUILD_DIR)
        set(EMBED_ASSET_BUILD_DIR "build/assets")
    endif()

    get_filename_component(asset_ext "${asset_file}" EXT)
    string(TOLOWER "${asset_ext}" asset_ext)
    get_filename_component(asset_stem "${asset_file}" NAME_WE)

    if(asset_ext STREQUAL ".gif")
        zos_add_assets(${target}
            "${EMBED_ASSET_BUILD_DIR}/${asset_stem}.zts"
            "${EMBED_ASSET_BUILD_DIR}/${asset_stem}.ztp"
        )
    elseif(asset_ext STREQUAL ".tmx")
        zos_add_assets(${target}
            "${EMBED_ASSET_BUILD_DIR}/${asset_stem}.ztm"
        )
    elseif(asset_ext STREQUAL ".zts" OR asset_ext STREQUAL ".ztp" OR asset_ext STREQUAL ".ztm")
        _zgdk_asset_to_source_relative(asset_rel "${asset_file}")
        zos_add_assets(${target} "${asset_rel}")
    else()
        message(FATAL_ERROR
            "embed_asset: unsupported asset extension '${asset_ext}' for '${asset_file}'. "
            "Supported: .gif .tmx .zts .ztp .ztm"
        )
    endif()

    target_compile_definitions(${target} PUBLIC GENERATED_ASSETS=1)
endfunction()

# embed_assets(target [BUILD_DIR <dir>] FILES <file> [...])
# Batch wrapper for embed_asset().
# Arguments:
# - target: target that receives embedded assets.
# - BUILD_DIR: optional build directory for generated assets.
# - FILES: list of files to embed.
# Notes:
# - Duplicate FILES entries are removed.
# Example:
# embed_assets(my_game
#     BUILD_DIR build/assets
#     FILES
#         assets/player.gif
#         assets/level1.tmx
#         build/assets/ui.zts
# )
function(embed_assets target)
    if(NOT COMMAND zos_add_assets)
        message(FATAL_ERROR "embed_assets requires zos_add_assets (include zos_init.cmake first).")
    endif()

    cmake_parse_arguments(
        EMBED
        ""
        "BUILD_DIR"
        "FILES"
        ${ARGN}
    )

    if(EMBED_FILES)
        list(REMOVE_DUPLICATES EMBED_FILES)
        foreach(asset_file ${EMBED_FILES})
            if(EMBED_BUILD_DIR)
                embed_asset(${target} "${asset_file}" BUILD_DIR "${EMBED_BUILD_DIR}")
            else()
                embed_asset(${target} "${asset_file}")
            endif()
        endforeach()
    endif()
endfunction()
