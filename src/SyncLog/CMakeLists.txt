
include_directories(../Common)
aux_source_directory(. LIBSRC)
add_library(synclog STATIC ${LIBSRC})
target_link_libraries(synclog common)
