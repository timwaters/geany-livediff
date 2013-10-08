cmake_minimum_required(VERSION 2.8)
project(geany-livediff)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O4 -Wfatal-errors")

file(GLOB SOURCES *.cpp)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GEANY REQUIRED geany)
pkg_check_modules(LIBGIT REQUIRED libgit2)
find_package(Boost COMPONENTS filesystem system REQUIRED)

add_definitions(${GEANY_CFLAGS} ${LIBGIT_CFLAGS})
add_library(livediff SHARED ${SOURCES})
include_directories(${GEANY_INCLUDE_DIRS} ${LIBGIT_INCLUDE_DIRS})
target_link_libraries(livediff ${GEANY_LIBRARIES} ${LIBGIT_LIBRARIES} ${Boost_LIBRARIES})

set(ICON_PATH share/geany-plugins/livediff)
install(TARGETS livediff DESTINATION lib/geany)
file(GLOB ICON_FILES ${PROJECT_SOURCE_DIR}/img/*.png)
install(FILES ${ICON_FILES} DESTINATION ${ICON_PATH})

configure_file(${CMAKE_SOURCE_DIR}/config.h.cmake ${CMAKE_SOURCE_DIR}/config.h)
