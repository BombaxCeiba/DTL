cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)

set(PROJECT_NAME "DTL")

project(${PROJECT_NAME})

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/source SRC_FILES)

option(DTL_COMPILE_STATIC "Compile DTL as static library." ON)

if(DTL_COMPILE_STATIC)
    add_library(${PROJECT_NAME} STATIC ${SRC_FILES})
else()
    add_library(${PROJECT_NAME} SHARED ${SRC_FILES})
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
