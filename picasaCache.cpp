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

void cacheMkdir( string cacheDir, const pathParser &p ) { 
  string path = cacheDir+"/";
  if ( p.haveUser() ) { 
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


void picasaCache::updateUser( const string userName ) { 
  pathParser p( "/" );
  struct cacheElement c,d;
  log( "picasaCache::updateUser("+userName+")\n" );
  getFromCache( p, c );
  c.contents.insert( userName );
  putIntoCache( p, c );
  p.parse("/"+userName);
  set<string> albums = api->albumList( userName );

  getFromCache( p, c );
  c.contents.clear();
  c.contents = api->albumList( userName );
  c.last_updated = time( NULL );
  putIntoCache( p, c );
  d.type = cacheElement::DIRECTORY;
  d.contents.clear();
  d.world_readable = true; //FIXME
  d.writeable = ( userName == api->getUser() );
  d.last_updated = 0;
  for( set<string>::iterator dir = c.contents.begin(); dir != c.contents.end(); ++dir ) { 
    d.name = *dir;
    d.authKey = "";
    p.parse( "/"+userName+"/"+*dir );
    putIntoCache( p, d );
  }
}



void picasaCache::doUpdate( const pathParser p ) {
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
      doUpdate( p );
    }
  }
}



bool picasaCache::getFromCache( const pathParser &p, struct cacheElement &e ) { 
  return getFromCache( p.getHash(), e );
}

void picasaCache::putIntoCache( const pathParser &p, const struct cacheElement &e ) {
  putIntoCache( p.getHash(), e );
}

bool picasaCache::isCached( const pathParser &p ) {
  return isCached( p.getHash() );
}

void picasaCache::clearCache() { 
  boost::mutex::scoped_lock l(cache_mutex);
  cache.clear();
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

void picasaCache::needPath( const pathParser &path ) {
  pleaseUpdate( path );
}

size_t picasaCache::getSize( const pathParser &p ) {
  struct cacheElement e;
  if ( ! exists(p) ) return -1;
  if ( isDir( p ) ) return sizeof(char)*1024;
  if ( getFromCache( p, e ) ) return e.size;
  return 0;
}

set<string> picasaCache::ls( const pathParser &path ) {
  set<string> ret;
  if ( ! isDir(path) ) return ret;
  struct cacheElement e;
  if ( getFromCache( path , e ) ) ret = e.contents;
  pleaseUpdate( path );
  return ret;
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


