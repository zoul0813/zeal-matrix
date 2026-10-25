cmake_minimum_required(VERSION 3.16)

if(NOT EXISTS "$ENV{ZOS_PATH}/cmake/zos_init.cmake")
    message(FATAL_ERROR "Set ZOS_PATH to a Zeal OS checkout with the CMake toolchain, or build with zde cmake.")
endif()

set(ZOS_TOOLCHAIN sdcc)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
include("$ENV{ZOS_PATH}/cmake/zos_init.cmake")

project(zeal_matrix LANGUAGES C ASM)

zos_use(zvb)
find_package(ZVB_SDK REQUIRED)

add_executable(matrix src/main.c)
target_link_libraries(matrix PRIVATE zvb)
zos_add_outputs(matrix)
