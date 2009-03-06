/***************************************************************
 * picasaAlbum.cpp
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

#ifndef TIXML_USE_TICPP
#define TIXML_USE_TICPP
#endif
#include "ticpp/ticpp.h"
#include "picasaAPI.h" 

#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


using namespace std;


bool picasaAlbum::isCached() { 
  if ( ! cacheEnabled ) return false;

  struct stat st;
  int ret = stat( cacheFileName.c_str(), &st );
  if ( ret == -1 ) return false;
  else return S_ISREG(st.st_mode);
}

void picasaAlbum::loadFromCache() { 
  if ( isCached() )
    loadFromXMLFile( cacheFileName );
}

void picasaAlbum::saveToCache() { 
  if ( !cacheEnabled ) return;
  xml->SaveFile( cacheFileName );
}
  

void picasaAlbum::setCachePath( std::string cacheMountPoint ) { 
  cacheFileName = cacheMountPoint+"/"+getUser()+"/"+shortTitle+"/.cache";
  cacheEnabled = true;
}


picasaAlbum::picasaAlbum( picasaAPI *API, string cMPNT, string user, string aName ):
	api(API), shortTitle( aName )
{
  selfURL = "http://picasaweb.google.com/data/entry/api/user/"+user+"/album/"+shortTitle;
  setCachePath( cMPNT );
  if ( isCached() ) {
    loadFromCache();
  } else { 
    authKey = extractAuthKey( selfURL );
    if ( authKey.compare("") == 0 ) access = PUBLIC;
    else access = UNLISTED;
    loadFromAtom( selfURL );
  }
}; 

void picasaAlbum::loadFromAtom( const std::string &feedURL ) {
  string URL = feedURL;
  if ( access == UNLISTED ) URL+="?authkey="+authKey;
  loadFromXML( api->GET( URL ) );
}



std::string picasaAlbum::getUser() const { 
  int userStart = selfURL.find("user/")+5;
  int userEnd = selfURL.find_first_of("/",userStart);
  return selfURL.substr(userStart,userEnd-userStart);
}

std::string picasaAlbum::getAlbumId() const { 
  int idStart = selfURL.find("/albumid/")+9;
  int idEnd = selfURL.find_first_not_of("0123456789",idStart);
  return selfURL.substr(idStart,idEnd-idStart);
}

void picasaAlbum::loadFromTicppXML( ticpp::Document *doc ) {
  xml = doc;
  try { 
    ticpp::Element *root = xml->FirstChildElement();
    ticpp::Iterator< ticpp::Element > links("link");
    for( links = links.begin( root ); links != links.end(); links++ ) {
      if ( links->GetAttribute( "rel" ).compare("edit") == 0 )
	editURL = links->GetAttribute( "href" );
      if ( links->GetAttribute( "rel" ).compare("alternate") == 0 )
	altURL = links->GetAttribute( "href" );
    }
    authKey = extractAuthKey( altURL );
    if ( authKey.compare("") == 0 ) access = PUBLIC;
    else access = UNLISTED;
    title = root->FirstChildElement("title")->GetText(false);
    description = root->FirstChildElement("summary")->GetText(false);
    pubDate = root->FirstChildElement("published")->GetText(false);
    modDate = root->FirstChildElement("updated")->GetText(false);
    int slashPos = altURL.find_last_of("/");
    shortTitle = altURL.substr( slashPos+1 );
    upToDate = true;
  } catch ( ticpp::Exception &ex ) { 
    syncPol = NO_SYNC;
    std::cerr << "picasaAlbum::loadFromTicppXML(...): " << ex.what() << "\n";
  }
};


void picasaAlbum::loadFromXML( const std::string &xmlText ) { 
  xml = new ticpp::Document();
  try { 
    xml->Parse( xmlText );
    loadFromTicppXML( xml );
  } catch ( ticpp::Exception &ex ) { 
    syncPol = NO_SYNC;
    std::cerr << "picasaAlbum::loadFromXML(...): " << ex.what() << "\n";
    std::cerr << xmlText << "\n";
  }
};

void picasaAlbum::loadFromXMLFile( const std::string &xmlFile ) { 
  try { 
    xml = new ticpp::Document( xmlFile );
    loadFromTicppXML( xml );
  } catch ( ticpp::Exception &ex ) { 
    syncPol = NO_SYNC;
    std::cerr << "picasaAlbum::loadFromXMLFile("<<xmlFile<<"): " << ex.what() << "\n";
  }
};


string picasaAlbum::extractAuthKey( std::string &URL ) { 
  int authKeyPos = URL.find( "?authkey=" );
  string ret;
  if ( authKeyPos != string::npos ) {
    authKeyPos+=9;
    ret = URL.substr( authKeyPos );
    URL = URL.substr( 0, authKeyPos-9 );
    return ret;
  } else return authKey;
}

string picasaAlbum::htmlFeedURL2AtomFeedURL( const std::string &feedURL ) {
  string URL = feedURL;
  if ( access == UNLISTED ) URL+="?authkey="+authKey;
  string html = api->GET( URL );
  int aIDStart = html.find("albumid/");
  if ( aIDStart == string::npos ) return "";
  else aIDStart+=8;
  int aIDEnd = html.find_first_not_of("0123456789",aIDStart);
  string albumID = html.substr( aIDStart, aIDEnd-aIDStart );

  aIDEnd = feedURL.find_last_of("/");
  aIDStart = feedURL.find_last_of("/",aIDEnd-1);
  if ( aIDEnd == string::npos || aIDStart == string::npos ) return "";
  string userName = feedURL.substr(aIDStart+1, aIDEnd-aIDStart-1);
  string ret = "http://picasaweb.google.com/data/entry/api/user/"+userName+"/albumid/"+albumID;
  return ret;
}
  
enum picasaAlbum::feedType picasaAlbum::guessFeedType( const std::string &feedURL ) { 
  if ( feedURL.find("://picasaweb.google") == string::npos ) return picasaAlbum::NEW_ALBUM_TITLE;
  if ( feedURL.find("/data/entry/api/user/") == string::npos ) return picasaAlbum::HTML_FEED;
  return picasaAlbum::ATOM_FEED;
}

picasaAlbum::picasaAlbum( picasaAPI *API, std::string URL, enum feedType urlType, std::string authKEY ):
	api(API), selfURL(URL), upToDate( false ), readOnly( true ),
	syncPol( AUTO_SYNC ), authKey(authKEY), cacheEnabled( false ),
	cacheFileName("")
{ 
  authKey = extractAuthKey( selfURL );
  if ( authKey.compare("") == 0 ) access = PUBLIC;
  else access = UNLISTED;
  if ( urlType == AUTO_GUESS ) urlType = guessFeedType( selfURL );
  switch( urlType ) { 
	  case ATOM_FEED:
		  loadFromAtom( selfURL );
		  break;
	  case HTML_FEED:
		  selfURL = htmlFeedURL2AtomFeedURL( selfURL );
		  loadFromAtom( selfURL );
		  break;
	  case NEW_ALBUM_TITLE:
		  newAlbum( URL );
		  break;
  }
}


void picasaAlbum::newAlbum( const std::string &albumTitle ) {
  if ( ! api->loggedIn() ) { 
    syncPol = NO_SYNC;
    cerr<<"Create album("<<albumTitle<<"): Not logged in!\n";
  } else {
    string postURL = "http://picasaweb.google.com/data/feed/api/user/"+api->getUser();
    string xml = "<entry xmlns='http://www.w3.org/2005/Atom'  \
		  xmlns:media='http://search.yahoo.com/mrss/' \
		  xmlns:gphoto='http://schemas.google.com/photos/2007'> \
		    <title type='text'>"+albumTitle+"</title> \
		    <summary type='text'></summary> \
		    <gphoto:location></gphoto:location> \
		    <gphoto:access>public</gphoto:access> \
		    <gphoto:commentingEnabled>true</gphoto:commentingEnabled> \
		    <media:group> \
		        <media:keywords></media:keywords> \
		    </media:group> \
		    <category scheme='http://schemas.google.com/g/2005#kind' term='http://schemas.google.com/photos/2007#album'></category> \
		  </entry>";
    loadFromXML( api->POST( postURL, xml ) );
  }
}

picasaAlbum::~picasaAlbum() { 
  if ( ! upToDate && ( syncPol == AUTO_SYNC || syncPol == SEMI_AUTO_SYNC ) ) update();
}

void picasaAlbum::setUpdatePolicy( const enum updatePolicy policy ) { 
  syncPol = policy;
}

bool picasaAlbum::update() { 
  if ( syncPol == NO_SYNC ) return false;
  if ( upToDate ) return true;
  if ( ! api->loggedIn() ) { 
    cerr<<"picasaAlbum::update: Not logged in!\n";
    return false;
  }
  ostringstream oss( ostringstream::out );
  oss << *xml;
  loadFromXML(api->PUT( editURL, oss.str() ));
  return true;
}

bool picasaAlbum::reload() { 
  loadFromAtom( selfURL );
};

bool picasaAlbum::deleteAlbum() { 
  if ( ! api->loggedIn() ) { 
    cerr<<"picasaAlbum::deleteAlbum: Not logged in!\n";
    return false;
  }
  string resp = api->DELETE( editURL );
  if ( resp.size() > 0 ) {
    cerr << resp;
    return false;
  } else return true;
}

bool picasaAlbum::modified() { 
  upToDate = false;
  if ( syncPol == AUTO_SYNC ) return update();
  return true;
}

bool picasaAlbum::setTitle( string t ) { 
  title = t;
  xml->FirstChildElement()->FirstChildElement("title")->SetText(title);
  return modified();
}

bool picasaAlbum::setPubDate( string d ) { 
  pubDate = d;
  return modified();
}

bool picasaAlbum::setDescription( string d ) { 
  description = d;
  xml->FirstChildElement()->FirstChildElement("summary")->SetText(description);
  return modified();
}

ostream & operator <<( ostream &out, picasaAlbum a ) { 
  return out << "Album Title: " << a.title << "\nAlbum URL: " << a.altURL << "\nAlbum Description: " <<a.description << "\nPublished: " << a.pubDate << "\nUpdated: "<< a.modDate <<"\n" << "Auth Key: " << a.authKey << "\n";
}





  
	

