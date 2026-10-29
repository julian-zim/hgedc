add_executable(TestExecutable test_executable.cpp)

add_executable(TestGedlib test_gedlib.cpp)
target_link_libraries(TestGedlib libgxlgedlib.so)

pybind11_add_module(TestPybind test_pybind.cpp test_pybind.h)
set_target_properties(TestPybind PROPERTIES SUFFIX ".so")
