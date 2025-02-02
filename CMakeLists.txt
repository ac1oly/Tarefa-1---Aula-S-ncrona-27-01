# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================



cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PICO_BOARD pico_w CACHE STRING "Board type")
include(pico_sdk_import.cmake)
project(tarefa02 C CXX ASM)
pico_sdk_init()
add_executable(tarefa02 tarefa02.c )
file(MAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/generated)
pico_generate_pio_header(tarefa02 ${CMAKE_CURRENT_LIST_DIR}/tarefa02.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)
target_sources(tarefa02 PRIVATE tarefa02.c)
target_link_libraries(tarefa02 PRIVATE pico_stdlib hardware_pio)
pico_add_extra_outputs(tarefa02) 



