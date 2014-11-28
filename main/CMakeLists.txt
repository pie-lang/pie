AUX_SOURCE_DIRECTORY("${CMAKE_CURRENT_SOURCE_DIR}" SOURCES)

add_executable(pie ${SOURCES})
target_link_libraries(pie ${PIE_LINK_LIBRARIES})
