set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/hgc/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/hgc/lib)

include_directories(${CMAKE_SOURCE_DIR}/include/dicoda
                    ${CMAKE_SOURCE_DIR}/include/gedlib
                    ${CMAKE_SOURCE_DIR}/include/gedlib/ext/boost.1.69.0
                    ${CMAKE_SOURCE_DIR}/include/gedlib/ext/eigen.3.3.4/Eigen
                    ${CMAKE_SOURCE_DIR}/include/gedlib/ext/fann.2.2.0/include
                    ${CMAKE_SOURCE_DIR}/include/gedlib/ext/libsvm.3.22
                    ${CMAKE_SOURCE_DIR}/include/gedlib/ext/lsape.5/include
                    ${CMAKE_SOURCE_DIR}/include/gedlib/ext/lsape.5/cpp/include
                    ${CMAKE_SOURCE_DIR}/include/gedlib/ext/nomad.3.8.1/src
                    ${CMAKE_SOURCE_DIR}/include/gedlib/ext/nomad.3.8.1/ext/sgtelib/src)
link_directories(${CMAKE_SOURCE_DIR}/include/gedlib/lib)

add_subdirectory(tests)
add_subdirectory(src)
