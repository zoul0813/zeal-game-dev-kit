# Called by: find_package(ZGDK REQUIRED)

if(NOT TARGET core)
    zos_use(coreutils)
    find_package(COREUTILS REQUIRED)
endif()

if(NOT TARGET zvb_dma)
    zos_use(zvb)
    find_package(ZVB_SDK REQUIRED)
endif()

add_library(zgdk INTERFACE IMPORTED)

set_target_properties(zgdk PROPERTIES
    IMPORTED_LIBNAME "zgdk"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/../include"
    INTERFACE_LINK_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/../lib"
)

target_link_libraries(zgdk INTERFACE core zvb_dma)
