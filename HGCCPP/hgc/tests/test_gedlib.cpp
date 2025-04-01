#include <iostream>
#include "src/env/ged_env.hpp"

ged::GEDEnv<double, ged::Options::GXLNodeEdgeType, ged::Options::GXLNodeEdgeType> env;

int main() {
    std::cout << "GEDLib Test: Success!" << std::endl;
    return EXIT_SUCCESS;
}
