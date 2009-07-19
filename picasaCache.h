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
struct fuse_file_info;
class pathParser;


struct cacheElement { 
  enum elementType { DIRECTORY, FILE };
 
  enum elementType type;
  
  /* shared */
  std::string name;
  ssize_t size;
  bool world_readable, writeable;

  time_t last_updated;

  /* only for DIRECTORY */
  std::string authKey;
  std::set<std::string> contents;

  /* only for FILE */
  bool generated;
  std::string cachePath;

  const struct cacheElement &operator=(const struct cacheElement &e);

  template<class Archive>
	    void serialize(Archive & ar,  const unsigned int version)
	    {
	      ar & name;
	      ar & size;
	      ar & world_readable;
	      ar & writeable;
	      ar & type;

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



  void *private_data;
};


class picasaCache { 

	public:
		picasaCache( const std::string &user = "", const std::string &password = "", const std::string &cache = "~/.picasaFUSE" );
		~picasaCache();

		bool isDir( const pathParser &p );
		bool isFile( const pathParser &p );
		bool exists( const pathParser &p );

		void needPath( const pathParser &p );

		std::set<std::string> ls( const pathParser &p );
		size_t getSize( const pathParser &p );
                int read( const pathParser &p, char *buf, size_t size, off_t offset, struct fuse_file_info *fi );

	private:
		boost::shared_ptr<boost::thread> update_thread;
		boost::mutex update_queue_mutex;
		std::list<pathParser> update_queue;
		volatile bool work_to_do;
		volatile bool kill_thread;
		void update_worker();
		void doUpdate( const pathParser p );

		void updateUser( const std::string userName );

		void pleaseUpdate( const pathParser p );


		bool isCached( const std::string &key );
		bool isCached( const pathParser &p );
		void clearCache();
		bool getFromCache( const pathParser &p, struct cacheElement &e );
		bool getFromCache( const std::string &key, struct cacheElement &e );
		void putIntoCache( const pathParser &p, const struct cacheElement &e );
		void putIntoCache( const std::string &key, const struct cacheElement &e );

		boost::mutex cache_mutex;
		std::map< std::string, struct cacheElement > cache;
		std::string cacheDir;

		gAPI *api;

		void log( std::string msg );
};

#endif /* _picasaCache_H */
