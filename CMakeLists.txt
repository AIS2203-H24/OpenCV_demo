cmake_minimum_required(VERSION 3.17)
project(OpenCV_demo)

set(CMAKE_CXX_STANDARD 17)

if (VCPKG_TARGET_TRIPLET AND VCPKG_MANIFEST_FEATURES MATCHES "opencv")
    set(OpenCV_DIR "${VCPKG_INSTALLED_DIR}/x64-windows/share/opencv4")
else ()
    set(OpenCV_DIR "$ENV{OpenCV_DIR}")
endif ()

find_package(OpenCV CONFIG REQUIRED)

add_executable(OpenCV_demo main.cpp)
target_link_libraries(OpenCV_demo PRIVATE ${OpenCV_LIBRARIES})
