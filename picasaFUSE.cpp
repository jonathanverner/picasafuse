// main.cpp
#include "picasaFS.h"
#include <string>

int main (int argc, char **argv) {
        PicasaFS picasa;
        // The first 3 parameters are identical to the fuse_main function.
        // The last parameter gives a pointer to a class instance, which is
        // required for static methods to access instance variables/ methods.
        return picasa.main (argc, argv, NULL, &picasa);
}

