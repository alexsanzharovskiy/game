cmake_minimum_required(VERSION 3.10)
project(SlotBackend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Wpedantic)


find_package(Boost)

if (Boost_FOUND)
    message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")
endif()

file(GLOB_RECURSE SRC_FILES
    "src/*.cpp"
)

add_executable(slot_backend ${SRC_FILES})

target_include_directories(slot_backend PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${Boost_INCLUDE_DIRS}
)

target_compile_definitions(slot_backend PRIVATE
    BOOST_ERROR_CODE_HEADER_ONLY
    BOOST_SYSTEM_NO_DEPRECATED
)
