project(particlefiltertest)
cmake_minimum_required(VERSION 3.10)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src 
                    ${CMAKE_CURRENT_SOURCE_DIR}/test)

set(SRC
	src/definitions.cpp
    src/coords.cpp
	src/particlefilter.cpp
    src/playingfield.cpp
    src/visiondefinitions.cpp
    src/platform.cpp
    src/mathtoolbox.cpp
    test/locatest.cpp

)

add_compile_options("-std=c++14")
add_executable(${PROJECT_NAME} ${SRC})
