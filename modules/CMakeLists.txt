# CMakeLists files in this project can
# refer to the root source directory of the project as ${HELLO_SOURCE_DIR} and
# to the root binary directory of the project as ${HELLO_BINARY_DIR}.
cmake_minimum_required (VERSION 2.8.11)
project (uw_img_proc)

# Recurse into the every subdirectory. This does not actually
# cause another cmake executable to run. The same process will walk through
# the project's entire directory structure.

add_subdirectory (colorcorrection)
add_subdirectory (contrastenhancement)
add_subdirectory (dehazing)
add_subdirectory (evaluationmetrics)
add_subdirectory (fusion)
