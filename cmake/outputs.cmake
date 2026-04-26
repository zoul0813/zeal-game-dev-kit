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
