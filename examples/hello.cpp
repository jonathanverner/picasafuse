// hello.cpp
#include "hello.hpp"

#include <string>

using namespace std;

// Constructor
HelloWorld::HelloWorld () : 
                m_strHelloWorld ("Hello World!\n"),
                m_strPath ("/hello") {
        // all we're doing is initialize the member variables
}

int HelloWorld::getattr (const char *path, struct stat *stbuf) {
        int res = 0;
        string strPath (path); // convert the path into a std::string

        // Zero out the file stat buffer
        memset (stbuf, 0, sizeof (struct stat));
        if (strPath == "/") {
                stbuf->st_mode = S_IFDIR | 0755;
                stbuf->st_nlink = 2;
        }
        // Compare the private variable of the HelloWorld object to the
        // passed-in path parameter from fuse
        else if (self->m_strPath == strPath) {
                stbuf->st_mode = S_IFREG | 0444; // read-only
                stbuf->st_nlink = 1;
                stbuf->st_size = self->m_strHelloWorld.length();
        }
        else {
                res = -ENOENT;
        }

        return res;
}

int HelloWorld::readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                off_t offset, struct fuse_file_info *fi) {
        string strPath (path);
        (void) offset;
        (void) fi;

        if(strPath != "/")
                return -ENOENT;

        filler(buf, ".", NULL, 0);
        filler(buf, "..", NULL, 0);

        // self->m_strPath contains the std::string "/hello"
        // Applying .c_str() returns a const char * containing "/hello"
        // Adding 1 to that pointer skips the '/' character and thus outputs
        // just "hello"
        filler(buf, self->m_strPath.c_str() + 1, NULL, 0);

        return 0;
}

int HelloWorld::open(const char *path, struct fuse_file_info *fi) {
        string strPath (path);

        // The only file that exists is "/hello"
        if(strPath != self->m_strPath)
                return -ENOENT;

        // Only allow read-only access
        if((fi->flags & 3) != O_RDONLY)
                return -EACCES;

        return 0;
}

int HelloWorld::read(const char *path, char *buf, size_t size, off_t offset,
                struct fuse_file_info *fi) {
        string strPath (path);
        size_t len;
        (void) fi;

        if(strPath != self->m_strPath)
                return -ENOENT;

        len = self->m_strHelloWorld.length ();
        if (offset < len) {
                if (offset + size > len)
                        size = len - offset;
                memcpy (buf, self->m_strHelloWorld.c_str () + offset, size);
        } else
                size = 0;

        return size;
}