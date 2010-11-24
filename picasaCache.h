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

#include <iosfwd>

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
#include "atomFeed.h"
#include "picasaAlbum.h"
#include "picasaPhoto.h"
#include "config.h"


struct cacheElement { 
  enum elementType { DIRECTORY, FILE }; 
  enum elementType type;
  
  /* cacheElement format version */
  int version;
  
  /* shared */
  std::string name;
  ssize_t size;
  bool world_readable, writeable;
  bool localChanges; // true if local changes not yet pushed to the server
  bool finalized; // false for files which were changed and not yet closed, true for everything else (also not cached to disk)
  int numOfOpenWr; // Number of times the element was opend (not only for writing !!!)

  std::time_t last_updated;
  
  std::string xmlRepresentation; // To reconstruct either picasaPhoto or picasaAlbum from...
  atomEntryPtr picasaObj; // Constructed from the xmlRepresentation
  std::string cachedVersion; // The "checksum" of the object (ETag)
   
  /* only for DIRECTORY */
  std::string authKey;
  std::set<std::string> contents;

  /* only for FILE */
  bool generated;
  std::string cachePath;
 

  cacheElement(): name(""), size(0), world_readable(false), writeable(false),
		  localChanges(false), last_updated(0), xmlRepresentation(""),
		  cachedVersion(""), authKey(""), generated(false),
		  cachePath(""), numOfOpenWr(0) {};
		  
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
			      break;
		      case cacheElement::FILE:
			      ar & generated;
			      ar & cachePath;
			      break;
	      }
	    }
  /*
   * Takes ownership of album
   */
  void fromAlbum( picasaAlbumPtr album );

  /*
   * Takes ownership of photo
   */
  void fromPhoto( picasaPhotoPtr photo );
  
  friend std::ostream &operator<<( std::ostream &out, const cacheElement &element );
};


class picasaCache { 
	public:
		enum exceptionType { OBJECT_DOES_NOT_EXIST, UNIMPLEMENTED, ACCESS_DENIED, OPERATION_NOT_SUPPORTED, UNEXPECTED_ERROR, OPERATION_FAILED, NO_NETWORK_CONNECTION };
	        static std::string exceptionString( exceptionType );



	public:
		picasaCache( picasaConfig &cf );
		~picasaCache();

		bool isDir( const pathParser &p );
		bool isCached( const pathParser &p );

		void needPath( const pathParser &p );

		bool isOffline() const { return ( ! haveNetworkConnection ); };
		bool goOffLine() { haveNetworkConnection = false; };
		bool goOnline();

		std::set<std::string> ls( const pathParser &p ) throw (enum exceptionType);
                int read( const pathParser &p, char *buf, size_t size, off_t offset, struct fuse_file_info *fi );
		std::string getXAttr( const pathParser &p, const std::string &attrName ) throw (enum exceptionType);
		std::list<std::string> listXAttr( const pathParser &p ) throw (enum exceptionType);
		int getAttr( const pathParser &p, struct stat *stBuf );
		
		void unlink( const pathParser &p ) throw ( enum exceptionType );
		void rmdir( const pathParser &p ) throw (enum exceptionType);
		void create( const pathParser &p ) throw (enum exceptionType);
		void my_mkdir( const pathParser &p ) throw (enum exceptionType);
		void my_open( const pathParser &p, int flags ) throw ( enum exceptionType );
	        int my_write( const pathParser &arg1, const char* arg2, size_t arg3, off_t arg4, fuse_file_info* arg5 );
		void my_close( const pathParser &p );
		
		void sync();


	private:
	        picasaConfig conf;
	  
		long numOfPixels;
		long maxJobThreads;
		
		int updateInterval;
		volatile bool haveNetworkConnection;
		boost::shared_ptr<boost::thread> update_thread, priority_update_thread;
		boost::mutex priority_update_queue_mutex, update_queue_mutex, local_change_queue_mutex, job_threads_mutex;
		std::list<pathParser> update_queue,local_change_queue, priority_update_queue;
		volatile bool work_to_do;
		volatile bool kill_thread;
		void update_worker();
		void doUpdate( const pathParser p ) throw ( enum picasaCache::exceptionType );
		void priority_worker();
		void pushChange( const pathParser p ) throw ( enum picasaCache::exceptionType );

		void updateUser( const pathParser p ) throw ( enum picasaCache::exceptionType );
		void updateAlbum( const pathParser p ) throw ( enum picasaCache::exceptionType );
		void updateImage( const pathParser p ) throw ( enum picasaCache::exceptionType );
		void updateStatsFile();
		void updateAuthKeys();
		void updateLCQueueFile();
                void updateUQueueFile();
                void updatePQueueFile();
		void updateSpecial( const pathParser p );
		void pleaseUpdate( const pathParser p, bool priority = false );
		void localChange( const pathParser p );

		void pushImage( const pathParser p ) throw ( enum picasaCache::exceptionType );
		void pushAlbum( const pathParser p ) throw ( enum picasaCache::exceptionType );
		void newAlbum( const pathParser p ) throw ( enum picasaCache::exceptionType );

		// WARNING: Need to acquire a lock on cache_mutex before calling this function.
		//          This function does not acquire any locks.
		// std::list<pathParser> getChildrenInCache( const pathParser &p );

		bool isCached( const std::string &key );
		void clearCache();
	
		void createRootDir();
		void insertControlDir();
		static const pathParser controlDirPath, logPath, statsPath, updateQueuePath, priorityQueuePath, localChangesQueuePath, authKeysPath, syncPath, offlinePath, onlinePath, helpPath;
		bool isSpecial( const pathParser &path );
		void log( std::string msg );
		std::string toString();
		
		
		bool insertDir( const pathParser &p, const std::string &authKey = "", bool writeable = false );
		bool insertSpecialFile( const pathParser &p, bool world_readable = true, bool world_writeable = false );
		bool getFromCache( const pathParser &p, struct cacheElement &e );
		void putIntoCache( const pathParser &p, const struct cacheElement &e );
		size_t getCacheSize();
		void removeFromCache( const pathParser &p );
		// WARNING: Need to acquire a lock on cache_mutex and update_queue_mutex before calling this function.
		//          This function does not acquire any locks.
		void no_lock_removeFromCache( const pathParser &p );
		
		time_t last_saved;
		void saveCacheToDisk();




		boost::mutex cache_mutex;
		std::map< std::string, struct cacheElement > cache;
		std::string cacheDir;

		gAPI *api;
		picasaService *picasa;


};

#endif /* _picasaCache_H */
