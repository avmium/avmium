cmake_minimum_required(VERSION 3.12)
project(common)

set(COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}  PARENT_SCOPE)

set(
	SRCS
		${CMAKE_CURRENT_SOURCE_DIR}/common.c
		${CMAKE_CURRENT_SOURCE_DIR}/blk_mgmt.c
		${CMAKE_CURRENT_SOURCE_DIR}/mem_ctrl.c
		${CMAKE_CURRENT_SOURCE_DIR}/port_mgmt.c
		${CMAKE_CURRENT_SOURCE_DIR}/que_mgmt.c
		${CMAKE_CURRENT_SOURCE_DIR}/route.c
)

add_library(
	common STATIC
		${SRCS}
)

target_include_directories(
	common PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(common PRIVATE
	SVSIM=${SVSIM}
)

target_link_libraries(common PUBLIC
  	libverilator
)

function(verilate_each target)
  	# Remaining arguments are the .sv files
  	foreach(sv_file IN LISTS ARGN)
    	get_filename_component(name ${sv_file} NAME_WE)

		message(STATUS ${sv_file})
		verilate(${target}
			SOURCES ${sv_file}
			TOP_MODULE unknown
			# TOP_MODULE ${name}
			TRACE_VCD
			VERILATOR_ARGS -Wno-lint --timing
    	)
  	endforeach()
endfunction()
