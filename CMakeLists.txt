cmake_minimum_required(VERSION 3.16)
project(QOV VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(qov_codec QOV.cpp)
target_include_directories(qov_codec PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(qov_image_io cli/image_io.cpp)
target_link_libraries(qov_image_io PUBLIC qov_codec)
find_package(OpenCV QUIET COMPONENTS core imgcodecs)
if(OpenCV_FOUND)
    target_compile_definitions(qov_image_io PRIVATE QOV_HAVE_OPENCV=1)
    target_include_directories(qov_image_io PRIVATE ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(qov_image_io PRIVATE ${OpenCV_LIBS})
    message(STATUS "OpenCV found: PNG input enabled")
else()
    message(STATUS "OpenCV not found: PPM input only")
endif()

add_executable(qov-encode encode/encode.cpp)
target_link_libraries(qov-encode PRIVATE qov_image_io)
add_executable(qov-decode decode/decode.cpp)
target_link_libraries(qov-decode PRIVATE qov_image_io)

include(CTest)
if(BUILD_TESTING)
    add_executable(qov-tests tests/test_codec.cpp)
    target_link_libraries(qov-tests PRIVATE qov_codec)
    add_test(NAME codec_roundtrip COMMAND qov-tests)
endif()

find_package(Python3 QUIET COMPONENTS Interpreter)
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(NAME cli_roundtrip
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_cli.py
                $<TARGET_FILE:qov-encode> $<TARGET_FILE:qov-decode>)
endif()
