#include <iostream>
#include "foo.hpp"

Foo::Foo(const std::string& name): m_name{name} {}

void Foo::printName() {
    std::cout << "Name: " << m_name << std::endl;
}

void Foo::printVersion() {
    std::cout << "Version: " << FOO_MAJOR_VERSION << "." << FOO_MINOR_VERSION << std::endl;
}