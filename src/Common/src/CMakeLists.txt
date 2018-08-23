
include_directories(../include)
aux_source_directory(. COMMON_SRCS)
add_library(common STATIC ${COMMON_SRCS})

IF (APPLE)
    target_link_libraries(common libevent.a pthread)
ELSEIF (UNIX)
    target_link_libraries(common rt libevent.a pthread)
ENDIF()

