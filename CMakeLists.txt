
cmake_minimum_required(VERSION 2.6)

set(CMAKE_CXX_FLAGS "-O0 -Wall -Werror -g")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH  ${PROJECT_BINARY_DIR}/lib)

include_directories(include)

IF (APPLE)
    link_directories(/usr/local/lib ${PROJECT_SOURCE_DIR}/lib/mac)
ELSEIF (UNIX)
    link_directories(/usr/local/lib ${PROJECT_SOURCE_DIR}/lib/linux)
ENDIF()

add_subdirectory(src)
