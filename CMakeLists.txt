cmake_minimum_required(VERSION 3.12)
project(amba_bridges)

add_compile_options(-g)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(VERILATOR REQUIRED)
# Define Verilator::verilator target if missing
if(NOT TARGET Verilator::verilator)
  if(DEFINED VERILATOR_ROOT)
    add_library(libverilator)
	target_include_directories(libverilator PUBLIC
      ${VERILATOR_ROOT}/include
      ${VERILATOR_ROOT}/include/vltstd
    )
	target_sources(libverilator PUBLIC
		${VERILATOR_ROOT}/include/verilated_dpi.cpp
  		${VERILATOR_ROOT}/include/verilated.cpp
		${VERILATOR_ROOT}/include/verilated_threads.cpp
	)
  endif()
endif()

find_package(SystemCLanguage CONFIG REQUIRED)
set (CMAKE_CXX_STANDARD ${SystemC_CXX_STANDARD})

# Find all subdirectories containing a CMakeLists.txt
file(GLOB children RELATIVE ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/*)


if(DEFINED VERILATOR_ROOT)
	set(SVSIM 1)
else()
	set(SVSIM 0)
endif()

foreach(child ${children})
  if(IS_DIRECTORY ${CMAKE_SOURCE_DIR}/${child} AND EXISTS ${CMAKE_SOURCE_DIR}/${child}/CMakeLists.txt)
    add_subdirectory(${child})
  endif()
endforeach()
