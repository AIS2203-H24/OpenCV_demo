cmake_minimum_required(VERSION 3.17)
project(OpenCV_demo)

set(CMAKE_CXX_STANDARD 20)

if (VCPKG_TARGET_TRIPLET)
    set(OpenCV_DIR "${VCPKG_INSTALLED_DIR}/x64-windows/share/opencv4")
else ()
    set(OpenCV_DIR "$ENV{OpenCV_DIR}")
endif ()

include(FetchContent)
set(SIMPLE_SOCKET_BUILD_TESTS OFF)
FetchContent_Declare(
        SimpleSocket
        GIT_REPOSITORY https://github.com/markaren/SimpleSocket.git
        GIT_TAG 13380c962c9681d0e078d1d95fc092803ccc1bfe
)
FetchContent_MakeAvailable(SimpleSocket)

find_package(OpenCV CONFIG REQUIRED)

file(COPY "${CMAKE_SOURCE_DIR}/data" DESTINATION "${CMAKE_BINARY_DIR}")

add_executable(OpenCV_demo main.cpp)
target_link_libraries(OpenCV_demo PRIVATE ${OpenCV_LIBRARIES})

if (UNIX)
    target_link_libraries(OpenCV_demo PRIVATE pthread)
endif ()

add_executable(image_sender image_sender.cpp)
target_link_libraries(image_sender PRIVATE simple_socket)

add_executable(image_receiver image_receiver.cpp)
target_link_libraries(image_receiver PRIVATE simple_socket ${OpenCV_LIBRARIES})
