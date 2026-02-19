
# Find Flex and Bison for lexer/parser generation
FIND_PACKAGE(FLEX REQUIRED)
FIND_PACKAGE(BISON REQUIRED)

IF(FLEX_FOUND)
	FLEX_TARGET(Scanner lexer.ll ${CMAKE_CURRENT_SOURCE_DIR}/lexer.yy.cpp COMPILE_FLAGS "-f")
	BISON_TARGET(Parser parser.y ${CMAKE_CURRENT_SOURCE_DIR}/parser.tab.cpp)
	ADD_FLEX_BISON_DEPENDENCY(Scanner Parser)
ENDIF(FLEX_FOUND)

# Collect all source files
AUX_SOURCE_DIRECTORY("${CMAKE_CURRENT_SOURCE_DIR}" SOURCES)
AUX_SOURCE_DIRECTORY("${CMAKE_CURRENT_SOURCE_DIR}/backend" BACKEND_SOURCES)

# Explicitly add new source files not caught by AUX_SOURCE_DIRECTORY cache
list(APPEND SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/typecheck.cpp)

# Add generated sources
list(APPEND SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/lexer.yy.cpp)
list(APPEND SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/parser.tab.cpp)
list(APPEND SOURCES ${BACKEND_SOURCES})

add_library(pie_compiler STATIC ${SOURCES})
