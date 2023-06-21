cmake_minimum_required(VERSION 3.0.0)
project(Kiku
    DESCRIPTION "LISTEN.moe radio application"
    VERSION 0.1.0
    LANGUAGES C
)

include(CTest)
enable_testing()

add_executable(kiku main.c)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSOUP libsoup-3.0 REQUIRED)
pkg_check_modules(JSONLIB json-glib-1.0 REQUIRED)

target_include_directories(kiku PRIVATE
    ${LIBSOUP_INCLUDE_DIRS}
    ${JSONLIB_INCLUDE_DIRS}
)

target_link_libraries(kiku
    ${LIBSOUP_LIBRARIES}
    ${JSONLIB_LIBRARIES}
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)