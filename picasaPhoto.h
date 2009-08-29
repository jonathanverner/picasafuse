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
		picasaPhoto( gAPI* api, const std::string& xml = "" );
		picasaPhoto( atomEntry &entry );
		picasaPhoto( gAPI *api, const std::string &fileName, const std::string &albumName, const std::string &Summary="", const std::string &Title="" ) throw( enum atomObj::exceptionType );

	public:
	  
		picasaPhoto( const picasaPhoto &p );

		std::string getSummary() const;
		std::string getPhotoURL() const;
		std::string getAlbumID() const;
		std::string getPhotoID() const;
		std::string getAuthKey() const;
		std::string getUser() const;
		size_t getSize() const;
		void download( const std::string &fileName );
		bool upload( const std::string &fileName );

		void setSummary( std::string summary );
		void addComment( std::string comment );
	

		std::list<std::string> getComments();

		friend std::ostream &operator<<(std::ostream &out, const picasaPhoto &photo);


		friend class picasaAlbum;
		friend class picasaService;

};




#endif /* _picasaPhoto_H */
