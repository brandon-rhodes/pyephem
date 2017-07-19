cmake_minimum_required(VERSION 3.8)
project(ephem)

set(ASTRO_VERSION 3.7.7)
set(ASTRO_VERSION_MAJOR 3)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
set(BUILD_SHARED_LIBS TRUE)

if (UNIX)
   set(CMAKE_C_FLAGS_RELEASE "-Wall -O3")
   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
elseif(WIN32)
   set (CMAKE_C_FLAGS_RELEASE "/O2 /D NDEBUG")
endif()

file(GLOB SOURCE_FILES
     libastro-${ASTRO_VERSION}/*.c
     extensions/dtoa.c extensions/data/*.c)

add_library(astro SHARED ${SOURCE_FILES})
set_target_properties(astro PROPERTIES
                      VERSION ${ASTRO_VERSION}
                      SOVERSION ${ASTRO_VERSION_MAJOR})
target_include_directories(astro PRIVATE
                           ${CMAKE_SOURCE_DIR}/libastro-${ASTRO_VERSION}
                           ${CMAKE_BINARY_DIR})

if (UNIX)
	target_link_libraries(astro PRIVATE m)
endif()

install(TARGETS astro
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

include(GenerateExportHeader)
generate_export_header(astro)
