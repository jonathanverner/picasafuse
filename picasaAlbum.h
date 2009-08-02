#ifndef _picasaAlbum_H
#define _picasaAlbum_H

/***************************************************************
 * picasaAlbum.h
 * @Author:      Jonathan Verner (jonathan.verner@matfyz.cz)
 * @License:     GPL v2.0 or later
 * @Created:     2009-02-27
 * @Last Change: 2009-02-27.
 * @Revision:    0.0
 * Description:
 * Usage:
 * TODO:
 *CHANGES:
 ***************************************************************/

#include <string>
#include <list>
#include <iosfwd>


class picasaPhoto;



#include "atomEntry.h"

class picasaAlbum : public atomEntry {
	public:
		enum accessType { PUBLIC, UNLISTED, ACL };


	protected:
		picasaAlbum( gAPI* api, const std::string& xml = "" );
		picasaAlbum( atomEntry &entry );
		picasaAlbum( gAPI *api, const std::string &Title, const std::string &Description="", const std::string &Location="", enum accessType access = PUBLIC, bool comments = false, const std::string &keywords = "");
	public:

	  	picasaAlbum( const picasaAlbum &a );
		
		std::string getSummary() const;
		std::string getLocation() const;
		std::string getShortTitle() const;
		std::string getAuthKey() const;
		std::string getUser() const;
		enum accessType getAccessType() const;
		

		void setSummary( std::string summary );
		void setLocation( std::string location );
		void setAccessType( enum accessType access );


		picasaPhoto *addPhoto( const std::string &fileName, const std::string &Title="", const std::string &Summary = ""   );

		std::list<picasaPhoto *> getPhotos();

		friend class picasaService;
		friend std::ostream &operator<<(std::ostream &out, const picasaAlbum &album);

};





#endif /* _picasaAlbum_H */
