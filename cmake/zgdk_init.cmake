if (NOT DEFINED ZGDK_DIR)
    set(ZGDK_DIR "${CMAKE_CURRENT_LIST_DIR}")
endif()

include("${ZGDK_DIR}/helpers.cmake")
include("${ZGDK_DIR}/asset_copy.cmake")
include("${ZGDK_DIR}/asset_embed.cmake")
include("${ZGDK_DIR}/outputs.cmake")
