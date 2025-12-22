cmake_minimum_required(VERSION 3.12)

set(TARGET apbsc)

set(
	INCS
	${CMAKE_CURRENT_SOURCE_DIR}
)

set(
    SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/apb.c
)

add_library(
	${TARGET} STATIC
        ${SRCS}
)

target_include_directories(
    ${TARGET} PUBLIC
		${INCS}
)

target_link_libraries(
    ${TARGET} PUBLIC
		common
)

target_compile_definitions(${TARGET} PRIVATE
	SVSIM=${SVSIM}
)
