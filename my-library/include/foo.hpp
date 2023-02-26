#ifndef FOO_HPP
#define FOO_HPP

#include <string>

class Foo {

    public:

    Foo(const std::string& name);

    void printName();

    void printVersion();

    private:

    std::string m_name;

};

#endif // FOO_HPP