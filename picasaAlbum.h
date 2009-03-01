#ifndef _picasaAlbum_H
#define _picasaAlbum_H

/***************************************************************
 * picasaAlbum.h
 * @Author:      Jonathan Verner (jonathan.verner@matfyz.cz)
 * @License:     GPL v2.0 or later
 * @Created:     2009-02-27.
 * @Last Change: 2009-02-27.
 * @Revision:    0.0
 * Description:
 * Usage:
 * TODO:
 *CHANGES:
 ***************************************************************/

#include <string>

class picasaAPI;

namespace ticpp { 
  class Document;
}

class picasaAlbum { 
	public:
		
		// AUTO_SYNC: synchronize upon every set operation
		// SEMI_AUTO_SYNC: synchronize upon delete
		// MANUAL_SYNC: synchronize only when calling the update method explicitly
		// NO_SYNC: never synchronize, not even when calling the update method
		//          (NO_SYNC causes all modifications to be lost upon delete)
		enum updatePolicy { AUTO_SYNC, SEMI_AUTO_SYNC, MANUAL_SYNC, NO_SYNC };
		enum feedType { ATOM_FEED, HTML_FEED, NEW_ALBUM_TITLE, AUTO_GUESS };
		enum accessType { PUBLIC, UNLISTED, ACL };

	private:
		std::string title, shortTitle, pubDate, modDate, description, selfURL, editURL,altURL, authKey;
		enum accessType access;

		ticpp::Document *xml;
		picasaAPI *api;
		bool upToDate, readOnly;
		enum updatePolicy syncPol;

		bool modified();

		void newAlbum( const std::string &albumTitle );
		void loadFromAtom( const std::string &feedURL );
		void loadFromXML( const std::string &xml );

		std::string htmlFeedURL2AtomFeedURL( const std::string &feedURL );
		enum feedType guessFeedType( const std::string &feedURL );

		std::string extractAuthKey( std::string &URL );
		picasaAPI *getAPI() { return api; }



	public:

		picasaAlbum( picasaAPI *api, std::string feedURL, enum feedType = AUTO_GUESS, std::string authKey="" );
		~picasaAlbum();
	
		void setUpdatePolicy(const enum updatePolicy policy);
		bool update();

		bool deleteAlbum();

		std::string getShortTitle() const { return shortTitle; };
		std::string getTitle() const { return title; };
		std::string getPubDate() const { return pubDate; };
		std::string getModDate() const { return modDate; };
		std::string getDescription() const { return description; };
		std::string getAuthKey() const { return authKey; };
		std::string getUser() const;
		std::string getAlbumId() const;

		enum accessType getAccessType() const { return access; };

		bool setTitle( std::string title );
		bool setPubDate( std::string pubDate );
		bool setDescription( std::string description );

		friend std::ostream & operator <<( std::ostream &out, picasaAlbum album );
		friend class picasaPhoto;
};



#endif /* _picasaAlbum_H */
