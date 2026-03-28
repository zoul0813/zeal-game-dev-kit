if (NOT DEFINED ZGDK_DIR)
    set(ZGDK_DIR "${CMAKE_CURRENT_LIST_DIR}")
endif()

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

function(_zgdk_asset_to_source_relative out_var asset_path)
    if(IS_ABSOLUTE "${asset_path}")
        file(RELATIVE_PATH asset_rel "${CMAKE_SOURCE_DIR}" "${asset_path}")
        if(asset_rel MATCHES "^\\.\\.")
            message(FATAL_ERROR "Asset path must be inside CMAKE_SOURCE_DIR: ${asset_path}")
        endif()
    else()
        set(asset_rel "${asset_path}")
    endif()

    set(${out_var} "${asset_rel}" PARENT_SCOPE)
endfunction()

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
