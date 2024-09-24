cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
include(../utils.cmake)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
project(rsl_test VERSION "1.0.0" DESCRIPTION "")

add_executable(rsl_test "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../ ${CMAKE_BINARY_DIR}/rsl)

# target_link_libraries(${PROJECT_NAME} rsl)
LinkToExecutable(${PROJECT_NAME} rsl)
CopyRuntimeDlls(${PROJECT_NAME} ${PROJECT_NAME})

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE "/MP")
endif()


