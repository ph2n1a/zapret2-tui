cmake_minimum_required(VERSION 3.16)

project(zapret-tui C)


set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# -------------------------
# Build type
# -------------------------
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# -------------------------
# Warnings
# -------------------------
add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
)

# -------------------------
# Include directories
# -------------------------
include_directories(
    include
    src/lib/inih-r62
)

# -------------------------
# Sources
# -------------------------
set(SOURCES
    src/main.c
    src/utils/readconf.c
    src/utils/writeconf.c
    src/utils/check_dependencies.c
    src/core/first_start.c
    src/core/read_zapret_conf.c
    src/utils/utils.c
    src/core/create_zapret_configs.c
)

add_executable(zapret-tui ${SOURCES})

target_link_libraries(zapret-tui confuse)
