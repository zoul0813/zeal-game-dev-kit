# _zgdk_runtime_out_dir(out_var target_name)
# Internal helper that resolves the runtime output directory.
# Resolution order:
# 1) target property RUNTIME_OUTPUT_DIRECTORY
# 2) CMAKE_RUNTIME_OUTPUT_DIRECTORY
# 3) CMAKE_BINARY_DIR
# Arguments:
# - out_var: variable name to set in parent scope.
# - target_name: target used to query per-target runtime output.
# Example (internal):
# _zgdk_runtime_out_dir(runtime_out_dir my_game)
function(_zgdk_runtime_out_dir out_var target_name)
    get_target_property(target_runtime_out_dir ${target_name} RUNTIME_OUTPUT_DIRECTORY)
    if(target_runtime_out_dir)
        set(runtime_out_dir ${target_runtime_out_dir})
    elseif(DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY AND NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY STREQUAL "")
        set(runtime_out_dir ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    else()
        set(runtime_out_dir ${CMAKE_BINARY_DIR})
    endif()

    set(${out_var} "${runtime_out_dir}" PARENT_SCOPE)
endfunction()

# _zgdk_runtime_assets_out_dir(out_var target_name)
# Internal helper that resolves the runtime assets directory:
# <runtime output dir>/assets
# Arguments:
# - out_var: variable name to set in parent scope.
# - target_name: target used to resolve runtime output location.
# Example (internal):
# _zgdk_runtime_assets_out_dir(assets_out my_game)
function(_zgdk_runtime_assets_out_dir out_var target_name)
    _zgdk_runtime_out_dir(runtime_out_dir ${target_name})
    set(${out_var} "${runtime_out_dir}/assets" PARENT_SCOPE)
endfunction()

# _zgdk_asset_to_source_relative(out_var asset_path)
# Internal helper that converts absolute asset paths into CMAKE_SOURCE_DIR-
# relative paths and validates they are inside CMAKE_SOURCE_DIR.
# Arguments:
# - out_var: variable name to set in parent scope.
# - asset_path: absolute or relative asset path.
# Example (internal):
# _zgdk_asset_to_source_relative(asset_rel ${CMAKE_SOURCE_DIR}/build/assets/a.zts)
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
