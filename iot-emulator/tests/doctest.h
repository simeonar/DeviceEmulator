// Single-header doctest framework (https://github.com/doctest/doctest)
// For brevity, you can download the latest version from https://raw.githubusercontent.com/doctest/doctest/master/doctest/doctest.h
// Here, just a stub to allow CMake to configure. Please replace with the real file for full functionality.
#ifndef DOCTEST_STUB_H
#define DOCTEST_STUB_H
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define TEST_CASE(x) void test_##x()
#define CHECK(x) if (!(x)) throw "Test failed: " #x;
#endif
