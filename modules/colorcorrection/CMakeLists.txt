# Bump to CMake to 3.10 or higher
cmake_minimum_required(VERSION 3.10)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

project(colorcorrection)

# Find OpenCV, you may need to set OpenCV_DIR variable
# to the absolute path to the directory containing OpenCVConfig.cmake file
# via the command line or GUI


# Probe for OS and set the appropriate flags
if (UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-missing-field-initializers -Wno-unknown-pragmas -Wno-unused-but-set-variable -Wno-unused-result -Wno-strict-aliasing -Wno-maybe-uninitialized -Wno-sign-compare -Wno-unused-value -Wno-ignored-attributes -Wno-unknown-warning-option -Wno-deprecated-declarations -Wno-ignored-qualifiers -Wno-unused-local-typedefs -Wno-attributes -Wno-ignored-attributes -Wno-unknown-warning-option -Wno-deprecated-declarations -Wno-ignored-qualifiers -Wno-unused-local-typedefs -Wno-attributes -Wno-ignored-attributes -Wno-unknown-warning-option -Wno-deprecated-declarations -Wno-ignored-qualifiers -Wno-unused-local-typedefs -Wno-attributes -Wno-ignored-attributes -Wno-unknown-warning-option -Wno-deprecated-declarations -Wno-ignored-qualifiers -Wno-unused-local-typedefs -Wno-attributes -Wno-ignored-attributes -Wno-unknown-warning-option -Wno-deprecated-declarations -Wno-ignored-qualifiers -Wno-unused-local-typedefs -Wno-attributes -Wno-ignored-attributes -Wno-unknown-warning-option -Wno-deprecated-declarations -Wno-ignored-qualifiers -Wno-unused-local-typedefs -Wno-attributes -Wno-ignored-attributes -Wno-unknown-warning-option -Wno-deprecated-declarations -Wno-ignored-qualifiers -Wno-unused-local-typedefs -Wno-attributes -Wno-ignored-attributes -Wno-unknown-warning-option -Wno-deprecated-declarations -Wno-ignored-qualifiers -Wno-unused-local-typedefs -Wno-attributes -Wno-ignored-attributes -Wno-unknown-warning-option -Wno-deprecated-declarations -Wno-ignored-qualifiers -Wno-unused-local-typedefs -Wno-attributes -Wno-ignored-attributes -Wno-unknown-warning-option")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O2 -g -DNDEBUG")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -Os -DNDEBUG")
    # set the OpenCV_DIR to the directory containing OpenCVConfig.cmake on Linux
    set(OpenCV_DIR /usr/local/lib/cmake/opencv4)
elseif (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    # set the OpenCV_DIR to the directory containing OpenCVConfig.cmake file for Windows
    set(OpenCV_DIR "C:/opencv/build/x64/vc15/lib")
    set (OpenCV_DIR "C:/opencv/build/x64/vc14/lib")
endif()

find_package(OpenCV REQUIRED)

# If the package has been found, several variables will be set, you can find the full list with descriptions
# in the OpenCVConfig.cmake file. Print some message showing some of them
message(STATUS "OpenCV library status:")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

find_package(CUDA)

if(CUDA_FOUND)
    INCLUDE(FindCUDA)
  # If the package has been found, CUDAConfig.cmake wil set the required vars, let's print them
  message(STATUS "CUDA library status:")
  message(STATUS "    toolkit root dir: ${CUDA_TOOLKIT_ROOT_DIR}")
  message(STATUS "    version: ${CUDA_VERSION}")
  message(STATUS "    libraries: ${CUDA_LIBRARIES}")
  message(STATUS "    include path: ${CUDA_INCLUDE_DIRS}")
endif(CUDA_FOUND)

if(CMAKE_VERSION VERSION_LESS "2.8.11")
  # Add OpenCV headers location to your include paths
  include_directories(${OpenCV_INCLUDE_DIRS})
endif()

# Declare the executable target built from your sources
# If detect CUDA, then select GPU implementation as prefered method
if(CUDA_FOUND AND USE_CUDA)
  set(FOUND_CUDA 1)
  add_definitions(-D USE_GPU)
  message(STATUS "Configuring for GPU version.")
  file(GLOB colorcorrection-files
    "src/colorcorrection.cpp"
    "src/main.cpp"
    "include/colorcorrection.h"
  ) 
  add_executable(colorcorrection ${colorcorrection-files})
  # Link your application with OpenCV libraries
target_link_libraries(colorcorrection ${OpenCV_LIBS} ${CUDA_LIBRARIES})
else()
  set(FOUND_CUDA 0)
  message(STATUS "Configuring for non-GPU version.")
  message(STATUS "Expect a slower speed...")
  file(GLOB colorcorrection-files
    "src/colorcorrection.cpp"
    "src/main.cpp"
    "include/colorcorrection.h"
  ) 
  add_executable(colorcorrection ${colorcorrection-files})
  # Link your application with OpenCV libraries
  target_link_libraries(colorcorrection ${OpenCV_LIBS})
endif(CUDA_FOUND AND USE_CUDA)