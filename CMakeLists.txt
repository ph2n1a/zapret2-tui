cmake_minimum_required(VERSION 3.16)
project(zapret2-tui C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_compile_options(-Wall -Wextra -Wpedantic)

include(FetchContent)
include(ExternalProject)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

FetchContent_Declare(
    confuse
    GIT_REPOSITORY https://github.com/martinh/libconfuse.git
    GIT_TAG        v3.3
)
FetchContent_MakeAvailable(confuse)

set(NCURSES_INSTALL "${CMAKE_BINARY_DIR}/_deps/ncurses_install")

file(MAKE_DIRECTORY ${NCURSES_INSTALL}/include ${NCURSES_INSTALL}/lib)
file(TOUCH ${NCURSES_INSTALL}/lib/libncurses.a)

ExternalProject_Add(
    ncurses_ext
    URL      https://ftp.gnu.org/pub/gnu/ncurses/ncurses-6.4.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    
    INSTALL_DIR ${NCURSES_INSTALL}
    
    CONFIGURE_COMMAND <SOURCE_DIR>/configure
        --prefix=${NCURSES_INSTALL}
        --without-cxx --without-cxx-binding --without-ada
        --without-manpages --without-tests
        --with-shared=no
        --with-normal=yes
        --enable-widec=no
        --disable-stripping
        CC=${CMAKE_C_COMPILER}
        CFLAGS=${CMAKE_C_FLAGS}
    
    BUILD_COMMAND   ${CMAKE_MAKE_PROGRAM}
    INSTALL_COMMAND ${CMAKE_MAKE_PROGRAM} install
)

add_library(ncurses STATIC IMPORTED GLOBAL)
set_target_properties(ncurses PROPERTIES
    IMPORTED_LOCATION             "${NCURSES_INSTALL}/lib/libncurses.a"
    INTERFACE_INCLUDE_DIRECTORIES "${NCURSES_INSTALL}/include"
)
add_dependencies(ncurses ncurses_ext)

set(SOURCES
    src/main.c
    src/utils/readconf.c
    src/utils/writeconf.c
    src/utils/log.c
    src/utils/check_dependencies.c
    src/core/first_start.c
    src/core/read_zapret_conf.c
    src/utils/utils.c
    src/core/create_zapret_configs.c
    src/core/create_link.c
    src/ui/app.c
    src/ui/core.c
    src/ui/input.c
    src/ui/ui.c
    src/core/testing_profiles.c
)

add_executable(zapret2-tui ${SOURCES})

target_include_directories(zapret2-tui PRIVATE
    include
    src/lib/inih-r62
)

target_link_libraries(zapret2-tui PRIVATE confuse ncurses Threads::Threads OpenSSL::SSL OpenSSL::Crypto)
