pybind11_add_module(HGCGED HGCGED.cpp HGCGED.h UserDefined.hpp)
set_target_properties(HGCGED PROPERTIES SUFFIX ".so")
target_link_libraries(HGCGED PRIVATE libgxlgedlib.so)

add_subdirectory(temp)
