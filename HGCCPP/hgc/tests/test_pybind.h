#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

class test_pybind {

public:
    void test();
    std::map<int, int> testMap();

};
