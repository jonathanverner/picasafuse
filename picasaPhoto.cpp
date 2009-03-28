/***************************************************************
 * picasaPhoto.cpp
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


#include "picasaPhoto.h"
#include "gAPI.h"
#include "picasaService.h"

#ifndef TIXML_USE_TICPP
#define TIXML_USE_TICPP
#endif
#include "ticpp/ticpp.h"



using namespace std;



picasaPhoto::picasaPhoto( atomEntry &entry ): atomEntry( entry ) { 
  extractMediaEditURL();
}

picasaPhoto::picasaPhoto( gAPI *API, const string &fileName, const string &albumName, const string &Title, const string &Summary ): 
	atomEntry( API )
{
  string url = picasaService::newPhotoURL( api->getUser(), albumName );
  if ( fileName.find(".jpg") == string::npos ) { 
      cerr << "picasaPhoto::picasaPhoto(...,"<<fileNameOrURL<<"): Only jpeg's allowed at the moment.\n";
  }
  list<string> hdrs;
  hdrs.push_back("Slug: "+Title );
  loadFromXML(api->POST_FILE( url, fileName, "image/jpeg", hdrs ) );
  addOrSet( xml->FirstChildElement(), "summary", Summary );
  UPDATE();
}


string picasaPhoto::getSummary() { 
  return xml->FirstChildElement()->FirstChildElement("summary")->GetText(false);
}

string picasaPhoto::getAlbumID() { 
  return xml->FirstChildElement()->FirstChildElement("gphoto:albumid")->GetText(false);
}

string picasaPhoto::getPhotoID() { 
  return xml->FirstChildElement()->FirstChildElement("gphoto:id")->GetText(false);
}

string picasaPhoto::getAuthKey() { 
  int authKeyPos = editURL.find( "?authkey=" );
  if ( authKeyPos != string::npos )
    return altURL.substr( authKeyPos+9 )
  else return "";
}


string picasaAlbum::getUser() { 
  int userSPos = selfURL.find("user/")+5;
  int userEPos = selfURL.find_first_of("/", userSPos );
  return selfURL.substr(userSPos, userEPos-userSPos );
}

string picasaPhoto::getPhotoURL() { 
  return xml->FirstChildElement()->FirstChildElement("content")->GetAttribute("src");
}

void picasaPhoto::download( const string fileName ) { 
 api->DOWNLOAD( getPhotoURL, fileName );
}

void picasaPhoto::setSummary( string summary ) { 
  addOrSet( xml->FirstChildElement(), "summary", summary );
}

void picasaPhoto::addComment( string comment ) { 
  string cXML = "<entry xmlns='http://www.w3.org/2005/Atom'>\\
              <content>"+comment+"</content>\\
	      <category scheme=\"http://schemas.google.com/g/2005#kind\"\\
	          term=\"http://schemas.google.com/photos/2007#comment\"/>\\
                 </entry>";
  api->POST( picasaService::newCommentURL( getUser(), getAlbumID(), getPhotoID(), getAuthKey() ), cXML );
}

list<string> picasaPhoto::getComments() { 
  atomFeed commentFeed( api );
  string URL = picasaService::commentFeedURL( getUser(), getAlbumID(), getPhotoID(), getAuthKey() );
  commentFeed.loadFromURL( URL );
  list<atomEntry *> comments = commentFeed.getEntries();
  list<string> ret;
  try { 
    for(list<atomEntry *>::iterator it=comments.begin(); it != comments.end(); it++)
      photos.push_back( (*it)->xml->FirstChildElement()->FirstChildElement("content")->GetText() );
  } catch ( ticpp::Exception &ex ) { 
    cerr << " picasaPhoto::getComments(): " << ex.what() << "\n";
  }
  for(list<atomEntry *>::iterator it=comments.begin(); it != comments.end(); it++)
    delete *it;
  return ret;
}


