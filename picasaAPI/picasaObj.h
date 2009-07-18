#ifndef _picasaObj_H
#define _picasaObj_H

/***************************************************************
 * picasaObj.h
 * @Author:      Jonathan Verner (jonathan.verner@matfyz.cz)
 * @License:     GPL v2.0 or later
 * @Created:     2009-03-18.
 * @Last Change: 2009-03-18.
 * @Revision:    0.0
 * Description:
 * Usage:
 * TODO:
 *CHANGES:
 ***************************************************************/

#include <string>

class gAPI;

namespace ticpp { 
  class Document;
}

class picasaObj { 
	public:
		enum cachePolicy { TRY_CACHE_FIRST, TRY_PICASA_FIRST, ONLY_CACHE, ONLY_PICASA };
	private:
		std::string cacheRoot;
		bool needsSync;

	protected:
		bool readOnly;
		std::string createURL, editURL, selfURL;
		std::string cacheFileName;

		ticpp::Document *xml;
		gAPI *api;

		virtual bool loadFromXML( ticpp::Document *doc ) = 0;
		virtual bool loadFromXML( const std::string &xml );
		virtual bool loadFromXMLFile( const std::string &xmlFile );

		static bool fileExists( const std::string &fName );
		bool mkCacheDir( const std::string &dirName );

		void markModified();

		std::string getCacheRoot() const { return cacheRoot; };
		void setCacheRoot( const std::string &cR ) { cacheRoot = cR; };

	protected:
		picasaObj( gAPI *api, std::string cacheRoot = "~/.picasaCache" );
		picasaObj( std::string userName, std::string password = "", std::string cacheRoot = "~/.picasaCache" );

	public:

		bool load( enum cachePolicy pol );
		bool save( enum cachePolicy pol );

		virtual bool createObj();
		virtual bool deleteObj();

		virtual bool loadFromCache();
		virtual bool loadFromPicasa();
		virtual bool saveToCache();
		virtual bool saveToPicasa();
		virtual bool isCached();

		void setReadOnly() { readOnly = true; };
		gAPI *getAPI() { return api; };
};


#endif /* _picasaObj_H */
