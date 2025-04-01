#include "test_pybind.h"

//#include <CLI11.hpp>

void test_pybind::test() {
    std::cout << "Pybind Test: Success!" << std::endl;
}

std::map<int, int> test_pybind::testMap() {
    std::map<int, int> testMap = std::map<int, int>();
    testMap[0] = 5;
    testMap[1] = 10;
    testMap[2] = 25;
    return testMap;
}

PYBIND11_MODULE(TestPybind, module) {
    pybind11::class_<test_pybind>(module, "test_pybind")
            .def(pybind11::init<>())
            .def("test", &test_pybind::test)
            .def("test_dict", &test_pybind::testMap);
}
