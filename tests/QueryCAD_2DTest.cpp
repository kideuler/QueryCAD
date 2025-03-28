#include "QueryCAD_2D.hpp"
#include <gtest/gtest.h>

TEST(QueryCAD2DTest, Constructor) {
    QueryCAD2 query("data/test.step");
    EXPECT_TRUE(query.isValid());
}

TEST(QueryCAD2DTest, Get1DMesh) {
    QueryCAD2 query("data/test.step");
    auto [segments, vertices] = query.get1DMesh();
}

TEST(QueryCAD2DTest, WriteVTK) {
    QueryCAD2 query("data/test.step");
    query.writeVTK("test.vtk");
}