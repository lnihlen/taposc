include(ExternalProject)
set(EXT_INSTALL_DIR "${PROJECT_BINARY_DIR}/install-ext" CACHE PATH "External dependencies installation directory")
file(MAKE_DIRECTORY "${EXT_INSTALL_DIR}")

#### liblo
ExternalProject_Add(liblo
    PREFIX ext
    STEP_TARGETS install
    EXCLUDE_FROM_ALL ON
    GIT_REPOSITORY https://github.com/radarsat1/liblo.git
    GIT_TAG master
    GIT_PROGRESS ON
    UPDATE_COMMAND ""
    BUILD_BYPRODUCTS ${EXT_INSTALL_DIR}/lib/liblo.${LIBLO_LIBRARY_SUFFIX}
    CMAKE_ARGS -DWITH_TOOLS=OFF
               -DWITH_TESTS=OFF
               -DWITH_EXAMPLES=OFF
               -DWITH_CPP_TESTS=OFF
               -DWITH_STATIC=ON
               -DWITH_THREADING=ON
               -DCMAKE_INSTALL_PREFIX=${EXT_INSTALL_DIR}
               -DCMAKE_BUILD_TYPE=Release
    SOURCE_SUBDIR cmake
)

### Google Benchmark
set(BENCHMARK_DOWNLOAD_DEPENDENCIES ON CACHE BOOL "Allow benchmark to download dependencies for building.")
add_subdirectory(benchmark)

### oscpack
include_directories(oscpack_1_1_0)
add_library(oscpack STATIC "OSCPackBuild.cpp")
target_include_directories(oscpack INTERFACE oscpack_1_1_0)

### oscpkt (header only)
add_library(oscpkt INTERFACE)
target_include_directories(oscpkt INTERFACE oscpkt-1.2)

### oscpp (header only)
add_library(oscpp INTERFACE)
target_include_directories(oscpp INTERFACE oscpp/include)

