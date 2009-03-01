#ifndef _picasaPhoto_H
#define _picasaPhoto_H

/***************************************************************
 * picasaPhoto.h
 * @Author:      Jonathan Verner (jonathan.verner@matfyz.cz)
 * @License:     GPL v2.0 or later
 * @Created:     2009-02-28.
 * @Last Change: 2009-02-28.
 * @Revision:    0.0
 * Description:
 * Usage:
 * TODO:
 *CHANGES:
 ***************************************************************/

#include <string>
#include <list>

#include "picasaAlbum.h"

namespace ticpp { 
  class Document;
  class Element;
}


class picasaPhoto { 
	private:
		std::string caption, fileName, checkSum, selfURL, editURL, altURL, photoURL;
		std::list< std::string > comments;
		ticpp::Document *xml;
		size_t size;

		picasaAlbum *album;

		enum picasaAlbum::updatePolicy syncPol;
		bool upToDate;
		bool modified();


		picasaPhoto( picasaAlbum *album, ticpp::Element *xmlEntry );

		bool isURL( const std::string &fileName );
		void loadFromXML( const std::string &xmlText );
		void loadFromXML( const ticpp::Document *xmlDoc );

	public:

		picasaPhoto( picasaAlbum *album, const std::string &fileNameOrURL );

		bool setCaption( const std::string &caption );
		bool setFileName( const std::string &fileName );
		bool setCheckSum( const std::string &checkSum );

		bool uploadPhoto( const std::string &fileName );
		bool downloadPhoto( const std::string &fileName );

		bool deletePhoto();

		void setUpdatePolicy(const enum picasaAlbum::updatePolicy policy);
		bool update();


		std::string getCaption();
		std::string getFileName();
		std::string getCheckSum();

		size_t getSize() { return size; }
		std::string URL() { return photoURL; }

		int numOfComments() { return comments.size(); };
		bool addComment( const std::string &comment );
		std::list< std::string > allComments() { return comments; }

		friend class picasaAlbum;
};

	


#endif /* _picasaPhoto_H */
