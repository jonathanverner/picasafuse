#ifndef _picasaCache_H
#define _picasaCache_H

/***************************************************************
 * picasaCache.h
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

#include <time.h>

#include <string>
#include <list>
#include <map>
#include <set>

#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>

class gAPI;
class picasaService;
struct fuse_file_info;
struct stat;
class pathParser;
class atomEntry;
class picasaAlbum;
class picasaPhoto;

#include "atomEntry.h"

struct cacheElement { 
  enum elementType { DIRECTORY, FILE }; 
  enum elementType type;
  
  /* shared */
  std::string name;
  ssize_t size;
  bool world_readable, writeable;
  bool localChanges; // true if local changes not yet pushed to the server

  std::time_t last_updated;
  
  std::string xmlRepresentation; // To reconstruct either picasaPhoto or picasaAlbum from...
  atomEntry *picasaObj; // Constructed from the xmlRepresentation
  std::string cachedVersion; // The "checksum" of the object (ETag)
   
  /* only for DIRECTORY */
  std::string authKey;
  std::set<std::string> contents;

  /* only for FILE */
  bool generated;
  std::string cachePath;
 

  cacheElement(): name(""), size(0), world_readable(false), writeable(false),
		  localChanges(false), last_updated(0), xmlRepresentation(""),
		  picasaObj(NULL), cachedVersion(""), authKey(""), generated(false),
		  cachePath("") {};
		  
  const struct cacheElement &operator=(const struct cacheElement &e);
  
  void buildPicasaObj( picasaService *picasa );

  template<class Archive>
	    void serialize(Archive & ar,  const unsigned int version)
	    {
	      ar & name;
	      ar & size;
	      ar & type;
	      ar & xmlRepresentation;
	      ar & cachedVersion;
	      ar & last_updated;
	      ar & localChanges;

	      switch ( type ) { 
		      case cacheElement::DIRECTORY:
			      ar & authKey;
			      ar & contents;
			      picasaObj = NULL;
			      break;
		      case cacheElement::FILE:
			      ar & generated;
			      ar & cachePath;
			      picasaObj = NULL;
			      break;
	      }
	    }
  /*
   * Takes ownership of album
   */
  void fromAlbum( picasaAlbum* album );

  /*
   * Takes ownership of photo
   */
  void fromPhoto( picasaPhoto* photo );
  
};


class picasaCache { 
	public:
		enum exceptionType { OBJECT_DOES_NOT_EXIST, UNIMPLEMENTED, ACCESS_DENIED, OPERATION_NOT_SUPPORTED, UNEXPECTED_ERROR, OPERATION_FAILED };
	        std::string exceptionString( exceptionType );



	public:
		picasaCache( const std::string &user = "", const std::string &password = "", const std::string &cache = "/tmp/.picasaFUSE", int updateInterval = 600 );
		~picasaCache();

		bool isDir( const pathParser &p );
		bool isFile( const pathParser &p );
		bool exists( const pathParser &p );

		void needPath( const pathParser &p );

		std::set<std::string> ls( const pathParser &p ) throw (enum exceptionType);
                int read( const pathParser &p, char *buf, size_t size, off_t offset, struct fuse_file_info *fi );
		int getAttr( const pathParser &p, struct stat *stBuf );

		void rmdir( const pathParser &p ) throw (enum exceptionType);
		void my_mkdir( const pathParser &p ) throw (enum exceptionType);


	private:
		int updateInterval;
		boost::shared_ptr<boost::thread> update_thread;
		boost::mutex update_queue_mutex;
		std::list<pathParser> update_queue;
		volatile bool work_to_do;
		volatile bool kill_thread;
		void update_worker();
		void doUpdate( const pathParser p ) throw ( enum picasaCache::exceptionType );

		void updateUser( const pathParser p ) throw ( enum picasaCache::exceptionType );
		void updateAlbum( const pathParser p ) throw ( enum picasaCache::exceptionType );
		void updateImage( const pathParser p ) throw ( enum picasaCache::exceptionType );
		void pleaseUpdate( const pathParser p );

		void pushAlbum( const pathParser p ) throw ( enum picasaCache::exceptionType );
		void newAlbum( const pathParser p ) throw ( enum picasaCache::exceptionType );

		// Warning: NOT THREAD SAFE. Need to acquire a lock on cache_mutex
		// before calling this function.
		std::list<pathParser> getChildrenInCache( const pathParser &p );

		bool isCached( const std::string &key );
		bool isCached( const pathParser &p );
		void clearCache();
		bool getFromCache( const pathParser &p, struct cacheElement &e );
		bool getFromCache( const std::string &key, struct cacheElement &e );
		void putIntoCache( const pathParser &p, const struct cacheElement &e );
		void putIntoCache( const std::string &key, const struct cacheElement &e );
		void removeFromCache( const pathParser &p );
		
		time_t last_saved;
		void saveCacheToDisk();

		boost::mutex cache_mutex;
		std::map< std::string, struct cacheElement > cache;
		std::string cacheDir;

		gAPI *api;
		picasaService *picasa;

		void log( std::string msg );
};

#endif /* _picasaCache_H */
