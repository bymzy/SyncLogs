
include_directories(../include)
aux_source_directory(. COMMON_SRCS)
add_library(common STATIC ${COMMON_SRCS})

IF (LINUX)
    target_link_libraries(common rt libevent.a pthread)
ELSEIF (APPLE)
    target_link_libraries(common libevent.a pthread)
ENDIF()

