# Module oriented structure for this project.
# Each directory is a module containing at least its own CMake, source and header files.
# A README file is strongly recommended but not enforced yet.

cmake_minimum_required (VERSION 3.10)   # bump to 3.10 for CMAKE_CXX_STANDARD 17

set (CMAKE_CXX_STANDARD 17)             # Being set at global level, but in the future we could use target_compile_features to set it per target
set (CMAKE_CXX_STANDARD_REQUIRED ON)

project (uw_img_proc)

# Recurse into the every subdirectory. This does not actually
# cause another cmake executable to run. The same process will walk through
# the project's entire directory structure.

# Add module as subdirectory already containing the src/ and include/ folders

# Each module wil be added to the project once it has been tested with its upated CMakeLists.txt
# and current toolchain

add_subdirectory (colorcorrection)
add_subdirectory (contrastenhancement)
add_subdirectory (dehazing)
add_subdirectory (illumination)
add_subdirectory (fusion)
add_subdirectory (videoenhancement)
add_subdirectory (evaluationmetrics)
