cmake_minimum_required(VERSION 3.28)
project(cdboost VERSION 0.3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "")
endif()

find_package(Boost REQUIRED COMPONENTS rational)
find_package(spdlog CONFIG REQUIRED)

add_library(cdboost INTERFACE)
target_include_directories(cdboost INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(cdboost INTERFACE Boost::boost spdlog::spdlog)
target_compile_options(cdboost INTERFACE -Wall -Wextra -pedantic)

option(CDBOOST_BUILD_TESTS "Build tests" ON)
option(CDBOOST_BUILD_EXAMPLES "Build examples" ON)

if(CDBOOST_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

if(CDBOOST_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()

find_program(CLANG_FORMAT clang-format)
if(CLANG_FORMAT)
    file(GLOB_RECURSE ALL_SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/example/*.cpp"
    )
    add_custom_target(format
        COMMAND ${CLANG_FORMAT} -i ${ALL_SOURCE_FILES}
        COMMENT "Applying clang-format"
    )
    add_custom_target(check-format
        COMMAND ${CLANG_FORMAT} --dry-run --Werror ${ALL_SOURCE_FILES}
        COMMENT "Checking clang-format"
    )
endif()
