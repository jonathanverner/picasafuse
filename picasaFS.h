#include "fusexx.hpp"

#include <string>

#include <sys/types.h>

class pathParser;
class picasaCache;



class PicasaFS : public fusexx::fuse<PicasaFS> {
          public:
		  PicasaFS ( const std::string &user="", const std::string &pass="", const std::string &cacheDir="/home/joni-test/.picasaFUSE", int updateInterval = 600 ); // Constructor

                  // Overload the fuse methods
		  static int getattr (const char *, struct stat *);
		  static int getxattr (const char *, const char *, char *, size_t );
		  static int listxattr ( const char *, char *, size_t );
		  static int readdir (const char *, void *, fuse_fill_dir_t, off_t, struct fuse_file_info *);
		  static int fuse_open (const char *, struct fuse_file_info *);
		  static int read (const char *, char *, size_t, off_t, struct fuse_file_info *);
		  static int rmdir( const char * );
		  static int mkdir( const char *, mode_t );
		  static void destroy(void *);
		  uid_t UID;
		  gid_t GID;

	  private:

		  // Private variables
		  // Notice that they aren't static, i.e. 
		  // they belong to an instantiated object


		  picasaCache *cache;
};

