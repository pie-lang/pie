
add_executable(debugger_native_test debugger_native_test.cpp)
target_link_libraries(debugger_native_test ${PIE_LINK_LIBRARIES})

add_test(NAME debugger_native_test COMMAND debugger_native_test)
