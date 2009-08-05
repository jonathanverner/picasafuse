/***************************************************************
 * varPicasaAlbum.cpp
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

#include "picasaAlbum.h"
#include "picasaPhoto.h"
#include "atomFeed.h"
#include "gAPI.h" 

#ifndef TIXML_USE_TICPP
#define TIXML_USE_TICPP
#endif
#include "ticpp/ticpp.h"

#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


using namespace std;
  


picasaAlbum::picasaAlbum( gAPI *api, const string& xml ) : atomEntry( api ) {
  if ( xml != "" ) loadFromXML(xml);
}

picasaAlbum::picasaAlbum( atomEntry &entry ) : atomEntry( entry ) {
}

picasaAlbum::picasaAlbum(const picasaAlbum& a): atomEntry(a) {
}


picasaAlbum::picasaAlbum( gAPI *API, const string &Title, const string &Description, const string &Location, enum accessType access, bool comments, const string &keywords ) :
	atomEntry( API )
{ 
  string acSTR, cm;
  switch ( access ) { 
	  case PUBLIC:
		  acSTR="public";
		  break;
	  case UNLISTED:
		  acSTR="private";
		  break;
	  case ACL:
		  acSTR="private"; /*FIXME*/
		  break;
  }
  if ( comments ) cm = "true"; else cm = "false";
  string xml = "<?xml version='1.0' encoding='utf-8'?> \
	          <entry xmlns='http://www.w3.org/2005/Atom'  \
		  xmlns:media='http://search.yahoo.com/mrss/' \
		  xmlns:gphoto='http://schemas.google.com/photos/2007'> \
		    <title type='text'>"+Title+"</title> \
		    <summary type='text'>"+Description+"</summary> \
		    <gphoto:location>"+Location+"</gphoto:location> \
		    <gphoto:access>"+acSTR+"</gphoto:access> \
		    <gphoto:commentingEnabled>"+cm+"</gphoto:commentingEnabled> \
		    <media:group> \
		        <media:keywords>"+keywords+"</media:keywords> \
		    </media:group> \
		    <category scheme='http://schemas.google.com/g/2005#kind' term='http://schemas.google.com/photos/2007#album'></category> \
		  </entry></xml>";
  loadFromXML( xml );
}


string picasaAlbum::getSummary() const { 
  try {
    return xml->FirstChildElement()->FirstChildElement("summary")->GetText(false);
  } catch ( ticpp::Exception &ex ) { 
    return "";
  }
}

string picasaAlbum::getLocation() const { 
  try {
    return xml->FirstChildElement()->FirstChildElement("location")->GetText(false);
  } catch ( ticpp::Exception &ex ) { 
    return "";
  }
}


string picasaAlbum::getShortTitle() const { 
  int slashPos = altURL.find_last_of("/");
  int queryPos = altURL.find_first_of("?", slashPos);
  if ( queryPos == string::npos ) { 
    return altURL.substr( slashPos+1 );
  } else { 
    return altURL.substr( slashPos+1, queryPos-slashPos-1 );
  }
}

string picasaAlbum::getAuthKey() const { 
  int authKeyPos = altURL.find( "?authkey=" );
  if ( authKeyPos != string::npos )
    return altURL.substr( authKeyPos+9 );
  else return "";
}

string picasaAlbum::getUser() const { 
  int userSPos = selfURL.find("user/")+5;
  int userEPos = selfURL.find_first_of("/", userSPos );
  return selfURL.substr(userSPos, userEPos-userSPos );
}
  
enum picasaAlbum::accessType picasaAlbum::getAccessType() const { 
  string acSTR=xml->FirstChildElement()->FirstChildElement("gphoto:access")->GetText(false);
  if ( acSTR == "private" ) return UNLISTED;
  else return PUBLIC; /*FIXME*/
}

void picasaAlbum::setSummary( string summary ) { 
  addOrSet( xml->FirstChildElement(), "summary", summary );
}

void picasaAlbum::setLocation( string location ) { 
   addOrSet( xml->FirstChildElement(), "gphoto:location", location );
}
 
picasaPhoto *addPhoto( const std::string &fileName, const std::string &Title, const std::string &Summary) {
}


list<picasaPhoto *> picasaAlbum::getPhotos() { 
  atomFeed photoFeed( api );
  string URL = selfURL;
  URL.replace(selfURL.find("entry"), 5, "feed");
  if (URL.find("?authkey=") != string::npos) 
    URL+="&kind=photo";
  else 
    URL+="?kind=photo";
  photoFeed.loadFromURL(URL);
  list<atomEntry *> entries = photoFeed.getEntries();
  list<picasaPhoto *> photos;
  for( list<atomEntry *>::iterator it=entries.begin();it !=entries.end();it++) {
    photos.push_back(new picasaPhoto( **it ) );
    delete *it;
  }
  return photos;
}

picasaPhoto *picasaAlbum::addPhoto( const std::string &fileName, const std::string &Title, const std::string &Summary ) { 
  return new picasaPhoto( api, fileName, getShortTitle(), Title, Summary );
}




std::ostream &operator<<(std::ostream &out, const picasaAlbum &album) { 
  out << "Title: " << album.getTitle() << " (" <<album.getShortTitle() << ")\n";
  out << "Summary: " << album.getSummary() << "\n";
  out << "Location: " << album.getLocation() << "\n";
  out << "User: " << album.getUser() << "\n";
  out << "AuthKey: " << album.getAuthKey() << "\n";
  return out;
}

