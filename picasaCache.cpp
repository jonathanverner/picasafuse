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

#include "convert.h"


#include <boost/bind.hpp>
#include <boost/filesystem/operations.hpp>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/date_time.hpp> 


#include <sstream>
#include <fstream>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

#include <string.h>


using namespace std;

string picasaCache::exceptionString( exceptionType ex ) {
  string ret;
  switch( ex ) { 
    case OBJECT_DOES_NOT_EXIST:
      ret = "OBJECT_DOES_NOT_EXIST";
      break;
    case UNIMPLEMENTED:
      ret = "UNIMPLEMENTED";
      break;
    case ACCESS_DENIED:
      ret = "ACCESS_DENIED";
      break;
    case OPERATION_NOT_SUPPORTED:
      ret = "OPERATION_NOT_SUPPORTED";
      break;
    case UNEXPECTED_ERROR:
      ret = "UNEXPECTED_ERROR";
      break;
    case OPERATION_FAILED:
      ret = "OPERATION_FAILED";
      break;
    case NO_NETWORK_CONNECTION:
      ret = "NO_NETWORK_CONNECTION";
      break;
    default:
      ret = "unknown exception";
      break;
  }
  return ret;
}

const struct cacheElement &cacheElement::operator=( const struct cacheElement &e ) {
    type = e.type;
    name = e.name;
    size = e.size;
    world_readable = e.world_readable;
    writeable = e.writeable;
    last_updated = e.last_updated;
    localChanges = e.localChanges;
    xmlRepresentation = e.xmlRepresentation;
    cachedVersion = e.cachedVersion;
    picasaObj = e.picasaObj;
    finalized = e.finalized;
    numOfOpenWr = e.numOfOpenWr;
    switch (e.type) { 
	    case cacheElement::DIRECTORY:
		    authKey = e.authKey;
		    contents = e.contents;
		    return e;
	    case cacheElement::FILE:
	            authKey   = "";
		    generated = e.generated;
		    cachePath = e.cachePath;
		    return e;
    }
    return e;
}

ostream &operator<<(ostream &out, const cacheElement &e ) { 
  out << "Name: " << e.name <<endl;
  out << "Size: " << e.size << endl;
  out << "Last Updated: " << e.last_updated << endl;
  out << "Cached Version: " << e.cachedVersion << endl;
  out << "Num of open fd: " << e.numOfOpenWr << endl;
  switch( e.type ) { 
    case cacheElement::DIRECTORY:
      out << "Type: Directory " << endl;
      out << "AuthKey: " << e.authKey << endl;
      out << "Contents Size: " << e.contents.size() << endl;
      break;
    case cacheElement::FILE:
      out << "Type: File " << endl;
      out << "CachePath: " << e.cachePath << endl;
      break;
  }
  out << " ---------- XML ------------ " << endl;
  out << e.xmlRepresentation << endl;
  out << " --------------------------- " << endl;
  return out;
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
//  contents.clear();
  localChanges = false;
  xmlRepresentation = album->getStringXML();
  // if ( picasaObj != album )  delete picasaObj;
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
  // if ( picasaObj != photo ) delete picasaObj;
  picasaObj = photo;
  generated = false;
  finalized = true;
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

const pathParser picasaCache::controlDirPath(".control");
const pathParser picasaCache::syncPath(".control/sync");
const pathParser picasaCache::offlinePath(".control/offline");
const pathParser picasaCache::onlinePath(".control/online");
const pathParser picasaCache::logPath(".control/log");
const pathParser picasaCache::statsPath( ".control/stats" );
const pathParser picasaCache::authKeysPath( ".control/auth_keys" );
const pathParser picasaCache::updateQueuePath(".control/update_queue");
const pathParser picasaCache::priorityQueuePath(".control/priority_queue");
const pathParser picasaCache::localChangesQueuePath(".control/local_changes_queue");

picasaCache::picasaCache( const string& user, const string& pass, const string& ch, int UpdateInterval, long maxPixels ):
	api( new gAPI( user, pass, "picasaFUSE" ) ),
	picasa( new picasaService( user, pass ) ),
	work_to_do(false), kill_thread(false), cacheDir( ch ), updateInterval(UpdateInterval),
	numOfPixels( maxPixels ), maxJobThreads( 10 ), haveNetworkConnection(true)
{
  if ( updateInterval < 1 ) updateInterval = 600;
  mkdir( cacheDir.c_str(), 0755 );
  string cacheFName = cacheDir + "/.cache", err;
  if ( boost::filesystem::exists( cacheFName ) ) { 
    try {
      std::ifstream ifs(cacheFName.c_str(), ios::binary );
      boost::archive::text_iarchive ia(ifs);
      ia >> cache;
      insertControlDir();
      return;
    } catch ( boost::archive::archive_exception ex ) { 
      std::cerr << ex.what() << "\n";
      err = ex.what();
      cache.clear();
    }
  }
  createRootDir();
  insertControlDir();
  if ( err != "" ) log( "Error reading cache from disk ("+err+")" );
  log( "----------NEW CACHE CREATED\n" );
}

bool picasaCache::goOnline() {
  haveNetworkConnection = true;
}

string picasaCache::toString() { 
  std::stringstream ss;
  boost::archive::text_oarchive oa( ss );
  boost::mutex::scoped_lock l(cache_mutex);
  oa << cache;
  return ss.str();
}

void picasaCache::createRootDir() { 
  pathParser root("");
  if ( isCached( root ) ) return;
  struct cacheElement e;
  e.last_updated = time( NULL );
  e.type = cacheElement::DIRECTORY;
  e.name="";
  e.size=sizeof(char)*1024;
  e.world_readable=true;
  e.writeable = false;
  e.authKey = "";
  putIntoCache( root, e );
}

bool picasaCache::insertDir( const pathParser &p, const string &authKey, bool writeable ) {
  boost::mutex::scoped_lock l(cache_mutex);
  string parentKey = p.chop().getHash();
  if ( cache.find( parentKey ) == cache.end() ) return false; // Parent does not exist.
  cache[parentKey].contents.insert( p.getLastComponent() );
  struct cacheElement e;
  e.last_updated = time( NULL );
  e.type = cacheElement::DIRECTORY;
  e.name=p.getLastComponent();
  e.size=sizeof(char)*1024;
  e.authKey = authKey;
  e.world_readable = ( authKey == "" );
  e.writeable = writeable;
  cache[p.getHash()] = e;
  return true;
}

bool picasaCache::insertSpecialFile( const pathParser &p, bool world_readable, bool world_writeable ) {
  boost::mutex::scoped_lock l(cache_mutex);
  string parentKey = p.chop().getHash();
  if ( cache.find( parentKey ) == cache.end() ) return false; // Parent does not exist.
  cache[parentKey].contents.insert( p.getLastComponent() );
  struct cacheElement e;
  e.last_updated = time( NULL );
  e.type = cacheElement::FILE;
  e.name=p.getLastComponent();
  e.size=0;
  e.generated=true;
  e.world_readable = world_readable;
  e.writeable = world_writeable;
  e.authKey   = "";
  cache[p.getHash()] = e;
  return true;
}

void picasaCache::insertControlDir() {
  if ( ! isCached( controlDirPath ) ) insertDir( controlDirPath );
  if ( ! isCached( logPath ) ) insertSpecialFile( logPath );
  insertSpecialFile( priorityQueuePath );
  insertSpecialFile( updateQueuePath );
  insertSpecialFile( localChangesQueuePath );
  insertSpecialFile( statsPath );
  insertSpecialFile( authKeysPath );
}

void picasaCache::updateUQueueFile() {
  struct cacheElement e;
  if ( ! getFromCache( updateQueuePath, e ) ) return;
  stringstream os;
  boost::mutex::scoped_lock l(update_queue_mutex);
  for( std::list<pathParser>::const_iterator it = update_queue.begin(); it != update_queue.end(); ++it ) {
    os << it->getFullName() << endl;
  }
  e.cachePath=os.str();
  e.size = e.cachePath.size();
  putIntoCache( updateQueuePath, e );
}

void picasaCache::updatePQueueFile() {
  struct cacheElement e;
  if ( ! getFromCache( priorityQueuePath, e ) ) return;
  stringstream os;
  boost::mutex::scoped_lock l(priority_update_queue_mutex);
  for( std::list<pathParser>::const_iterator it = priority_update_queue.begin(); it != priority_update_queue.end(); ++it ) {
    os << it->getFullName() << endl;
  }
  e.cachePath=os.str();
  e.size = e.cachePath.size();
  putIntoCache( priorityQueuePath, e );
}

void picasaCache::updateLCQueueFile() {
  struct cacheElement e;
  if ( ! getFromCache( localChangesQueuePath, e ) ) return;
  stringstream os;
  boost::mutex::scoped_lock l(local_change_queue_mutex);
  for( std::list<pathParser>::const_iterator it = local_change_queue.begin(); it != local_change_queue.end(); ++it ) {
    os << it->getFullName() << endl;
  }
  e.cachePath=os.str();
  e.size = e.cachePath.size();
  putIntoCache( localChangesQueuePath, e );
}

void picasaCache::updateStatsFile() {
  struct cacheElement e;
  if ( ! getFromCache( statsPath, e ) ) return;
  stringstream os;
  boost::mutex::scoped_lock lu(update_queue_mutex), lp(priority_update_queue_mutex),lc(local_change_queue_mutex);
  os << "Update Queue size:" << update_queue.size() << std::endl; lu.unlock();
  os << "Priority Queue size:" << priority_update_queue.size() << std::endl; lp.unlock();
  os << "Local Changes Queue size:" << local_change_queue.size() << std::endl; lc.unlock();
  os << "Network connection:";
  if ( haveNetworkConnection ) os <<"online";
  else os << "offline";
  os<<std::endl;
  e.cachePath=os.str();
  e.size = e.cachePath.size();
  putIntoCache( statsPath, e );
}

void picasaCache::updateAuthKeys() {
  stringstream os;
  boost::mutex::scoped_lock l(cache_mutex);
  for( map<string,cacheElement>::iterator it = cache.begin(); it != cache.end(); ++it ) {
    if ( it->second.authKey != "" ) {
      os << it->first <<" = " << it->second.authKey << std::endl;
    }
  }
  cacheElement e = cache[authKeysPath.getHash()];
  e.cachePath = os.str();
  e.size = e.cachePath.size();
  cache[authKeysPath.getHash()] = e;
}

void picasaCache::updateSpecial(const pathParser p) {
  if ( p == statsPath ) updateStatsFile();
  else if ( p == localChangesQueuePath ) updateLCQueueFile();
  else if ( p == updateQueuePath ) updateUQueueFile();
  else if ( p == priorityQueuePath ) updatePQueueFile();
  else if ( p == authKeysPath ) updateAuthKeys();
}


bool picasaCache::isSpecial( const pathParser &p ) { 
  if ( p == controlDirPath ) return true;
  if ( p == logPath ) return true;
  struct cacheElement e;
  if ( getFromCache( p, e ) && e.generated ) return true;
  if ( p.getUser() == "logs" ) return true;
  return false;
}

picasaCache::~picasaCache() {
  kill_thread = true;
  saveCacheToDisk();
  update_thread->interrupt();
  priority_update_thread->interrupt();
  update_thread->join();
  priority_update_thread->join();
}

void picasaCache::saveCacheToDisk() { 
  string cacheFName = cacheDir + "/.cache";
  log( "---------CACHE SAVED TO DISK\n" );
  std::ofstream ofs( cacheFName.c_str(), ios::binary );
  {
    boost::archive::text_oarchive oa(ofs);
    boost::mutex::scoped_lock l(cache_mutex);
    oa << cache;
  }
  last_saved = time( NULL );
}

void picasaCache::log( string msg ) { 
  time_t now = time( NULL );
  stringstream os;
  os << now << ": " << msg;
  struct cacheElement c;
  getFromCache( logPath, c );
  c.cachePath+=os.str();
  c.size = c.cachePath.size();
  putIntoCache( logPath, c );
}

void picasaCache::pleaseUpdate( const pathParser p, bool priority ) { 
//  log("Scheduling update of: " + p.getFullName() + "\n");
  if ( ! p.isValid() ) return;
  if ( ! p.haveUser() ) return;
  time_t now = time( NULL );
  cacheElement c;
  if ( (! isSpecial(p)) && getFromCache( p, c ) && (now - c.last_updated < updateInterval) && (! priority ) ) return;
  if ( priority ) {
    if ( ! priority_update_thread ) priority_update_thread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&picasaCache::priority_worker, this)));
    boost::mutex::scoped_lock pl(priority_update_queue_mutex);
    for(std::list<pathParser>::iterator it = priority_update_queue.begin(); it != priority_update_queue.end(); ++it ) {
      if ( *it == p ) return;
    }
    priority_update_queue.push_back(p);
    pl.unlock();
    if ( haveNetworkConnection ) priority_update_thread->interrupt();
  } else if ( haveNetworkConnection ) {
    if ( ! update_thread ) update_thread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&picasaCache::update_worker, this )));
    boost::mutex::scoped_lock ul(update_queue_mutex);
    update_queue.push_back(p);
    ul.unlock();
    update_thread->interrupt();
  }
}

void picasaCache::localChange( const pathParser p ) { 
  boost::mutex::scoped_lock l(local_change_queue_mutex);
  local_change_queue.push_back(p);
  l.unlock();
  if ( haveNetworkConnection ) update_thread->interrupt();
}


void picasaCache::newAlbum( const pathParser A ) throw ( enum picasaCache::exceptionType ) {
  picasaAlbum album = picasa->newAlbum( A.getAlbum() );
  cacheElement c;
  if ( ! getFromCache( A.chop(), c ) ) { 
    updateUser( A.chop() );
  }
  if ( ! getFromCache( A.chop(), c ) ) throw UNEXPECTED_ERROR;
  c.fromAlbum( new picasaAlbum( album ) );
  c.contents.clear();
  c.localChanges = true;
  c.last_updated = 0;
  putIntoCache( A, c );
  getFromCache( A.chop(), c );
  c.contents.insert( album.getTitle() );
  putIntoCache( A.chop(), c );
  if ( haveNetworkConnection ) {
    try {
      doUpdate( A );
    } catch (...) {
      localChange(A);
    }
  } else localChange(A);
}

void picasaCache::pushAlbum( const pathParser A ) throw ( enum picasaCache::exceptionType ) {
  cacheElement c;
  if ( ! getFromCache( A, c ) ) throw OBJECT_DOES_NOT_EXIST;
  if ( ! c.localChanges ) return;
  
  if ( ! c.picasaObj ) c.buildPicasaObj(picasa);
  if ( ! c.picasaObj ) {
    log( "updateAlbum( " +A.getFullName()+" ): picasaAlbum could not be reconstructed\n" );
    log( "    offending xml:" + c.xmlRepresentation + "\n" );
    removeFromCache( A );
    throw UNEXPECTED_ERROR;
  }
  
  picasaAlbum *album = dynamic_cast<picasaAlbum *>( c.picasaObj );
  if ( ! haveNetworkConnection ) throw NO_NETWORK_CONNECTION;
  if ( ! album->PUSH_CHANGES() ) throw OPERATION_FAILED;
  c.fromAlbum( album );
  c.localChanges = false;
  putIntoCache( A, c );
}
  

/*
 * Assumes A is already in the cache (or unlisted), otherwise throws
 */
void picasaCache::updateAlbum( const pathParser A ) throw ( enum picasaCache::exceptionType ) {
  cacheElement c, pElement;
  string pName;
  log( "picasaCache::updateAlbum("+A.getFullName()+"):\n" );

  if ( ! getFromCache( A, c ) ) {
    int authKeyPos = A.getAlbum().find( "?authkey=" );
    if ( authKeyPos != string::npos ) { // Possibly an unlisted album, need not be in the cache
      string authKey = A.getAlbum().substr( authKeyPos+9 ),
             albumName = A.getAlbum().substr( 0, authKeyPos );
      if ( ! haveNetworkConnection ) throw NO_NETWORK_CONNECTION;
      picasaAlbum album = picasa->getAlbumByName( albumName, A.getUser(), authKey );
      c.fromAlbum( new picasaAlbum( album ) );
      pathParser B = A.chop() + album.getTitle();
      putIntoCache( B, c );
      getFromCache( A.chop(), c );
      c.contents.insert( album.getTitle() );
      if ( album.getAccessType() != picasaAlbum::UNLISTED ) {
	log( " WTF???: album " + album.getTitle() + " is not unlisted\n" );
	throw UNEXPECTED_ERROR;
      }
      putIntoCache( A.chop(), c );
      updateAlbum( B );
      return;
    }
    // A listed album must be present in the cache to be updated !!!
    //log( "  ERROR: Object not present in cache, throwing ... \n" );
    throw OBJECT_DOES_NOT_EXIST;
  }
  
  if ( c.localChanges ) {
    pushAlbum( A );
    return;
  }
  
  if ( ! c.picasaObj ) c.buildPicasaObj(picasa);
  if ( ! c.picasaObj ) {
    log( "updateAlbum( " +A.getFullName()+" ): picasaAlbum could not be reconstructed\n" );
    log( "    offending xml:" + c.xmlRepresentation + "\n" );
    removeFromCache( A );
    throw UNEXPECTED_ERROR;
  }
  
  picasaAlbum *album = dynamic_cast<picasaAlbum *>(c.picasaObj);
  if ( ! haveNetworkConnection ) throw NO_NETWORK_CONNECTION;
  if ( ! album->PULL_CHANGES() ) {
    log( " ERROR: Error updating album info, throwing ... \n" );
    removeFromCache( A );
    throw OBJECT_DOES_NOT_EXIST;
  }
  c.fromAlbum( album );
  
  // If albumName changed, update the parent accordingly
  if ( A.getAlbum() != c.name ) { 
    cacheElement u;
    getFromCache( A.chop(), u );
    u.contents.erase( A.getAlbum() );
    u.contents.insert( c.name );
    putIntoCache( A.chop(), u );
  }
  
  list<picasaPhoto *> photos = album->getPhotos();
  
  set<string> photoTitles;
  for( list<picasaPhoto *>::iterator p = photos.begin(); p != photos.end(); ++p )
    photoTitles.insert( (*p)->getTitle() );

  // Remove photos present in the album but not on picasa
  // which have no local changes
  set<string> toDelete;
  // find candidates first
  for( set<string>::iterator p = c.contents.begin(); p != c.contents.end(); ++p ) { 
    if ( photoTitles.find( *p ) == photoTitles.end() ) {
      getFromCache( A + *p, pElement );
      if ( ! pElement.localChanges ) toDelete.insert( *p );
    }
  }
  // then remove them
  for( set<string>::iterator p = toDelete.begin(); p != toDelete.end(); ++p ) {
  	removeFromCache( A + *p );
	c.contents.erase( *p );
  }

  // Add new photos and update old ones
  for( list<picasaPhoto *>::iterator p = photos.begin(); p != photos.end(); ++p ) {   
    pName = (*p)->getTitle();
    if ( c.contents.find( pName ) == c.contents.end() ) { // A new photo
      pElement.fromPhoto( *p );
      c.contents.insert( pName );
      pElement.cachePath = A.getFullName()+"/"+pName;
      putIntoCache( A + pName, pElement );
    } else { // photo already in cache
      getFromCache( A + pName, pElement );
      if ( ! pElement.localChanges ) {
	pElement.fromPhoto( *p );
	putIntoCache( A + pName, pElement );
      }
    }
    pleaseUpdate( A + pName );
  }
  c.last_updated = time( NULL );
  putIntoCache( A, c );
}


void picasaCache::pushImage( const pathParser P ) throw ( enum picasaCache::exceptionType ) { 
  cacheElement c;
  log( "picasaCache::pushImage(" + P.getFullName() + "):\n" );

  if ( ! getFromCache( P, c ) ) {
    log( "  ERROR: Object not present in cache, throwing ... \n" );
    throw OBJECT_DOES_NOT_EXIST;
  }
  
  if ( c.localChanges ) {
    log( "picasaCache::pushImage("+P.getFullName()+"): local changes\n" );
    if ( ! c.finalized ) {
      log( "Photo not finalized yet ... \n");
      return;
    }
    c.buildPicasaObj( picasa );
    picasaPhoto *photo = dynamic_cast<picasaPhoto *>(c.picasaObj);
    convert magic;
    string summary = "";
    if ( photo ) {
      if ( numOfPixels > 0 ) {
	magic.resize( numOfPixels, cacheDir + "/" + c.cachePath );
      }
      summary = magic.getComment( cacheDir + "/" + c.cachePath );
      if ( ! haveNetworkConnection ) throw NO_NETWORK_CONNECTION;
      if ( ! photo->upload( cacheDir + "/" + c.cachePath ) ) {
	log( "Failed uploading ...\n" );
	throw OPERATION_FAILED;
      }
      c.localChanges = false;
      log( "Photo uploaded...\n" );
      if ( photo->getSummary() != summary && summary != "") { 
	photo->setSummary(summary);
	if ( ! photo->UPDATE() ) c.localChanges = true;
	log( "Photo updated...\n" );
      }
    } else {
      cacheElement a;
      getFromCache( P.chop(), a );
      a.buildPicasaObj( picasa );
      picasaAlbum *album = dynamic_cast<picasaAlbum *>( a.picasaObj );
      if ( ! album ) {
	log( "Parent not an album !!! WTF???\n" );
	throw OPERATION_FAILED;
      }
      if ( numOfPixels > 0 ) {
	magic.resize( numOfPixels, cacheDir + "/" + c.cachePath );
      }
      summary = magic.getComment( cacheDir + "/" + c.cachePath );
      if ( ! haveNetworkConnection ) throw NO_NETWORK_CONNECTION;
      try {
	photo = album->addPhoto( cacheDir + "/" + c.cachePath, summary );
	log( "Photo uploaded...\n" );
	c.fromPhoto( photo );
	c.last_updated = time( NULL );
	pleaseUpdate( P.chop() );
      } catch ( atomObj::exceptionType &ex ) { 
	log( "Failed posting new photo...\n" );
	throw OPERATION_FAILED;
      }
    }
    putIntoCache( P, c );
    return;
  }
}

/*
 * Assumes P is already in the cache, otherwise throws
 */
void picasaCache::updateImage( const pathParser P ) throw ( enum picasaCache::exceptionType ) { 
  cacheElement c;

  log( "picasaCache::updateImage("+P.getFullName()+"):\n" );
  
  if ( ! getFromCache( P, c ) ) {
    log( "  ERROR: Object not present in cache, throwing ... \n" );
    throw OBJECT_DOES_NOT_EXIST;
  }
  
  if ( c.localChanges ) {
    localChange( P );
    return;
  }

  log( " No local changes...\n" );
  c.buildPicasaObj(picasa);
  picasaPhoto *photo = dynamic_cast<picasaPhoto *>(c.picasaObj);
  if ( ! photo ) {
    stringstream ss;
    ss << c;
    log("Could not reconstruct photo !!!\n");
    log( ss.str() );
    cerr << ss.str();
    throw UNEXPECTED_ERROR;
  }

  if ( ! haveNetworkConnection ) throw NO_NETWORK_CONNECTION;
  
  if ( ! photo->PULL_CHANGES() ) {
    log( " ERROR: Error updating photo info, throwing ... \n" );
    cacheElement a;
    getFromCache( P.chop(), a );
    a.contents.erase( photo->getTitle() );
    putIntoCache( P.chop(), a );
    removeFromCache( P );
    throw OBJECT_DOES_NOT_EXIST;
  }

  if ( c.cachedVersion != photo->getVersion() ) {
    log( " Downloading " + photo->getPhotoURL() + " to " + c.cachePath + "\n" );
    photo->download(cacheDir+"/"+c.cachePath); // FIXME: make atomic.
    c.cachedVersion = photo->getVersion();
    log( " Download OK \n" );
  }
  c.fromPhoto( photo );
  
 // If photoName changed, update the parent accordingly
  if ( P.getImage() != c.name ) { 
    cacheElement a;
    getFromCache( P.chop(), a );
    a.contents.erase( P.getImage() );
    a.contents.insert( c.name );
    putIntoCache( P.chop(), a );
  }
  c.last_updated = time( NULL );
  putIntoCache( P, c );
}

void picasaCache::updateUser ( const pathParser U ) throw ( enum picasaCache::exceptionType ) { 
  set<picasaAlbum> albums;
  set<string> albumDirNames;
  log( "picasaCache::updateUser("+U.getUser()+"):\n" );
  if ( ! haveNetworkConnection ) throw NO_NETWORK_CONNECTION;
  
  try { 
    albums = picasa->albumList( U.getUser() );
    //log( "  albumList ...OK\n" );
  } catch ( enum picasaService::exceptionType ex ) { 
    log( "  User not found (ERROR).\n" );
    // We remove the user from the cache, if it was present.
    // (we assume it has no local changes, since we
    //  do not allow creation of users)
    removeFromCache( U );
    throw OBJECT_DOES_NOT_EXIST;
  }


  struct cacheElement c,d;
  pathParser p;
  
  /*
   * Add user to root directory
   */
  
  p.parse( "/" );
  getFromCache( p, c );
  c.contents.insert( U.getUser() );
  putIntoCache( p, c );

  c.writeable = ( U.getUser() == picasa->getUser() );
  c.type = cacheElement::DIRECTORY;


  // Construct a list of the public albums;
  for( set<picasaAlbum>::iterator a = albums.begin(); a != albums.end(); ++a )
    albumDirNames.insert( a->getTitle() );

 
  /*
   * Add user directory to cache
   */
  p.parse("/"+U.getUser());

  if ( getFromCache( p, c ) ) { // If already present, we need to update it
    // Add already present unlisted albums to albumDirNames
    //FIXME: How do we know when an unlisted album has been deleted ?
    for( set<string>::iterator a = c.contents.begin(); a != c.contents.end(); ++a ) { 
      if ( getFromCache( p + *a, d ) ) {
	if ( ! d.picasaObj ) d.buildPicasaObj( picasa );
	if ( d.picasaObj ) { 
	  if ( dynamic_cast<picasaAlbum*>(d.picasaObj)->getAccessType() == picasaAlbum::UNLISTED ) {	    
	    albumDirNames.insert( *a );
	  } else {
	    log( " ignoring public album " + *a + "\n" );
	  }
	} else {
	  log( " cache element " + (p + *a).getFullName()+ " does not have object\n" );
	}
      }
    }
    // Remove albums present in the cache but not on picasa
    // provided there are no local changes
    for( set<string>::iterator a = c.contents.begin(); a != c.contents.end(); ++a ) {
      if ( albumDirNames.find( *a ) == albumDirNames.end() && getFromCache( p + *a, d) ) {
	if ( ! d.localChanges ) removeFromCache( p + *a );
	else albumDirNames.insert( *a );
      }
    }
    c.contents = albumDirNames;
  } else { // User not yet present in the cache
    c.contents = albumDirNames;
  }
  
  c.world_readable=true;
  c.last_updated = time( NULL );
  putIntoCache( p, c );
  log( "  update user dir ...OK\n" );
  c.contents.clear();
  time_t lu;
  for( set<picasaAlbum>::iterator a = albums.begin(); a != albums.end(); ++a ) {
    if ( getFromCache( U + a->getTitle(), c) ) lu = c.last_updated;
    else lu = 0;
    c.fromAlbum( new picasaAlbum( *a ) );
    c.last_updated = lu;
    putIntoCache( U + a->getTitle(), c);
  }
}

void picasaCache::pushChange( const pathParser p ) throw ( enum picasaCache::exceptionType ) { 
  struct cacheElement c;
  if ( ! getFromCache( p, c ) ) throw OBJECT_DOES_NOT_EXIST;
  if ( ! c.localChanges ) return;
  if ( ! haveNetworkConnection ) throw NO_NETWORK_CONNECTION;
  if ( p.getType() == pathParser::IMAGE ) pushImage( p );
  else doUpdate( p );
}

void picasaCache::doUpdate( const pathParser p ) throw ( enum picasaCache::exceptionType ) {
  struct cacheElement c;
  pathParser np;
  time_t now = time( NULL );
  if ( ! p.isValid() ) return;
  if ( ! p.haveUser() ) return;
  if ( isSpecial(p) ) {
    updateSpecial(p);
    return;
  }

  if ( ! haveNetworkConnection ) throw NO_NETWORK_CONNECTION;
  
  /* Object is already present in the cache */
  if ( getFromCache( p, c ) ) {
    
    /*long cacheValidity = updateInterval;
    switch( p.getType() ) { 
      case pathParser::USER:
	cacheValidity = 600;
      case pathParser::ALBUM:
	cacheValidity = 600;
      case pathParser::IMAGE:
	cacheValidity = 600;
    }*/
    // If the cached version is recent, do nothing
    if ( now - c.last_updated < updateInterval ) {
	stringstream estream;
	estream << "Not updating " << p.getFullName() << " since " << now << " - " << c.last_updated << " < " << updateInterval << " \n";
	log(estream.str());
	return;
    };
    
    stringstream estream;
    //estream << "doUpdate: p.getType() == " << p.getType() << "\n";
    //log(estream.str());
   
    switch( p.getType() ) { 
      case pathParser::IMAGE:
	updateImage( p );
	return;
      case pathParser::ALBUM:
	updateAlbum( p );
	return;
      case pathParser::USER:
	updateUser( p.getUser() );
	return;
    }
  } else /* Object not cached yet at all */ {
    if ( p.haveImage() ) { 
      doUpdate( p.chop() );
      updateImage( p );
    }
    if ( p.haveAlbum() ) { 
      doUpdate( p.chop() );
      updateAlbum( p );
    }
    updateUser( p );
  }
}

void picasaCache::priority_worker() {
  boost::mutex::scoped_lock l(priority_update_queue_mutex);
  l.unlock();
  pathParser p;
  bool wtodo;
  while( ! kill_thread ) {
    l.lock();
    if ( (! priority_update_queue.empty() ) && haveNetworkConnection ) {
      p = priority_update_queue.front();
      l.unlock();
      try {
	//log( "update_worker: Processing scheduled job (" + p.getFullName() + ")\n" );
	doUpdate( p );
	l.lock();
	priority_update_queue.pop_front();
	l.unlock();
      } catch (enum picasaCache::exceptionType ex ) {
	log( "update_worker: Exception ("+exceptionString( ex ) + ") caught while doing update of "+p.getFullName() + "\n");
      }
    } else {
      l.unlock();
      boost::xtime t;
      boost::xtime_get(&t, boost::TIME_UTC);
      t.sec+=10;
      try {
	boost::this_thread::sleep(t);
      } catch ( ... ) {
      }
    }
  }
}

void picasaCache::update_worker() {
  boost::mutex::scoped_lock l(update_queue_mutex), lc(local_change_queue_mutex);
  l.unlock(); lc.unlock();
  pathParser p;
  time_t now;
  bool wtodo;
  while( ! kill_thread ) {
    l.lock();
    if ( ! update_queue.empty() ) {
      p = update_queue.front();
      update_queue.remove(p);
      l.unlock();
      try {
	//log( "update_worker: Processing scheduled job (" + p.getFullName() + ")\n" );
	doUpdate( p );
      } catch (enum picasaCache::exceptionType ex ) {
	log( "update_worker: Exception ("+exceptionString( ex ) + ") caught while doing update of "+p.getFullName() + "\n");
      }
    } else l.unlock();
    lc.lock();
    if ( ! local_change_queue.empty() ) {
      p = local_change_queue.front();
      lc.unlock();
      try {
	pushChange( p );
	lc.lock();
	local_change_queue.remove(p);
	lc.unlock();
      } catch ( enum picasaCache::exceptionType ex ) { 
	log( "update_worker: Exception ("+exceptionString( ex ) + ") caught while doing update of "+p.getFullName() + "\n");
	switch( ex ) {
	  case OPERATION_FAILED:
	    localChange( p );
	}
      }
    } else lc.unlock();
    l.lock();
    wtodo = ( ! ( update_queue.empty() ) );
    l.unlock();
    lc.lock();
    wtodo = ( wtodo || ( ! local_change_queue.empty() ) );
    lc.unlock();
    if ( ! wtodo ) {
      boost::xtime t;
      boost::xtime_get(&t, boost::TIME_UTC);
      t.sec += 10;
      try {
	boost::this_thread::sleep(t);
      } catch (...) {
      }
    }
    now = time( NULL );
    if ( now - last_saved > 180 ) saveCacheToDisk();
  }
}

void picasaCache::sync() {
  list<pathParser> failed_list;
  boost::mutex::scoped_lock lc(local_change_queue_mutex);
  pathParser p;
  while( local_change_queue.size() > 0 ) { 
    p = local_change_queue.front();
    local_change_queue.pop_front();
    try {
      pushChange( p );
    } catch ( enum picasaCache::exceptionType ex ) { 
      log( "fsync: Exception ("+exceptionString( ex ) + ") caught while doing update of "+p.getFullName() + "\n");
      switch( ex ) { 
	case OPERATION_FAILED:
	  failed_list.push_back( p );
      }
    }
  }
  local_change_queue = failed_list;
}

bool picasaCache::getFromCache( const pathParser &p, struct cacheElement &e ) {
  if ( ! p.isValid() ) return false;
  boost::mutex::scoped_lock l(cache_mutex);
  string key = p.getHash();
  if ( cache.find( key ) == cache.end() ) {
    return false;
  }
  e = cache[key];
  return true;
}

void picasaCache::putIntoCache( const pathParser &p, const struct cacheElement &e ) {
  if ( ! isSpecial( p ) ) cacheMkdir( cacheDir, p );
  boost::mutex::scoped_lock l(cache_mutex);
  cache[p.getHash()] = e;
}

bool picasaCache::isCached( const pathParser &p ) {
  if ( ! p.isValid() ) return false;
  return isCached( p.getHash() );
}

void picasaCache::clearCache() { 
  boost::mutex::scoped_lock l(cache_mutex);
  cache.clear();
}

/*list<pathParser> picasaCache::getChildrenInCache( const pathParser &p ) {
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
}*/

void picasaCache::no_lock_removeFromCache( const pathParser &p ) { 
  if ( p.isRoot() ) return;
  string key = p.getHash();
  if ( cache.find( key ) == cache.end() ) return;
  struct cacheElement c = cache[key];
  switch( c.type ) { 
    case cacheElement::FILE:
      if ( c.generated != true ) {
	try {
	  boost::filesystem::remove( cacheDir+"/"+c.cachePath );
	} catch (...) {}
      }
      break;
    case cacheElement::DIRECTORY:
      for( set<string>::iterator child = c.contents.begin(); child != c.contents.end(); ++child ) no_lock_removeFromCache( p+*child );
      try {
	boost::filesystem::remove( cacheDir+"/"+c.cachePath );
      } catch (...) {}
      break;
  }
  update_queue.remove( p );
  cache.erase( key );
}

void picasaCache::removeFromCache( const pathParser &p ) { 
  boost::mutex::scoped_lock cl(cache_mutex);
  boost::mutex::scoped_lock ql(update_queue_mutex);
  no_lock_removeFromCache( p );
  string parentKey = p.chop().getHash();
  if ( cache.find( parentKey ) == cache.end() ) return;
  cache[parentKey].contents.erase( p.getLastComponent() );
}
/*
void picasaCache::removeFromCache( const pathParser &p ) { 
  if ( p.isRoot() ) return;
  boost::mutex::scoped_lock cl(cache_mutex);
  boost::mutex::scoped_lock ql(update_queue_mutex);
  no_lock_removeFromCache( p );
  return;
  list<pathParser> children = getChildrenInCache( p );
  for( list<pathParser>::iterator child = children.begin(); child != children.end(); ++child ) { 
    update_queue.remove( *child );
    cache.erase( child->getHash() );
  }
  ql.unlock();
  pathParser parent = p.chop();
  string me = p.getLastComponent();
  struct cacheElement c = cache[parent.getHash()];
  c.contents.erase(me);
  cache[parent.getHash()] = c;
  cache.erase( p.getHash() );
  cl.unlock();
  log( "rm -rf " + cacheDir + "/" + p.getFullName() + "\n" );
  if ( c.type == cacheElement::FILE ) { 
    try {
      boost::filesystem::remove( cacheDir+"/"+c.cachePath );
    } catch (...) {
    }
  } else {
      boost::filesystem::remove_all( cacheDir + "/" + p.getFullName() );
  }
}*/




bool picasaCache::isCached( const string &key ) {
  boost::mutex::scoped_lock l(cache_mutex);
  return (cache.find(key) != cache.end() );
}


bool picasaCache::isDir( const pathParser &path ) {
  struct cacheElement e;
  if ( getFromCache( path, e) ) { 
    return ( e.type == cacheElement::DIRECTORY );
  } else return false;
}

string picasaCache::getXAttr( const pathParser &p, const string &attrName ) throw (enum exceptionType) {
  cacheElement c;
  log( "picasaCache::getXAttr( " + p.getFullName() + ", " + attrName + " )\n" );
  cerr << " picasaCache::getXAttr( " + p.getFullName() + ", " + attrName + " )\n";
  if ( ! getFromCache( p, c ) ) throw OBJECT_DOES_NOT_EXIST;
  if ( attrName == "CacheElement" ) {
    stringstream ss;
    ss << c;
    return ss.str();
  }
  if ( isSpecial( p ) ) throw OBJECT_DOES_NOT_EXIST;
  c.buildPicasaObj( picasa );
  if ( ! c.picasaObj ) throw UNEXPECTED_ERROR;
  if ( attrName == "AuthKey" && p.getType() == pathParser::ALBUM ) {
    return dynamic_cast<picasaAlbum *>(c.picasaObj)->getAuthKey();
  }
  string ret; 
  try {
    ret = c.picasaObj->getAttr( attrName );
  } catch ( atomObj::exceptionType ) {
    throw OBJECT_DOES_NOT_EXIST;
  }
  return ret;
}

list<string> picasaCache::listXAttr( const pathParser &p ) throw (enum exceptionType) { 
  cacheElement c;
  if ( ! getFromCache( p, c ) ) throw OBJECT_DOES_NOT_EXIST;
  list<string> ret;
  ret.push_back( "CacheElement" );
  if ( isSpecial( p ) ) return ret;
  c.buildPicasaObj( picasa );
  if ( ! c.picasaObj ) throw UNEXPECTED_ERROR;
  try {
    ret = c.picasaObj->listAttr();
    ret.push_back( "AuthKey" );
  } catch ( atomObj::exceptionType ) {
    throw OBJECT_DOES_NOT_EXIST;
  }
  return ret;
}

int picasaCache::getAttr( const pathParser &path, struct stat *stBuf ) {
  std::cerr << "picasaCache::getAttr(" << path.getFullName() <<"): start..."<< endl;
  struct cacheElement e;
  if ( ! getFromCache( path, e ) ) {
    std::cerr << "picasaCache::getAttr(" << path.getFullName() <<"): not cached..."<< endl;
    // If we are not looking at a subdirectory of the root/user
    // and the path is not cached it doesn't exist 
    // (at least not now, maybe at some later point, 
    // when we update the parent directory)
    switch ( path.getType() ) { 
      case pathParser::USER:
	try {
	  doUpdate( path );
	} catch ( enum exceptionType ex ) {
	  return -ENOENT;
	}
	if ( ! getFromCache( path, e ) ) return -ENOENT;
	break;
      case pathParser::ALBUM:
	pleaseUpdate( path, true );
	return -ENOENT;
	break;
      default:
	return -ENOENT;
    }
  }
  pleaseUpdate( path );
  std::cerr << "picasaCache::getAttr(" << path.getFullName() <<"): size..."<< e.size<<endl;
  if (e.generated) std::cerr << "picasaCache::getAttr(" << path.getFullName() <<"): generated("<< e.cachePath.size()<< ")"<<endl;
  stBuf->st_size = e.size;
  switch ( e.type ) { 
	  case cacheElement::DIRECTORY:
		  stBuf->st_mode = S_IFDIR | S_IRUSR | S_IXUSR;
		  if ( e.world_readable ) stBuf->st_mode |= S_IXGRP | S_IRGRP | S_IROTH | S_IXOTH;
		  if ( e.writeable ) stBuf->st_mode |= S_IWUSR;
		  stBuf->st_nlink = 2;
		  return 0;
	  case cacheElement::FILE:
		  stBuf->st_mode = S_IFREG | S_IRUSR;
		  if ( e.world_readable ) stBuf->st_mode |= S_IRGRP | S_IROTH;
		  if ( e.writeable ) stBuf->st_mode |= S_IWUSR;
		  stBuf->st_nlink = 1;
		  if ( ! e.generated ) { 
		    string fp = cacheDir + "/" + e.cachePath;
		    struct stat myStat;
		    if ( stat( fp.c_str(), &myStat ) == 0 ) { 
		      stBuf->st_size = myStat.st_size;
		    }
		  }
		  return 0;
  }
  std::cerr << "Very weird !!!!!!!!!!!!!!!!!" << endl;
  return 0;
}

void picasaCache::needPath( const pathParser &path ) {
  pleaseUpdate( path );
}

void picasaCache::unlink( const pathParser &p ) throw ( enum picasaCache::exceptionType ) {
  if ( p == logPath )  {
    cacheElement c;
    getFromCache( p, c );
    c.cachePath="";
    c.size=0;
    putIntoCache( p, c );
    log( "Clear log file.\n" );
    return;
  } else if ( p == updateQueuePath ) {
    boost::mutex::scoped_lock l(update_queue_mutex);
    log( "Clearing update queue" );
    update_queue.clear();
    return;
  } else if ( p == priorityQueuePath ) {
    boost::mutex::scoped_lock l(priority_update_queue_mutex);
    log( "Clearing priority update queue" );
    priority_update_queue.clear();
    return;
  }
    
  if ( p.getType() != pathParser::IMAGE ) throw OPERATION_NOT_SUPPORTED;
  if ( p.getUser() != picasa->getUser() ) throw ACCESS_DENIED;
  cacheElement c;
  if ( ! getFromCache( p, c ) ) throw OBJECT_DOES_NOT_EXIST;
  c.buildPicasaObj( picasa );
  picasaPhoto *photo = dynamic_cast<picasaPhoto *>( c.picasaObj );
  if ( ! photo ) {
    if ( ! c.localChanges ) throw OPERATION_FAILED;
  } else {
    if ( ! haveNetworkConnection ) throw NO_NETWORK_CONNECTION;
    try {
      photo->DELETE();
    } catch (...) {
      throw OPERATION_FAILED;
    }
  }
  removeFromCache( p );
  getFromCache( p.chop(), c );
  c.contents.erase( p.getImage() );
  putIntoCache( p.chop(), c );
  pleaseUpdate( p.chop() );
}

void picasaCache::rmdir( const pathParser &p ) throw ( enum picasaCache::exceptionType ) {
  if ( p.isUser() ) { 
    if ( p == controlDirPath ) throw OPERATION_NOT_SUPPORTED;
    removeFromCache( p );
    return;
  } 
  if ( p.getUser() != picasa->getUser() ) throw ACCESS_DENIED;
  if ( p.isAlbum() ) { 
    cacheElement c;
    if ( ! getFromCache( p, c ) ) throw OBJECT_DOES_NOT_EXIST;
    if ( c.contents.size() > 0 ) throw OPERATION_FAILED;
    c.buildPicasaObj( picasa );
    picasaAlbum *album = dynamic_cast<picasaAlbum *>( c.picasaObj );
    if ( ! album ) {
      if ( ! c.localChanges ) throw OPERATION_FAILED;
    } else {
      if ( ! haveNetworkConnection ) throw NO_NETWORK_CONNECTION;
      try {
	album->DELETE();
      } catch (...) {
	throw OPERATION_FAILED;
      }
    }
    removeFromCache( p );
    getFromCache( p.chop(), c );
    c.contents.erase( p.getImage() );
    putIntoCache( p.chop(), c );
    pleaseUpdate( p.chop() );
    return;
  }
  throw OPERATION_NOT_SUPPORTED;
}

void picasaCache::create( const pathParser &p ) throw ( enum picasaCache::exceptionType ) { 
  cacheElement c;
  if ( p.getType() != pathParser::IMAGE ) {
    if ( p == syncPath ) picasaCache::sync();
    else if ( p == offlinePath ) goOffLine();
    else if ( p == onlinePath ) goOnline();
    throw UNIMPLEMENTED;
  }
  if ( p.getUser() != picasa->getUser() ) throw ACCESS_DENIED;
  if ( ! getFromCache( p.chop(), c ) || ! getFromCache( p.chop().chop(), c ) || getFromCache( p, c ) ) throw OPERATION_FAILED;
  log( "create("+p.getFullName()+"...): creating file.\n" );
  c.type=cacheElement::FILE;
  c.picasaObj=NULL;
  c.localChanges=true;
  c.finalized=false;
  c.generated = false;
  c.authKey="";
  c.name=p.getImage();
  c.size=0;
  c.last_updated=0;
  c.cachePath = p.getFullName();
  string absPath = cacheDir + "/" + c.cachePath;
  int fd = open( absPath.c_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
  if ( fd == -1 ) {
    log( "create(" + p.getFullName()+"...): operation failed.\n" );
    throw OPERATION_FAILED;
  }
  close(fd);
  putIntoCache( p, c );
  getFromCache( p.chop(), c );
  c.contents.insert( p.getImage() );
  putIntoCache( p.chop(), c );
}

void picasaCache::my_open( const pathParser &p, int flags ) throw ( enum picasaCache::exceptionType ) {
  bool rdonly = (( flags & 3 ) == O_RDONLY );
  cacheElement c;
  if ( ! getFromCache( p, c) ) throw OBJECT_DOES_NOT_EXIST;
  c.numOfOpenWr++;
  putIntoCache( p, c);
  if ( c.generated ) updateSpecial( p );
  if ( rdonly ) return;
  switch( p.getType() ) { 
    case pathParser::IMAGE:
      if ( p.getUser() == picasa->getUser() ) {
	log( "my_open("+p.getFullName()+"...): opening for writing.\n" );
	return;
      }
    default:
      throw ACCESS_DENIED;
  }
}
      
      
      
void picasaCache::my_mkdir( const pathParser &p ) throw ( enum picasaCache::exceptionType ) {
  switch( p.getType() ) { 
    case pathParser::USER:
      throw UNIMPLEMENTED;
    case pathParser::IMAGE:
      throw OPERATION_NOT_SUPPORTED;
    case pathParser::ALBUM:
      if ( p.getUser() != picasa->getUser() ) throw ACCESS_DENIED;
      newAlbum( p );
  }
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
  int ret=0;
  struct cacheElement e;
  std::cerr<<"read(" << path.getFullName() << "): start " << endl;
  if ( getFromCache( path, e ) ) {
    if ( e.type != cacheElement::FILE ) return 0;
    if ( e.generated ) {
      if ( e.cachePath.size() > 0 ) { 
	return fillBufFromString( e.cachePath, buf, size, offset );
      } else return fillBufFromString( "Data not yet available...\n", buf, size, offset );
    }
    string absPath = cacheDir + "/" + e.cachePath;
    int fd = open( absPath.c_str() , O_RDONLY );
    if ( fd == -1 ) { 
      char *errBuf = strerror( errno );
      string err = "Error opening "+absPath+" (";
      err+=errBuf;
      err+=")\n";
      pleaseUpdate( path, true );
      return fillBufFromString( err, buf, size, offset );
    }
    ret = pread(fd,buf,size, offset);
    close(fd);
    return ret;
  } else return 0;
}

int picasaCache::my_write( const pathParser &path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi ) {
  struct cacheElement e;
  if ( getFromCache( path, e ) ) {
    if ( e.generated ) return -EPERM;
    string absPath = cacheDir + "/" + e.cachePath;
    int fd = open( absPath.c_str(), O_WRONLY );
    if ( fd == -1 ) { 
      string errBuf( strerror( errno ) );
      log( "write("+path.getFullName() +"): open("+absPath+") failed with error:"+errBuf+"\n" );
      return -ENOENT;
    }
    e.localChanges = true;
    e.finalized = false;
    putIntoCache( path, e );
    int ret = pwrite( fd, buf, size, offset );
    close( fd );
    return ret;
  } else return -ENOENT;
}

void picasaCache::my_close( const pathParser &path ) { 
  cacheElement e;
  stringstream ss;
  if ( getFromCache( path, e ) ) {
    ss << " Closing file " << path.getFullName() << " which was opened " << e.numOfOpenWr << " times" << endl;
    log( ss.str() );
    e.numOfOpenWr--;
    if ( e.numOfOpenWr <= 0  && e.localChanges ) { 
      e.numOfOpenWr=0;
      if ( numOfPixels > 0 ) {
	convert magic;
	magic.resize( numOfPixels, cacheDir + "/" + e.cachePath );
      }
      e.finalized = true;
      e.last_updated=0;
      putIntoCache( path, e );
      localChange( path );
    }
    putIntoCache(path, e);
  } else {
    log( "Closing non-existant file " + path.getFullName() + " ... WTF ???\n" );
    throw OBJECT_DOES_NOT_EXIST;
  }
}

