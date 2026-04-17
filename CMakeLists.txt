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
    src/lib/inih-r62/ini.c
)

add_executable(zapret-tui ${SOURCES})

# -------------------------
# Config path
# -------------------------
target_compile_definitions(zapret-tui PRIVATE
    CONFIG_PATH=\"${CMAKE_SOURCE_DIR}/config\"
)

# -------------------------
# Install rules
# -------------------------
install(TARGETS zapret-tui
    RUNTIME DESTINATION bin
)

install(DIRECTORY config/
    DESTINATION share/zapret-tui/config
)
