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

#include "atomEntry.h"

#include <string>
#include <list>

class gAPI;


class picasaPhoto: public atomEntry { 
	private:
		std::string mediaEditURL;

		void extractMediaEditURL();

	protected:
		picasaPhoto( atomEntry &entry );
		picasaPhoto( gAPI *api, const std::string &fileName, const std::string &albumName, const std::string &Title="", const std::string &Summary="" );

	public:

		std::string getSummary();
		std::string getPhotoURL();
		std::string getAlbumID();
		std::string getPhotoID();
		std::string getAuthKey();
		std::string getUser();
		void download( const std::string &fileName );

		void setSummary( std::string summary );
		void addComment( std::string comment );
	

		void UPDATE( const std::string &fileName );


		std::list<std::string> getComments();

		friend class picasaAlbum;

};




#endif /* _picasaPhoto_H */
