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

    if(ZGDK_ADD_OUTPUTS_KEEP_BIN)
        set(strip_bin_extension OFF)
    else()
        set(strip_bin_extension ON)
    endif()

    coreutils_add_post_build_artifacts(
        ${target_name}
        OUT_DIR "${runtime_out_dir}"
        DEBUG_DIR "${CMAKE_SOURCE_DIR}/debug"
        STRIP_BIN_EXTENSION "${strip_bin_extension}"
    )
endfunction()
