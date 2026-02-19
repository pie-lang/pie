
add_executable(debugger_native_test debugger_native_test.cpp)
target_link_libraries(debugger_native_test ${PIE_LINK_LIBRARIES})

add_test(NAME debugger_native_test COMMAND debugger_native_test)

add_executable(typecheck_test typecheck_test.cpp)
target_link_libraries(typecheck_test ${PIE_LINK_LIBRARIES})

add_executable(struct_test struct_test.cpp)
target_link_libraries(struct_test ${PIE_LINK_LIBRARIES})

add_test(NAME struct_test COMMAND struct_test)

add_test(NAME typecheck_test COMMAND typecheck_test)
