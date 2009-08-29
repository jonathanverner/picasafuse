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

#include <boost/filesystem/operations.hpp>

#include "picasaPhoto.h"
#include "gAPI.h"
#include "picasaService.h"
#include "atomFeed.h"

#ifndef TIXML_USE_TICPP
#define TIXML_USE_TICPP
#endif
#include "ticpp/ticpp.h"



using namespace std;


picasaPhoto::picasaPhoto(gAPI* api, const string& xml): atomEntry(api) {
  if ( xml != "" ) loadFromXML(xml);
}

picasaPhoto::picasaPhoto(const picasaPhoto& p): atomEntry(p) {

}


void picasaPhoto::extractMediaEditURL() {// FIXME: Needs real implementation
  mediaEditURL = "";
}

picasaPhoto::picasaPhoto( atomEntry &entry ): atomEntry( entry ) { 
  extractMediaEditURL();
}

picasaPhoto::picasaPhoto( gAPI *API, const string &fileName, const string &albumName, const string &Summary, const string &Title ) throw( enum atomObj::exceptionType ): 
	atomEntry( API )
{
  string url = picasaService::newPhotoURL( api->getUser(), albumName );
  if ( fileName.find(".jpg") == string::npos ) { 
      cerr << "picasaPhoto::picasaPhoto(...,"<<fileName<<"): Only jpeg's allowed at the moment.\n";
  }
  list<string> hdrs;
  string myTitle = Title;
  if ( myTitle == "" ) {
    boost::filesystem::path pth( fileName );
    myTitle = pth.filename();
  }
  hdrs.push_back("Slug: "+myTitle );
  if ( ! loadFromXML(api->POST_FILE( url, fileName, "image/jpeg", hdrs ) ) ) throw atomObj::ERROR_CREATING_OBJECT;
  if ( Summary != "" ) {
    addOrSet( xml->FirstChildElement(), "summary", Summary );
    atomEntry::UPDATE();
  }
}

bool picasaPhoto::upload( const std::string &fileName ) {
  list<string> hdrs;
  api->PUT_FILE( editURL, fileName, "image/jpeg", hdrs );
  UPDATE();
}

string picasaPhoto::getSummary() const { 
  return xml->FirstChildElement()->FirstChildElement("summary")->GetText(false);
}

string picasaPhoto::getAlbumID() const { 
  return xml->FirstChildElement()->FirstChildElement("gphoto:albumid")->GetText(false);
}

string picasaPhoto::getPhotoID() const { 
  return xml->FirstChildElement()->FirstChildElement("gphoto:id")->GetText(false);
}

size_t picasaPhoto::getSize() const { 
  int sz=0;
  try {
    xml->FirstChildElement()->FirstChildElement("gphoto:size")->GetText<int>(&sz);
  } catch ( ticpp::Exception &ex ) {
    return 0;
  }
  return sz*sizeof(char);
}

string picasaPhoto::getAuthKey() const { 
  int authKeyPos = editURL.find( "?authkey=" );
  if ( authKeyPos != string::npos )
    return editURL.substr( authKeyPos+9 );
  else return "";
}


string picasaPhoto::getUser() const { 
  int userSPos = selfURL.find("user/")+5;
  int userEPos = selfURL.find_first_of("/", userSPos );
  return selfURL.substr(userSPos, userEPos-userSPos );
}

string picasaPhoto::getPhotoURL() const { 
  return xml->FirstChildElement()->FirstChildElement("content")->GetAttribute("src");
}


void picasaPhoto::download( const string &fileName ) { 
 api->DOWNLOAD( getPhotoURL(), fileName );
}

void picasaPhoto::setSummary( string summary ) { 
  addOrSet( xml->FirstChildElement(), "summary", summary );
}

void picasaPhoto::addComment( string comment ) { 
  string cXML = "<entry xmlns='http://www.w3.org/2005/Atom'>\
                 <content>"+comment+"</content>\
                 <category scheme=\"http://schemas.google.com/g/2005#kind\"\
                 term=\"http://schemas.google.com/photos/2007#comment\"/>\
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
      ret.push_back( (*it)->xml->FirstChildElement()->FirstChildElement("content")->GetText() );
  } catch ( ticpp::Exception &ex ) { 
    cerr << " picasaPhoto::getComments(): " << ex.what() << "\n";
  }
  for(list<atomEntry *>::iterator it=comments.begin(); it != comments.end(); it++)
    delete *it;
  return ret;
}

std::ostream &operator<<(std::ostream &out, const picasaPhoto &photo) { 
  out << "Title: " << photo.getTitle()<< "\n";
  out << "Summary: " << photo.getSummary() << "\n";
  out << "Size: " << photo.getSize()/1024 << "Kb \n";
  out << "AlbumID: " << photo.getAlbumID() << "\n";
  out << "PhotoID: " << photo.getPhotoID() << "\n";
  out << "AuthKey: " << photo.getAuthKey() << "\n";
  out << "SelfURL: " << photo.selfURL << "\n";
  out << "PhotoURL: " << photo.getPhotoURL() << "\n";
}

