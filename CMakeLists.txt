cmake_minimum_required(VERSION 3.20)

if (NOT CMAKE_TOOLCHAIN_FILE)
    set(_phys_vcpkg_roots
        "$ENV{VCPKG_ROOT}"
        "D:/app/vcpkg"
        "C:/vcpkg"
        "C:/dev/vcpkg"
        "$ENV{USERPROFILE}/vcpkg"
    )
    set(_phys_vcpkg_toolchain "")
    foreach (_root IN LISTS _phys_vcpkg_roots)
        if (_root AND EXISTS "${_root}/scripts/buildsystems/vcpkg.cmake")
            if (EXISTS "${_root}/installed/x64-windows/share/raylib")
                set(_phys_vcpkg_toolchain "${_root}/scripts/buildsystems/vcpkg.cmake")
                break()
            elseif (NOT _phys_vcpkg_toolchain)
                set(_phys_vcpkg_toolchain "${_root}/scripts/buildsystems/vcpkg.cmake")
            endif()
        endif()
    endforeach()
    if (_phys_vcpkg_toolchain)
        set(CMAKE_TOOLCHAIN_FILE "${_phys_vcpkg_toolchain}" CACHE FILEPATH "vcpkg toolchain file")
        message(STATUS "phys: vcpkg toolchain = ${_phys_vcpkg_toolchain}")
    endif()
endif()

project(phys_sap LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    add_compile_options(/W4 /permissive- /utf-8)
else()
    add_compile_options(-Wall -Wextra)
endif()

option(PHYS_BUILD_APP "Build the raylib visualization app" ON)

file(GLOB_RECURSE PHYS_CORE_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sim/*.cpp
)
add_library(phys_core STATIC ${PHYS_CORE_SOURCES})
target_include_directories(phys_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(phys_tests tests/test_sap.cpp)
target_link_libraries(phys_tests PRIVATE phys_core)

add_executable(phys_bench bench/bench_sap.cpp)
target_link_libraries(phys_bench PRIVATE phys_core)

if (PHYS_BUILD_APP)
    find_package(raylib QUIET)
    if (raylib_FOUND)
        add_executable(phys_app src/app/main.cpp)
        target_link_libraries(phys_app PRIVATE phys_core raylib)
    else()
        message(WARNING
            "raylib not found: the 'phys_app' target will NOT be created.\n"
            "  Configure with the vcpkg toolchain, e.g.:\n"
            "    cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake\n"
            "  or select the 'vcpkg-debug' preset in your IDE.\n"
            "  To build only core/tests/bench, pass -DPHYS_BUILD_APP=OFF.")
    endif()
endif()

enable_testing()
add_test(NAME sap_equivalence COMMAND phys_tests)
