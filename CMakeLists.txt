cmake_minimum_required(VERSION 3.20)
project(container
    VERSION 0.1.0
    DESCRIPTION "Linux container runtime in C"
    LANGUAGES C
)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -O0 -fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

add_library(container STATIC
    src/container.c
    src/namespaces.c
    src/mount.c
    src/cgroups.c
    src/cli.c
)
target_compile_definitions(container PRIVATE CONTAINER_LIB)
target_include_directories(container
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/log>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(container PUBLIC c_std_23)

add_executable(container_bin
    src/container.c
)
set_target_properties(container_bin PROPERTIES OUTPUT_NAME container)
set_target_properties(container_bin PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

target_link_libraries(container_bin PRIVATE container log)
target_include_directories(container_bin PRIVATE include lib/log)

enable_testing()

add_executable(test_container tests/test_container.c)
target_link_libraries(test_container PRIVATE container log)
target_include_directories(test_container PRIVATE include)

add_test(NAME test_container COMMAND test_container)
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS test_container
)

add_library(log STATIC lib/log/log.c)
target_include_directories(log PUBLIC lib/log)

# Drop other vendored sources into lib/<name>/ and add:
#   add_subdirectory(lib/<name>)
#   target_link_libraries(container PRIVATE <name>)
# For system libs: find_package(...)

include(GNUInstallDirs)
install(TARGETS container container_bin
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

find_program(CLANG_FORMAT clang-format)
if(CLANG_FORMAT)
    add_custom_target(format
        COMMAND ${CLANG_FORMAT} -i
            ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
            ${CMAKE_CURRENT_SOURCE_DIR}/include/container/*.h
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.c
        COMMENT "Formatting with clang-format"
    )
endif()

find_program(CLANG_TIDY clang-tidy)
if(CLANG_TIDY)
    set(CMAKE_C_CLANG_TIDY ${CLANG_TIDY})
endif()
