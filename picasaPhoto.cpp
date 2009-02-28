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

#ifndef TIXML_USE_TICPP
#define TIXML_USE_TICPP
#endif
#include "ticpp/ticpp.h"


#include "picasaAlbum.h"
#include "picasaAPI.h"

using namespace std;

bool isURL( const std::string &fileName ) { 
  if ( fileName.find( "http://" ) == 0 || fileName.find( "https://" ) == 0 ) return true;
  return false;
}


picasaPhoto::loadFromXML( const ticpp::Document *xmlDoc ) { 
  ticpp::Element *root = xmlDoc->FirstChild();
  for( links = links.begin( root ); links != links.end(); links++ ) {
    if ( links->GetAttribute( "rel" ).compare( "edit-media" ) == 0 )
      editURL = links->GetAttribute( "href" );  
    if ( links->GetAttribute( "rel" ).compare( "alternate" ) == 0 )
      altURL = links->GetAttribute( "href" );
    if ( links->GetAttribute( "rel" ).compare( "self" ) == 0 )
      selfURL = links->GetAttribute( "href" );
  }
  extractAuthKey( editURL );
  extractAuthKey( altURL );
  extractAuthKey( selfURL );
  photoURL = root->FirstChildElement( "content" )->GetAttribute( "src" );
  fileName = root->FirstChildElement( "title" )->GetText(false);
  caption = root->FirstChildElement( "summary" )->GetText(false);
  checkSum = root->FirstChildElement( "gphoto:checksum" )->GetText(false);
  size = picasaAPI::string2int( root->FirstChildElement("gphoto:size")->GetText(false) );
}

picasaPhoto::loadFromXML( const std::string &xmlText ) { 
  xml = new ticpp::Document();
  try { 
    xml->Parse( xmlText );
    loadFromXML( xml );
  } catch ( ticpp::Exception &ex ) { 
    std::cerr << "picasaPhoto::loadFromXML( xmlText ): " << ex.what() << "\n";
    std::cerr << xmlText;
  }
}


picasaPhoto::picasaPhoto( picasaAlbum *Album, const ticpp::Element *xmlEntry ):
	album(Album) 
{
  syncPol = album->syncPol;
  try { 
    xml = new ticpp::Document();
    xml->LinkEndChild( xmlEntry->Clone() );
    loadFromXML( xml );
  } catch ( ticpp::Exception &ex ) { 
    std::cerr << "picasaPhoto::picasaPhoto( ..., xmlEntry ): " << ex.what() << "\n";
  }
}

picasaPhoto::picasaPhoto( picasaAlbum *Album, const std::string &fileNameOrURL ):
	album(Album)
{
  syncPol = album->syncPol;
  if ( isURL( fileNameOrURL ) ) { 
    string URL = fileNameOrURL;
    if ( album->getAccessType() == picasaAlbum::UNLISTED )
      URL+="?authkey="+album->getAuthKey();
    string xmlText = album->getAPI()->GET( URL );
    loadFromXML( xmlText );
  } else { 
    cerr << "picasaPhoto::picasaPhoto(...,"<<fileNameOrURL<<"): Posting new photos not implemented yet.\n";
  }
}

void picasaPhoto::setUpdatePolicy( const enum picasaAlbum::updatePolicy policy ) { 
  syncPol = policy;
}

bool picasaPhoto::modified() { 
  upToDate = false;
  if ( syncPol == AUTO_SYNC ) return update();
  return true;
}

bool picasaPhoto::update() { 
  if ( syncPol == NO_SYNC ) return false;
  if ( upToDate ) return true;
  if ( ! album->getAPI()->loggedIn() ) { 
    cerr<<"picasaAlbum::update: Not logged in!\n";
    return false;
  }
  cerr << "picasaPhoto::update(): Updating not implemented yet; \n";
  return false;
  /*
  ostringstream oss( ostringstream::out );
  oss << *xml;
  loadFromXML(api->PUT( editURL, oss.str() ));
  return true;*/
}
void picasaPhoto::setCaption( const std::string &c ) { 
  caption = c;
  xml->FirstChildElement()->FirstChildElement( "summary" )->SetText( caption );
  return modified();
};

void picasaPhoto::setFileName( const std::string &f ) {
  fileName = f;
  xml->FirstChildElement()->FirstChildElement( "title" )->SetText( fileName );
  return modified();
};

void picasaPhoto::setCheckSum( const std::string &c ) { 
  checkSum = c;
  xml->FirstChildElement()->FirstChildElement( "gphoto:checksum" )->SetText( checkSum );
  return modified();
};

bool picasaPhoto::addComment( const std::string &comment ) {
  if ( ! album->getAPI()->loggedIn() ) { 
    cerr<<"picasaPhoto::addComment: Not logged in!\n";
    return false;
  }
  cerr<<"picasaPhoto::addComment(): Adding comments not implemented yet;\n";
  return false;
}

bool picasaPhoto::deletePhoto() {
  if ( ! album->getAPI()->loggedIn() ) { 
    cerr<<"picasaPhoto::deletePhoto: Not logged in!\n";
    return false;
  }
  string resp = album->getAPI()->DELETE( editURL );
  if ( resp.size() > 0 ) {
    cerr << resp;
    return false;
  } else return true;
}

bool picasaPhoto::uploadPhoto( const std::string &fileName ) { 
  if ( ! album->getAPI()->loggedIn() ) { 
    cerr<<"picasaPhoto::deletePhoto: Not logged in!\n";
    return false;
  }
  cerr<<"picasaPhoto::uploadPhoto(): Uploading photos not implemented yet;\n";
  return false;
}

bool picasaPhoto::downloadPhoto( const std::string &fileName ) { 
  cerr<<"picasaPhoto::downloadPhoto(): Downloading photos not implemented yet;\n";
  return false;
}

