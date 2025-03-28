enable_testing()
find_package(GTest REQUIRED)
include(GoogleTest)

# Copy data from the data directory to the binary directory
file(COPY ${CMAKE_SOURCE_DIR}/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})


add_executable(QueryCAD_2DTest QueryCAD_2DTest.cpp)
target_link_libraries(QueryCAD_2DTest PRIVATE QueryCAD GTest::GTest GTest::Main)
gtest_discover_tests(QueryCAD_2DTest)