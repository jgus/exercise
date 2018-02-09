
#include <gtest/gtest.h>
#include <iostream>
#include <string>

#include "obfuscated/obfuscated_string.hpp"

TEST(ObfuscatedString, Test)
{
    constexpr auto s = obfuscated::obfuscate("Here's a SECRET! 87bd58b4-0354-4e8f-a3e6-1cdac5048cfa");

    std::cout << "This test doesn't actually check anything, but it has a secret in the source code, and it can do things with that secret, such and print it like this:" << std::endl;
    std::cout << std::string{ s } << std::endl;
    std::cout << "And yet, if you search the binary, nothing like that secret will be found. Of course, it's just \"security by obscurity\", but useful for some things, and an interesting use of compile-time evaluation." << std::endl;

    SUCCEED();
}
