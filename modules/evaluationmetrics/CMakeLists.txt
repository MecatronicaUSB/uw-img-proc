cmake_minimum_required(VERSION 2.8)

project(evaluationmetrics)

# Find OpenCV, you may need to set OpenCV_DIR variable
# to the absolute path to the directory containing OpenCVConfig.cmake file
# via the command line or GUI
# set (OpenCV_DIR C:/opencv/build/x64/vc14/lib)
find_package(OpenCV REQUIRED)

# If the package has been found, several variables will
# be set, you can find the full list with descriptions
# in the OpenCVConfig.cmake file.
# Print some message showing some of them
message(STATUS "OpenCV library status:")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

find_package(CUDA)

if(CUDA_FOUND)
    INCLUDE(FindCUDA)
  # If the package has been found, several variables will
  # be set, you can find the full list with descriptions
  # in the CUDAConfig.cmake file.
  # Print some message showing some of them
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
if(CUDA_FOUND)
  set(FOUND_CUDA 1)
  add_definitions(-D USE_GPU)
  message(STATUS "Configuring for GPU version.")
  file(GLOB evaluationmetrics-files
    "src/main.cpp"
    "src/evaluationmetrics.cpp"
    "include/evaluationmetrics.h"
  ) 
  add_executable(evaluationmetrics ${evaluationmetrics-files})
  # Link your application with OpenCV libraries
target_link_libraries(evaluationmetrics ${OpenCV_LIBS} ${CUDA_LIBRARIES})
else()
  set(FOUND_CUDA 0)
  message(STATUS "Configuring for non-GPU version.")
  message(STATUS "	Expect a slower speed...")
  file(GLOB evaluationmetrics-files
    "src/main.cpp"
    "src/evaluationmetrics.cpp"
    "include/evaluationmetrics.h"
  ) 
  add_executable(evaluationmetrics ${evaluationmetrics-files})
  # Link your application with OpenCV libraries
  target_link_libraries(evaluationmetrics ${OpenCV_LIBS})
endif(CUDA_FOUND)
