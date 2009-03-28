#ifndef _picasaService_H
#define _picasaService_H

/***************************************************************
 * picasaService.h
 * @Author:      Jonathan Verner (jonathan.verner@matfyz.cz)
 * @License:     GPL v2.0 or later
 * @Created:     2009-03-27.
 * @Last Change: 2009-03-27.
 * @Revision:    0.0
 * Description:
 * Usage:
 * TODO:
 *CHANGES:
 ***************************************************************/

#include <string>

class picasaAlbum;
class picasaPhoto;

class picasaService { 
	private:

		static std::string newPhotoURL( const std::string &user, const std::string &albumName ) { 
		  return "http://picasaweb.google.com/data/feed/api/user/"+user"/album/"+albumName;
		}
		static std::string newCommentURL( const std::string &user, const std::string &albumID, const std::string &photoID ) {
		  return "http://picasaweb.google.com/data/feed/api/user/"+user+"/albumid/"+albumID+"/photoid/"+photoID;
		}



	public:
		picasaService( const std::string & email, const std::string &password = "" );

		picasaAlbum *createAlbum( const std::string Title );
		picasaAlbum *getAlbum( const std::string &user, const std::string &albumName, const std::string authKey = "" );
		std::list<picasaAlbum *> getAlbums( const std::string &user = "");
		std::list<picasaPhoto *> getPhotos( const std::string &user = "");
};







endif /* _picasaService_H */
