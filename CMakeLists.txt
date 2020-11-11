cmake_minimum_required(VERSION 3.10.2)

project(planarityfilter)

if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
	execute_process(
		COMMAND git rev-parse --abbrev-ref HEAD
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		OUTPUT_VARIABLE GIT_BRANCH
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	
	execute_process(
		COMMAND git log -1 --format=%h
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		OUTPUT_VARIABLE GIT_COMMIT_HASH
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
else(EXISTS "${CMAKE_SOURCE_DIR}/.git")
	set(GIT_BRANCH "")
	set(GIT_COMMIT_HASH "")
endif(EXISTS "${CMAKE_SOURCE_DIR}/.git")

configure_file(
	${CMAKE_SOURCE_DIR}/include/version.h.in
	${CMAKE_BINARY_DIR}/generated/version.h
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-O3 -DBOOST_LOG_DYN_LINK -fopenmp")
# note, for HCC probably need to do:
# set(CMAKE_CXX_FLAGS "-O3 -DBOOST_LOG_DYN_LINK -lboost_system -lboost_filesystem -lboost_log -lboost_log_setup -fopenmp")
# find_package(Boost COMPONENTS filesystem system program_options log log_setup REQUIRED)

find_package(Boost COMPONENTS program_options log log_setup REQUIRED)

include_directories(include)
include_directories(${CMAKE_BINARY_DIR}/generated)

file(GLOB SOURCES "src/*.cpp")

list(REMOVE_ITEM SOURCES "src/tests.cpp")

add_executable(planarityfilter ${SOURCES})

target_link_libraries(planarityfilter Boost::program_options Boost::log Boost::log_setup)

find_package(OpenMP)
if(Open_MP_CXX_FOUND)
	target_link_libraries(MyTarget PUBLIC OpenMP::OpenMP_CXX)
endif()
