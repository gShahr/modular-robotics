#define BOOST_TEST_MODULE MetaModuleTest
#include <boost/test/included/unit_test.hpp>
#include <fstream>
#include <iostream>
#include <vector>
#include "metamodule.h"

struct MetaModuleFixture {
    MetaModule* metaModule;
    MetaModuleFixture() {
        std::ofstream file("temp_metamodule.txt");
        file << "##\n##";
        file.close();
        metaModule = new MetaModule("docs/examples/3_by_3_metamodule.json", 2, 3);
    }
    ~MetaModuleFixture() {
        delete metaModule;
        std::remove("temp_metamodule.txt");
    }
};

BOOST_FIXTURE_TEST_SUITE(MetaModuleTestSuite, MetaModuleFixture)

BOOST_AUTO_TEST_CASE(ConstructorTest) {
    BOOST_CHECK_EQUAL(metaModule->coords.size(), 5);
}

BOOST_AUTO_TEST_SUITE_END()