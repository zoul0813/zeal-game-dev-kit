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
