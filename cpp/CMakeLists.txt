cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
project(ashl VERSION "1.0.0" DESCRIPTION "")

file(GLOB_RECURSE SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/lib/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp" )
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES "${SOURCE_FILES}")

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include> 
)


if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE "/MP")
  endif()

  install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include
    DESTINATION include
  )

  install(
    TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
  )

  install(
    EXPORT ${PROJECT_NAME}-targets
    FILE ${PROJECT_NAME}Config.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION lib/cmake/${PROJECT_NAME}
  )

