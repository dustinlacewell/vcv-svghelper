#include <gtest/gtest.h>

#include "foo.hpp"

TEST(FooTest, PrintNameTest) {
    // Arrange
    Foo foo{"Fox Molder"};
    // Action
    foo.printName();
    // Result
    EXPECT_TRUE(true);
}

TEST(FooTest, PrintVersionTest) {
    // Arrange
    Foo foo{"Fox Molder"};
    // Action
    foo.printVersion();
    // Result
    EXPECT_TRUE(true);
}
