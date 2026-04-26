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
