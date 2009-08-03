/***************************************************************
 * picasaCache.cpp
 * @Author:      Jonathan Verner (jonathan.verner@matfyz.cz)
 * @License:     GPL v2.0 or later
 * @Created:     2009-07-18.
 * @Last Change: 2009-07-18.
 * @Revision:    0.0
 * Description:
 * Usage:
 * TODO:
 *CHANGES:
 ***************************************************************/


#include "gAPI.h"
#include "pathParser.h"

#include "picasaService.h"
#include "picasaPhoto.h"
#include "picasaAlbum.h"

#include "picasaCache.h"
#include "fusexx.hpp"


#include <boost/bind.hpp>
#include <boost/filesystem/operations.hpp>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>


#include <sstream>
#include <fstream>

#include <sys/stat.h>
#include <sys/types.h>


using namespace std;

const struct cacheElement &cacheElement::operator=( const struct cacheElement &e ) {
    type = e.type;
    name = e.name;
    size = e.size;
    world_readable = e.world_readable;
    writeable = e.writeable;
    last_updated = e.last_updated;
    localChanges = e.localChanges;
    xmlRepresentation = e.xmlRepresentation;
    picasaObj = e.picasaObj;
    switch (e.type) { 
	    case cacheElement::DIRECTORY:
		    authKey = e.authKey;
		    contents = e.contents;
		    return e;
	    case cacheElement::FILE:
		    generated = e.generated;
		    cachePath = e.cachePath;
		    return e;
    }
    return e;
}

void cacheElement::buildPicasaObj(picasaService* picasa) {
  if ( xmlRepresentation == "" ) return;
  switch (type) {
    case cacheElement::DIRECTORY:
      picasaObj = picasa->albumFromXML( xmlRepresentation );
      return;
    case cacheElement::FILE:
      picasaObj = picasa->photoFromXML( xmlRepresentation );
      return;
  }
}


void cacheElement::fromAlbum(picasaAlbum *album) {
  type = cacheElement::DIRECTORY;
  name = album->getTitle();
  size = sizeof(char)*1024;
  world_readable = (album->getAccessType() == picasaAlbum::PUBLIC);
  writeable = false;
  last_updated = 0;
  authKey = album->getAuthKey();
  contents.clear();
  localChanges = false;
  xmlRepresentation = album->getStringXML();
  picasaObj = album;
}

void cacheElement::fromPhoto(picasaPhoto *photo) {
  type = cacheElement::FILE;
  name = photo->getTitle();
  size = photo->getSize();
  world_readable = true;
  writeable = false;
  last_updated = 0;
  authKey = photo->getAuthKey();
  contents.clear();
  localChanges = false;
  xmlRepresentation = photo->getStringXML();
  picasaObj = photo;
  generated = false;
}

void cacheMkdir( string cacheDir, const pathParser &p ) { 
  string path = cacheDir+"/";
  if ( p.haveUser() && p.getUser() != "logs" ) { 
    path+=p.getUser();
    mkdir( path.c_str(), 0755 );
    path+="/";
  }
  if ( p.haveAlbum() ) { 
    path+=p.getAlbum();
    mkdir( path.c_str(), 0755 );
  }
}
    
picasaCache::picasaCache( const string &user, const string &pass, const string &ch):
	api( new gAPI( user, pass, "picasaFUSE" ) ),
	picasa( new picasaService( user, pass ) ),
	work_to_do(false), kill_thread(false), cacheDir( ch )
{
  mkdir( cacheDir.c_str(), 0755 );
  string cacheFName = cacheDir + "/.cache";
  if ( boost::filesystem::exists( cacheFName ) ) { 
    try {
      std::ifstream ifs(cacheFName.c_str(), ios::binary );
      boost::archive::text_iarchive ia(ifs);
      boost::mutex::scoped_lock l(cache_mutex);
      ia >> cache;
      return;
    } catch ( boost::archive::archive_exception ex ) { 
      std::cerr << ex.what() << "\n";
      cache.clear();
    }
  }
    pathParser root(""), logs("/logs");
    struct cacheElement e;
    e.last_updated = time( NULL );
    e.type = cacheElement::DIRECTORY;
    e.name="";
    e.contents.insert("logs");
    e.size=sizeof(char)*1024;
    e.world_readable=true;
    e.writeable = false;
    e.authKey = "";
    putIntoCache( root, e );
    e.type=cacheElement::FILE;
    e.name="logs";
    e.size=0;
    e.generated=true;
    putIntoCache( logs, e );
}

picasaCache::~picasaCache() { 
  string cacheFName = cacheDir + "/.cache";
  std::ofstream ofs(cacheFName.c_str(), ios::binary );
  kill_thread = true;
  {
    boost::archive::text_oarchive oa(ofs);
    boost::mutex::scoped_lock l(cache_mutex);
    oa << cache;
  }
  update_thread->join();
}

void picasaCache::log( string msg ) { 
  pathParser p("/logs");
  struct cacheElement c;
  getFromCache( p, c );
  c.cachePath+=msg;
  c.size = c.cachePath.size();
  putIntoCache( p, c );
}

void picasaCache::pleaseUpdate( const pathParser p ) { 
  if ( ! update_thread ) { 
    update_thread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&picasaCache::update_worker, this)));
  }
  boost::mutex::scoped_lock l(update_queue_mutex);
  update_queue.push_back(p);
  work_to_do = true;
}

void picasaCache::updateUser ( const string userName ) throw ( enum picasaCache::exceptionType ) { 
  set<picasaAlbum> albums;
  set<string> albumDirNames;
  
  log( "picasaCache::updateUser("+userName+"):\n" );

  try { 
    albums = picasa->albumList( userName );
    log( "  albumList ...OK\n" );
  } catch ( enum picasaService::exceptionType ex ) { 
    log( "  User not found (ERROR).\n" );
    throw OBJECT_DOES_NOT_EXIST;
  }


  struct cacheElement c;
  pathParser p;
  
  // Add user to root directory
  p.parse( "/" );
  getFromCache( p, c );
  c.contents.insert( userName );
  putIntoCache( p, c );

  c.writeable = ( userName == picasa->getUser() );
  c.type = cacheElement::DIRECTORY;


  for( set<picasaAlbum>::iterator a = albums.begin(); a != albums.end(); ++a )
    albumDirNames.insert( a->getTitle() );

 
  // Add user directory to cache
  p.parse("/"+userName);
  getFromCache( p, c );
  c.world_readable = true;
  c.contents.clear();
  c.contents = albumDirNames;
  c.last_updated = time( NULL );
  putIntoCache( p, c );
  log( "  update user dir ...OK\n" );
  
  for( set<picasaAlbum>::iterator a = albums.begin(); a != albums.end(); ++a ) { 
    c.fromAlbum( new picasaAlbum( *a ) );
    p.parse( "/"+userName+"/"+a->getTitle() );
    putIntoCache( p, c );
  }

}


void picasaCache::doUpdate( const pathParser p ) throw ( enum picasaCache::exceptionType ) {
  struct cacheElement c;
  pathParser np;
  time_t now = time( NULL );
  if ( ! p.isValid() ) return;
  if ( ! p.haveUser() ) return;

  if ( p.haveUser() ) { 
    np.parse( "/"+p.getUser() );
    if ( getFromCache( np, c ) ) {
      if ( now - c.last_updated > 60 ) updateUser( p.getUser() );
      else {
	stringstream estream;
	estream << "Not updating " << p.getUser() << " since " << now << " - " << c.last_updated << " < 60 \n";
	log(estream.str());
      }
    } else {
	updateUser( p.getUser() );
    }
  }
}

void picasaCache::update_worker() { 
  boost::mutex::scoped_lock l(update_queue_mutex);
  l.unlock();
  pathParser p;
  while( ! kill_thread ) { 
    if ( work_to_do ) { 
      l.lock();
      p = update_queue.front();
      update_queue.pop_front();
      if ( update_queue.size() == 0 ) work_to_do = false;
      l.unlock();
      try { 
	doUpdate( p );
      } catch (enum picasaCache::exceptionType ex ) { }
    }
  }
}



bool picasaCache::getFromCache( const pathParser &p, struct cacheElement &e ) { 
  return getFromCache( p.getHash(), e );
}

void picasaCache::putIntoCache( const pathParser &p, const struct cacheElement &e ) {
  if ( isDir( p ) ) cacheMkdir( cacheDir, p );
  putIntoCache( p.getHash(), e );
}

bool picasaCache::isCached( const pathParser &p ) {
  return isCached( p.getHash() );
}

void picasaCache::clearCache() { 
  boost::mutex::scoped_lock l(cache_mutex);
  cache.clear();
}

list<pathParser> picasaCache::getChildrenInCache( const pathParser &p ) {
  list<pathParser> ret;
  struct cacheElement c = cache[ p.getHash() ];
  pathParser child;
  ret.push_back( p );
  if ( c.type == cacheElement::DIRECTORY ) {
    for( set<string>::iterator ch = c.contents.begin(); ch != c.contents.end(); ++ch ) {
      child = p;
      child.append( *ch );
      ret.push_back( child );
    }
  }
  return ret;
}
  
void picasaCache::removeFromCache( const pathParser &p ) { 
  if ( p.isRoot() ) return;
  boost::mutex::scoped_lock cl(cache_mutex);
  boost::mutex::scoped_lock ql(update_queue_mutex);
  list<pathParser> children = getChildrenInCache( p );
  for( list<pathParser>::iterator child = children.begin(); child != children.end(); ++child ) { 
    update_queue.remove( *child );
    cache.erase( child->getHash() );
  }
  ql.unlock();
  pathParser parent = p;
  parent.toParent();
  string me = p.getLastComponent();
  struct cacheElement c = cache[parent.getHash()];
  c.contents.erase(me);
  cache[parent.getHash()] = c;
  cl.unlock();
  log( "rm -rf " + cacheDir + "/" + p.getFullName() + "\n" );
  boost::filesystem::remove_all( cacheDir + "/" + p.getFullName() );
}
  


bool picasaCache::getFromCache( const string &key, struct cacheElement &e ) {
  boost::mutex::scoped_lock l(cache_mutex);
  if ( cache.find( key ) == cache.end() ) return false;
  e = cache[key];
  return true;
}

void picasaCache::putIntoCache( const std::string &key, const struct cacheElement &e ) {
  boost::mutex::scoped_lock l(cache_mutex);
  cache[key] = e;
}

bool picasaCache::isCached( const string &key ) {
  boost::mutex::scoped_lock l(cache_mutex);
  return (cache.find(key) != cache.end() );
}


bool picasaCache::isDir( const pathParser &path ) {
  if ( path.haveUser() && path.getUser() == "logs" ) return false;
  return ( ! path.haveImage() && path.isValid() );
}

bool picasaCache::isFile( const pathParser &path ) {
  if ( path.haveUser() && ! path.haveAlbum() && path.getUser() == "logs" ) return true;
  return ( path.haveImage() && path.isValid() );
}

bool picasaCache::exists( const pathParser &path ) {
  if ( ! path.isValid() ) return false;
  if ( isCached( path ) ) return true;
  if ( ! path.haveUser() ) return true;
  if ( path.getUser() == "test" ) { 
    if ( ! path.haveAlbum() ) return true;
    return ( path.getAlbum() == "album1" && ! path.haveImage() );
  }
  return false;
}

int picasaCache::getAttr( const pathParser &path, struct stat *stBuf ) { 
  struct cacheElement e;
  if ( ! getFromCache( path, e ) ) {
    // If we are not looking at a subdirectory of the root
    // and the path is not cached it doesn't exist 
    // (at least not now, maybe at some later point, 
    // when we update the parent directory)
    if ( path.getType() != pathParser::USER ) return -ENOENT;
    try {
      doUpdate( path );
    } catch ( enum exceptionType ex ) { 
      return -ENOENT;
    }
    if ( ! getFromCache( path, e ) ) return -ENOENT;
  }
  stBuf->st_size = e.size;
  switch ( e.type ) { 
	  case cacheElement::DIRECTORY:
		  stBuf->st_mode = S_IFDIR | 0755;
		  stBuf->st_nlink = 2;
		  return 0;
	  case cacheElement::FILE:
		  stBuf->st_mode = S_IFREG;
		  stBuf->st_nlink = 1;
		  return 0;
  }
}

void picasaCache::needPath( const pathParser &path ) {
  pleaseUpdate( path );
}

void picasaCache::rmdir( const pathParser &p ) throw ( enum picasaCache::exceptionType ) {
  if ( p.isUser() ) { 
    if ( p.getUser() == "logs" ) throw ACCESS_DENIED;
    removeFromCache( p );
    return;
  } else throw UNIMPLEMENTED;
}

set<string> picasaCache::ls( const pathParser &path ) throw ( enum picasaCache::exceptionType) {
  set<string> ret;
  if ( ! isDir(path) ) return ret;
  struct cacheElement e;
  if ( getFromCache( path , e ) ) {
    pleaseUpdate( path );
    return e.contents;
  } else { 
    doUpdate( path );
    if ( getFromCache( path, e ) ) { 
      return e.contents;
    } else {
      log ( "picasaCache::ls("+path.getHash()+"): exception." );
      throw OBJECT_DOES_NOT_EXIST;
    }
  }
}

int fillBufFromString( string data, char *buf, size_t size, off_t offset ) { 
  int len = data.length();
  if ( offset < len ) { 
    if ( offset + size > len )
      size = len-offset;
    memcpy( buf, data.c_str() + offset, size );
    return size;
  } else return 0;
}

int picasaCache::read( const pathParser &path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi ) {
  if ( ! exists( path ) ) return 0;
  if ( ! isFile( path ) ) return 0;

  struct cacheElement e;
  if ( getFromCache( path, e ) ) { 
    if ( e.generated ) {
      if ( e.cachePath.size() > 0 ) { 
	return fillBufFromString( e.cachePath, buf, size, offset );
      } else return fillBufFromString( "Data not yet available...\n", buf, size, offset );
    }
    int fd = open( e.cachePath.c_str() , O_RDONLY );
    if ( fd == -1 ) { 
      char *errBuf = strerror( errno );
      string err = "Error opening "+e.cachePath+" (";
      err+=errBuf;
      err+=")\n";
      return fillBufFromString( err, buf, size, offset );
    }
    return pread(fd,buf,size, offset);
  }
  return fillBufFromString( "Data not yet available...", buf, size, offset );
}


