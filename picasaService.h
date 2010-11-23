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
#include <list>
#include <set>

#include "picasaAlbum.h"
#include "atomFeed.h"

class picasaPhoto;
class gAPI;

class picasaService { 
	private:
	  gAPI *api;

		static std::string newPhotoURL( const std::string &user, const std::string &albumName ) { 
		  return "http://picasaweb.google.com/data/feed/api/user/"+user+"/album/"+albumName;
		}
		static std::string newCommentURL( const std::string &user, const std::string &albumID, const std::string &photoID, const std::string &authKey="" ) {
		  std::string auth = "";
		  if ( authKey.compare("") != 0 ) auth = "?authkey="+authKey;
		  return "http://picasaweb.google.com/data/feed/api/user/"+user+"/albumid/"+albumID+"/photoid/"+photoID+auth;
		}
		static std::string commentFeedURL( const std::string &user, const std::string &albumID, const std::string &photoID, const std::string &authKey="" ) {
		  std::string auth = "?";
		  if ( authKey.compare("") != 0 ) auth = "?authkey="+authKey+"&";
		  return "http://picasaweb.google.com/data/feed/api/user/"+user+"/albumid/"+albumID+"/photoid/"+photoID+auth+"kind=comment";
		}

		static std::string albumFeedURL( const std::string &user ) {
		  return "http://picasaweb.google.com/data/feed/api/user/"+user;
		}

		static std::string albumURL( const std::string &user, const std::string &albumID, const std::string &authKey = "" ) { 
		  std::string auth="";
		  if ( authKey.compare("") != 0 ) auth = "?authkey="+authKey; 
		  return "http://picasaweb.google.com/data/entry/api/user/"+user+"/albumid/"+albumID+auth;
		}
		
		static std::string albumNameURL( const std::string &user, const std::string &albumName, const std::string &authKey = "" ) { 
		  std::string auth="";
		  if ( authKey.compare("") != 0 ) auth = "?authkey="+authKey; 
		  return "http://picasaweb.google.com/data/entry/api/user/"+user+"/album/"+albumName+auth;
		}
		

		  

	public:

		enum exceptionType { GENERAL_ERROR };

	public:
		picasaService( gAPI *API );
		std::set<picasaAlbum> albumList( const std::string &user = "" ) throw ( enum exceptionType );
		picasaAlbum getAlbumByID( const std::string &albumID, const std::string &user = "", const std::string &authKey = "" ) throw ( enum exceptionType );
		picasaAlbum getAlbumByName( const std::string &albumName, const std::string &user = "", const std::string &authKey = "" ) throw ( enum exceptionType );
		picasaAlbum newAlbum( const std::string &Title, const std::string &Desc = "", const std::string &Location = "", picasaAlbum::accessType access = picasaAlbum::PUBLIC, bool comments = false, const std::string &keywords = "");
		atomEntryPtr albumFromXML( const std::string &xml ) const;
		atomEntryPtr photoFromXML( const std::string &xml ) const;

		friend class picasaPhoto;

		std::string getUser() const;
};







#endif /* _picasaService_H */
