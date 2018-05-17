cmake_minimum_required(VERSION 3.1)
project(bisen)

# variables
set(USE_WIKI_TEST 1)
set(SPARSEPP_DIR ${CMAKE_SOURCE_DIR}/sparsepp)

########################################################################################################################
# paths for binaries
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR})

# external dependencies
#find_package(sodium REQUIRED)

# flags (-DLOCALTEST -DVERBOSE)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -fPIC -Wall -Wno-unused-function -Wno-unused-variable")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -fPIC -Wall -Wno-unused-function -Wno-unused-variable")
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

# lib common
include_directories(${PROJECT_SOURCE_DIR}/src/Common)
add_subdirectory(src/Common)

# lib untrusted
add_library(Untrusted ${PROJECT_SOURCE_DIR}/src/tsgx/src/untrusted/untrusted_free_bytes.c)
target_include_directories(Untrusted PRIVATE ${PROJECT_SOURCE_DIR}/src/tsgx/include)

# lib iee
file(GLOB IEE_SOURCES src/Iee/*.c src/Iee/util/*.c)
add_library(Iee ${IEE_SOURCES})
target_link_libraries(Iee sodium)
add_custom_command(TARGET Iee POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ../bin/libIee.a ../src/tsgx/test_bisen/f/iee.a#TODO check this for new client and server tsgx
)

# the following may not be needed in the future
add_custom_command(TARGET Iee POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ../bin/libIee.a ../src/tsgx-client/test_bisen/f/iee.a)

# server module
file(GLOB SERVER_SOURCES src/Server/*.cpp)
add_executable(Server ${SERVER_SOURCES})
target_include_directories(Server PRIVATE ${SPARSEPP_DIR})
target_link_libraries(Server Common pthread)

# client module
file(GLOB CLIENT_SOURCES src/Client/*.cpp src/Client/*.c)
add_library(Client ${CLIENT_SOURCES})
target_link_libraries(Client Common sodium)

# test module
if(USE_WIKI_TEST)
    set(TEST_SRC ${PROJECT_SOURCE_DIR}/src/Test/generate_wiki.cpp)
else()
    set(TEST_SRC ${PROJECT_SOURCE_DIR}/src/Test/generate_enron.cpp)
endif()

add_executable(Test ${TEST_SRC})
target_link_libraries(Test Common Iee Client Untrusted)

# mpc modules
add_custom_target(
   mpc_server
   COMMAND make
   WORKING_DIRECTORY ../src/tsgx
)

add_custom_target(
   mpc_client
   COMMAND make
   WORKING_DIRECTORY ../src/tsgx-client
)
