/***************************************************************
 * picasaService.cpp
 * @Author:      Jonathan Verner (jonathan.verner@matfyz.cz)
 * @License:     GPL v2.0 or later
 * @Created:     2009-07-12.
 * @Last Change: 2009-07-12.
 * @Revision:    0.0
 * Description:
 * Usage:
 * TODO:
 *CHANGES:
 ***************************************************************/
#include <iostream>
 
#include "picasaService.h"
#include "picasaAlbum.h"
#include "picasaPhoto.h"
#include "gAPI.h"
#include "atomFeed.h"

using namespace std;

picasaService::picasaService( const std::string &email, const std::string &password ):
	api( new gAPI( email, password, "picasaFUSE" ) )
{
};

set<picasaAlbum> picasaService::albumList( const string &user ) throw (enum exceptionType) {
  atomFeed albumFeed( api );
  if ( ! albumFeed.loadFromURL( albumFeedURL( user ) ) ) {
    throw GENERAL_ERROR;
  }
  list<atomEntryPtr> albums = albumFeed.getEntries();
  set<picasaAlbum> ret;
  for(list<atomEntryPtr>::iterator a = albums.begin(); a != albums.end(); ++a) {
    picasaAlbum album( **a );
    ret.insert( album );
  }
  return ret;
}
picasaAlbum picasaService::getAlbumByID( const std::string &albumID, const std::string &user, const std::string &authKey ) throw ( enum exceptionType ) {
  picasaAlbum album( api );
  std::string us = user;
  if ( user == "" ) us = api->getUser();
  std::cerr << "URL: " << albumURL( us, albumID, authKey ) << "\n";
  if ( ! album.loadFromURL( albumURL( us, albumID, authKey ) ) ) { 
    throw GENERAL_ERROR;
  }
  return album;
}


picasaAlbum picasaService::newAlbum( const std::string& Title, const std::string& Desc, const std::string& Location, picasaAlbum::accessType access, bool comments, const std::string& keywords ) {
  picasaAlbum album( api, Title, Desc, Location, access, comments, keywords );
  return album;
}


picasaAlbum picasaService::getAlbumByName( const string& albumName, const string& user, const string& authKey ) throw ( enum exceptionType ) {
  picasaAlbum album( api );
  std::string us = user;
  if ( user == "" ) us = api->getUser();
  std::cerr << "URL: " << albumNameURL( us, albumName, authKey ) << "\n";
  if ( ! album.loadFromURL( albumNameURL( us, albumName, authKey ) ) ) { 
    throw GENERAL_ERROR;
  }
  return album;
}


atomEntryPtr picasaService::albumFromXML(const std::string& xml) const {
  atomEntryPtr ret( new picasaAlbum( api ) );
  ret->loadFromXML( xml );
  return ret;
}

atomEntryPtr picasaService::photoFromXML( const std::string& xml ) const {
  atomEntryPtr ret( new picasaPhoto( api, xml ) );
  return ret;
}

std::string picasaService::getUser() const {
  return api->getUser();
}
