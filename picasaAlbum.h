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
#include <list>

#include "picasaObj.h"

class picasaPhoto;



#include "atomEntry.h"

class picasaAlbum : public atomEntry {
	public:
		enum accessType { PUBLIC, UNLISTED, ACL };


	protected:
		picasaAlbum( atomEntry &entry );
		picasaAlbum( gAPI *api, const std::string &Title, const std::string &Description="", const std::string &Location="", enum accessType access = PUBLIC, bool comments, const std::string &keywords );
	public:

		std::string getSummary();
		std::string getLocation();
		std::string getShortTitle();
		std::string getAuthKey();
		std::string getUser();
		enum accessType getAccessType();
		

		void setSummary( std::string summary );
		void setLocation( std::string location );
		void setAccessType( enum accessType access );


		picasaPhoto *addPhoto( const std::string &fileName, const std::string &Title="", const std::string &Summary = ""   );

		std::list<picasaPhoto *> getPhotos();

		friend picasaService;

};





#endif /* _picasaAlbum_H */
