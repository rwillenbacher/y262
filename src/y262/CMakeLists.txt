cmake_minimum_required(VERSION 3.1)
project(liby262)

find_package(Threads)
find_program(YASM_EXE NAMES yasm)

message( "architecture: ${CMAKE_SYSTEM_PROCESSOR}")

set(Y262_TARGET_ARCH "unknown")
if(CMAKE_SYSTEM_PROCESSOR MATCHES "amd64.*|x86_64.*|AMD64.*")
	set(Y262_TARGET_ARCH "intelx86")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i686.*|x86.*|i386.*")
	set(Y262_TARGET_ARCH "intelx86")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm64.*|aarch64.*|ARM64.*|AARCH64.*)")
	set(Y262_TARGET_ARCH "arm64")
endif()

message( "target_arch: ${Y262_TARGET_ARCH}")

if(Y262_TARGET_ARCH MATCHES "intelx86")
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		if(WIN32)
			set(YASM_ARGS -f win32 -m amd64 -DARCH_X86_64 -DPIC)
		elseif(APPLE)
			set(YASM_ARGS -f macho64 -m amd64 -DARCH_X86_64 -DPIC --prefix=_)
		else()
			set(YASM_ARGS -f elf64 -m amd64 -DARCH_X86_64 -DPIC)
		endif()
	else()
		if(WIN32)
			set(YASM_ARGS -f win32 --prefix=_)
		elseif(APPLE)
			set(YASM_ARGS -f macho32 --prefix=_)
		else()
			set(YASM_ARGS -f elf32)
		endif()
	endif()

	add_custom_command(OUTPUT pixelop_x86.o COMMAND ${YASM_EXE}
		ARGS ${YASM_ARGS} -o ${CMAKE_CURRENT_BINARY_DIR}/pixelop_x86.o ${CMAKE_CURRENT_SOURCE_DIR}/pixelop_x86.asm)
	add_custom_command(OUTPUT transform_x86.o COMMAND ${YASM_EXE}
		ARGS ${YASM_ARGS} -o ${CMAKE_CURRENT_BINARY_DIR}/transform_x86.o ${CMAKE_CURRENT_SOURCE_DIR}/transform_x86.asm)
endif()

set(liby262_sources_basic
    ${CMAKE_CURRENT_SOURCE_DIR}/aboveslicelevel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/bitstream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/lookahead.h
    ${CMAKE_CURRENT_SOURCE_DIR}/me.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pixelop.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pixelop_x86.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ratectrl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tables.h
    ${CMAKE_CURRENT_SOURCE_DIR}/threads.h
    ${CMAKE_CURRENT_SOURCE_DIR}/transform.h
    ${CMAKE_CURRENT_SOURCE_DIR}/transform_x86.h
    ${CMAKE_CURRENT_SOURCE_DIR}/types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/y262.h
    ${CMAKE_CURRENT_SOURCE_DIR}/y262api.h
    ${CMAKE_CURRENT_SOURCE_DIR}/aboveslicelevel.c
    ${CMAKE_CURRENT_SOURCE_DIR}/bitstream.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lookahead.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/me.c
    ${CMAKE_CURRENT_SOURCE_DIR}/pixelop.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ratectrl.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tables.c
    ${CMAKE_CURRENT_SOURCE_DIR}/threads.c
    ${CMAKE_CURRENT_SOURCE_DIR}/transform.c
    ${CMAKE_CURRENT_SOURCE_DIR}/y262.c
    ${CMAKE_CURRENT_SOURCE_DIR}/y262api.c
)

set(liby262_sources_assembly "")

if(Y262_TARGET_ARCH MATCHES "intelx86")
	set(liby262_sources_assembly
		${CMAKE_CURRENT_BINARY_DIR}/pixelop_x86.o
		${CMAKE_CURRENT_BINARY_DIR}/transform_x86.o
	)
elseif(Y262_TARGET_ARCH MATCHES "arm64")
	set(liby262_sources_assembly
		${CMAKE_CURRENT_SOURCE_DIR}/transform_arm64.c
		${CMAKE_CURRENT_SOURCE_DIR}/transform_arm64.h
		${CMAKE_CURRENT_SOURCE_DIR}/pixelop_arm64.c
		${CMAKE_CURRENT_SOURCE_DIR}/pixelop_arm64.h
	)
endif()

add_library(liby262 STATIC
	${liby262_sources_basic}
	${liby262_sources_assembly}
)

target_include_directories(liby262 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(Y262_TARGET_ARCH MATCHES "intelx86")
	add_compile_definitions(ASSEMBLY_X86)
elseif(Y262_TARGET_ARCH MATCHES "arm64")
	add_compile_definitions(ASSEMBLY_ARM64)
endif()

if(WIN32)
	target_compile_definitions(liby262 PRIVATE WIN32)
elseif(APPLE)
	target_compile_definitions(liby262 PRIVATE HAVE_LIBPTHREAD)
else()
	target_compile_definitions(liby262 PRIVATE HAVE_LIBPTHREAD)
	target_link_libraries(liby262 PUBLIC m)
endif()

set_target_properties(liby262 PROPERTIES 
	OUTPUT_NAME "liby262$<$<CONFIG:Debug>:d>${MY_ARCH}"
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
	LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
	POSITION_INDEPENDENT_CODE ON
)


target_link_libraries(liby262 PUBLIC Threads::Threads)
