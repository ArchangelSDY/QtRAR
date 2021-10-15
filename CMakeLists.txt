project (QtRAR)
cmake_minimum_required(VERSION 2.8.11)

find_package(Threads)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
find_package(Qt6 COMPONENTS Core Test Gui REQUIRED)
if (NOT Qt6_FOUND)
    find_package(Qt5 5.15 COMPONENTS Core Test Gui REQUIRED)
endif()

# All build libraries are moved to this directory
SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)")
set(LIB_DESTINATION "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}" CACHE STRING "Library directory name" FORCE)

add_subdirectory(src)

if (NOT DEFINED QTRAR_NO_TEST)
    enable_testing()
    add_subdirectory(test)
endif()
