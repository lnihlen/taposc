include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    #    ${CMAKE_CURRENT_BINARY_DIR}
    ${EXT_INSTALL_DIR}/include
)

add_executable(cppbench
    bench.cpp
)

target_link_libraries(cppbench
    ${EXT_INSTALL_DIR}/lib/liblo.${LIBLO_LIBRARY_SUFFIX}
    benchmark::benchmark
    oscpack
    oscpkt
    oscpp
)

add_dependencies(cppbench
    liblo-install
)

add_executable(e2e
    e2e.cpp
)

target_link_libraries(e2e
    ${EXT_INSTALL_DIR}/lib/liblo.${LIBLO_LIBRARY_SUFFIX}
    gflags
    oscpack
    oscpkt
    oscpp
)
