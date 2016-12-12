cmake_minimum_required(VERSION 2.6)

project(sigmatig)
set(EXE "main")

enable_testing()

set(warnings
    "-Wall"
    "-Wextra"
    "-pedantic-errors"
    "-Wfatal-errors"
    "-Wcast-align"
    "-Wmissing-declarations"
    "-Wredundant-decls"
    "-Wuninitialized"
    "-Wno-unused-parameter"
    "-Wno-missing-field-initializers")

set(cxxflags
    "-std=c++14"
    "-g3"
    "-O2"
    #"-flto"
    ${warnings})

#set(ldflags
#    "-flto")

string(REGEX REPLACE ";" " " cxxflags "${cxxflags}")
string(REGEX REPLACE ";" " " ldflags "${ldflags}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${cxxflags}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${ldflags}")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(sources
    main.cc
    expression.cc
    environment.cc
    builtin-functions.cc)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})
find_package(Boost REQUIRED COMPONENTS system filesystem)

add_executable(${EXE} ${sources})
target_link_libraries(${EXE} ${Boost_LIBRARIES} dl pthread)
