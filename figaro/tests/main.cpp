#include <gtest/gtest.h>
#include <boost/program_options.hpp>
#include "UtilTest.h"

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    std::string globalDataPath = argv[1];
    GL_DATA_PATH = globalDataPath;
    return RUN_ALL_TESTS();
}
