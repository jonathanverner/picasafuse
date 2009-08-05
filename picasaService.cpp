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
  list<atomEntry *> albums = albumFeed.getEntries();
  set<picasaAlbum> ret;
  for(list<atomEntry *>::iterator a = albums.begin(); a != albums.end(); ++a) { 
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
  album.deleteXmlOnExit=false;
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
  album.deleteXmlOnExit=false;
  return album;
}


atomEntry* picasaService::albumFromXML(const std::string& xml) const {
//  return NULL;
  picasaAlbum *ret =  new picasaAlbum( api );
  ret->loadFromXML( xml );
  return ret;
}

atomEntry* picasaService::photoFromXML( const std::string& xml ) const {
  return new picasaPhoto( api, xml );
}

std::string picasaService::getUser() const {
  return api->getUser();
}
