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
