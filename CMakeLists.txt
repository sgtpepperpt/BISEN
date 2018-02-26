cmake_minimum_required(VERSION 3.1)
project(bisen)

# paths for binaries
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR})

# external dependencies
find_package(sodium)

# flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -g -O0")
set(CMAKE_CXX_STANDARD 11)

# libs to compile
include_directories("${PROJECT_SOURCE_DIR}/src/Common")
add_subdirectory(src/Common)

file(GLOB SOURCES src/Server/*.cpp)
add_executable(Server ${SOURCES})

file(GLOB SOURCES src/Client/*.cpp src/Client/*.c)
add_library(Client ${SOURCES}) # TODO executable?

# linking
target_link_libraries(Server Common)
target_link_libraries(Client Common)
target_link_libraries(Client sodium)
