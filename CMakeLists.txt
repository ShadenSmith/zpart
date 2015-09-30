project(ZPART)
cmake_minimum_required(VERSION 2.6.0)

find_package(MPI)
include_directories(SYSTEM ${MPI_INCLUDE_PATH})

add_definitions(-D_GNU_SOURCE)

set(C_DEFINES "${C_DEFINES}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${C_DEFINES}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -funroll-loops")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ftree-vectorize")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fgnu89-inline")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstrict-aliasing")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopt-info-vec")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopt-info-vec-missed")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wextra")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-missing-field-initializers")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-parameter")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-variable")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-result")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-function")
# Unfortunately, these warnings pop up in GCC 5.1 despite our MAX_NMODES checks
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-array-bounds")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

file(GLOB ZPART_SOURCES *.c)
add_executable(zpart_bin ${ZPART_SOURCES})

target_link_libraries(zpart_bin m)
target_link_libraries(zpart_bin ${MPI_C_LIBRARIES})
target_link_libraries(zpart_bin zoltan)
install(TARGETS zpart_bin RUNTIME DESTINATION bin)
