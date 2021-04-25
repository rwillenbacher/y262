cmake_minimum_required(VERSION 3.1)
project(y262app)

set( SRC_FILES 
	${CMAKE_CURRENT_SOURCE_DIR}/main.c
)

add_executable(y262app ${SRC_FILES})
target_link_libraries(y262app liby262)
set_target_properties(y262app PROPERTIES 
	OUTPUT_NAME "y262$<$<CONFIG:Debug>:d>"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
	POSITION_INDEPENDENT_CODE ON
)
