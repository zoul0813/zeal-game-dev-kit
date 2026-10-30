cmake_minimum_required(VERSION 3.16)
set(ZOS_TOOLCHAIN sdcc)
include($ENV{ZOS_PATH}/cmake/zos_init.cmake)
zos_use(zvb)
include($ENV{ZGDK_PATH}/cmake/zgdk_init.cmake)
project(stream_target C)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
add_executable(sdktest main.c)
target_compile_options(sdktest PRIVATE --opt-code-size)
target_include_directories(sdktest PRIVATE "$ENV{ZVB_SDK_PATH}/include")
find_package(ZGDK REQUIRED CONFIG PATHS "$ENV{ZGDK_PATH}/cmake" NO_DEFAULT_PATH)
target_link_libraries(sdktest PUBLIC zgdk)
set_property(TARGET sdktest APPEND PROPERTY LINK_DEPENDS "$ENV{ZGDK_PATH}/lib/zgdk.lib")
zos_add_outputs(sdktest)

# Prove unused camera compilation units stay out of a real SDCC-linked image.
add_executable(caminit camera_init_only.c)
target_compile_options(caminit PRIVATE --opt-code-size)
target_link_libraries(caminit PUBLIC zgdk)
set_property(TARGET caminit APPEND PROPERTY LINK_DEPENDS "$ENV{ZGDK_PATH}/lib/zgdk.lib")
find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_custom_command(TARGET caminit POST_BUILD
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/check_camera_link.py
        ${CMAKE_CURRENT_BINARY_DIR}/caminit.map
    VERBATIM
)
zos_add_outputs(caminit)
