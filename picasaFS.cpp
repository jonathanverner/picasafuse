// hello.cpp
#include "picasaFS.h"
#include "picasaCache.h"
#include "pathParser.h"

#include <string>

#define _XOPEN_SOURCE 500

#include <unistd.h>
#include <errno.h>
#include <iostream>

using namespace std;


// Constructor
PicasaFS::PicasaFS ( const string &user, const string &pass, const string &cacheDir ) : 
		cache( new picasaCache( user, pass, cacheDir ) )
{
        // all we're doing is initialize the member variables
}





int PicasaFS::getattr (const char *path, struct stat *stbuf) {
  pathParser p(path);
  // Zero out the file stat buffer
  memset (stbuf, 0, sizeof (struct stat));

  return self->cache->getAttr( p, stbuf );
}

int PicasaFS::readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                off_t offset, struct fuse_file_info *fi) {

	pathParser p(path);

	if ( ! self->cache->isDir( p ) ) return -ENOENT;

	set<string> dirList;

	try { 
	  dirList = self->cache->ls( p );
	} catch ( picasaCache::exceptionType ex ) { 
	  return -ENOENT;
	}

	filler( buf, ".", NULL, 0 );
        filler( buf, "..", NULL, 0);

	for( set<string>::iterator it = dirList.begin(); it != dirList.end(); ++it )
	  filler( buf, it->c_str(), NULL, 0 );

        return 0;
}

int PicasaFS::fuse_open(const char *path, struct fuse_file_info *fi) {
  pathParser p(path);
  if ( ! self->cache->exists( p ) ) return -ENOENT;
  self->cache->needPath( p );
  // Only allow read-only access
  if((fi->flags & 3) != O_RDONLY)
    return -EACCES;
  return 0;
}

int PicasaFS::read(const char *path, char *buf, size_t size, off_t offset,
                struct fuse_file_info *fi) {
        size_t len;
        (void) fi;
	pathParser p(path);
	if ( ! self->cache->exists( p ) ) return -ENOENT;
	return self->cache->read( p, buf, size, offset, fi );
}

