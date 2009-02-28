// main.cpp
#include "hello.hpp"

int main (int argc, char **argv) {
        HelloWorld hello;

        // The first 3 parameters are identical to the fuse_main function.
        // The last parameter gives a pointer to a class instance, which is
        // required for static methods to access instance variables/ methods.
        return hello.main (argc, argv, NULL, &hello);
}

