
include_directories(../Common ../SyncLog)

aux_source_directory(. MAINSRC)
add_executable(SyncLog ${MAINSRC})
target_link_libraries(SyncLog common synclog)


