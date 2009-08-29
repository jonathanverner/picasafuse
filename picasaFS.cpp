// hello.cpp
#include "picasaFS.h"
#include "picasaCache.h"
#include "pathParser.h"

#include <string>

#define _XOPEN_SOURCE 500

#include <unistd.h>
#include <errno.h>
#include <attr/xattr.h>

#include <iostream>

using namespace std;


// Constructor
PicasaFS::PicasaFS ( const string &user, const string &pass, const string &cacheDir, int updateInterval ) : 
		cache( new picasaCache( user, pass, cacheDir, updateInterval ) ), UID( getuid() ), GID( getgid() )
{
        // all we're doing is initialize the member variables
}

void PicasaFS::destroy( void * ) { 
  delete self->cache;
}





int PicasaFS::getattr (const char *path, struct stat *stbuf) {
  pathParser p(path);
  cerr << "getattr("<<p.getFullName()<<"): start"<<endl;
  // Zero out the file stat buffer
  memset( stbuf, 0, sizeof (struct stat) );
  int ret = self->cache->getAttr( p, stbuf );
  stbuf->st_gid = self->GID;
  stbuf->st_uid = self->UID;
  cerr << "getattr("<<p.getFullName()<<"): success ("<<ret<<")"<<endl;
  return ret;
}

int PicasaFS::getxattr( const char *path, const char *attrName, char *buf, size_t sz ) {
  pathParser p( path );
  string attr( attrName );
  string value;
  int userPos = attr.find( "user." );
  if ( userPos == string::npos ) return -ENOATTR;
  try {
    value = self->cache->getXAttr( p, attr.substr( 5 ) );
  } catch ( enum picasaCache::exceptionType ex ) {
    return -ENOATTR;
  }
  if ( sz == 0 ) return value.size();
  if ( value.size()  > sz ) return -ERANGE;
  memcpy( buf, value.c_str(), value.size() );
  return value.size();
}

int PicasaFS::listxattr( const char *path, char *buf, size_t sz ) {
  list<string> attrList;
  int ret = 0;
  pathParser p( path );
  try {
    attrList = self->cache->listXAttr( p );
  } catch ( enum picasaCache::exceptionType ex ) {
    return -ENOENT;
  }
  string attrName;
  for( list<string>::iterator attr = attrList.begin(); attr != attrList.end(); ++attr ) { 
    attrName = "user." + *attr;
    if ( 0 < sz && sz < ret + attrName.size() + 1 ) return -ERANGE;
    if ( sz > 0 && buf != NULL ) {
      memcpy( buf, attrName.c_str(), attrName.size() + 1 );
      buf += attrName.size()+1;
    }
    ret += attrName.size()+1;
  }
  return ret;
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
  cerr << "fuse_open("<<p.getFullName()<<"): start"<<endl;
  self->cache->needPath( p );
  // Only allow read-only access
  if((fi->flags & 3) != O_RDONLY)
    return -EACCES;
  cerr << "fuse_open("<<p.getFullName()<<"): success"<<endl;
  return 0;
}

int PicasaFS::read(const char *path, char *buf, size_t size, off_t offset,
                struct fuse_file_info *fi) {
        size_t len;
        (void) fi;
	pathParser p(path);
	cerr << "read("<<p.getFullName()<<"): start"<<endl;
	if ( ! self->cache->exists( p ) ) return -ENOENT;
	int ret = self->cache->read( p, buf, size, offset, fi );
	cerr << "read("<<p.getFullName()<<"): success ("<< ret <<")"<<endl;
	return ret;
}

int PicasaFS::rmdir( const char *path ) { 
  pathParser p(path);
  try { 
    self->cache->rmdir( p );
  } catch ( enum picasaCache::exceptionType ex ) { 
    switch ( ex ) { 
	    case picasaCache::ACCESS_DENIED:
		    return -EACCES;
	    case picasaCache::UNIMPLEMENTED:
		    return -EPERM;
	    default:
		    return -ENOENT;
    }
  }
  return 0;
}

int PicasaFS::mkdir( const char *path, mode_t mode ) {
  pathParser p(path);
  try {
    self->cache->my_mkdir( p );
    cerr << "mkdir( " << p.getFullName() << " ): success ! " << endl;
  } catch( enum picasaCache::exceptionType ex ) { 
    switch( ex ) { 
      case picasaCache::ACCESS_DENIED:
	return -EACCES;
      case picasaCache::UNIMPLEMENTED:
	return -EPERM;
      default:
	return -ENOENT;
    }
  }
  return 0;
}



