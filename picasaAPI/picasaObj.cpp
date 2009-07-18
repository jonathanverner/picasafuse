/***************************************************************
 * picasaObj.cpp
 * @Author:      Jonathan Verner (jonathan.verner@matfyz.cz)
 * @License:     GPL v2.0 or later
 * @Created:     2009-03-18.
 * @Last Change: 2009-03-18.
 * @Revision:    0.0
 * Description:
 * Usage:
 * TODO:
 *CHANGES:
 ***************************************************************/
 
#include "picasaObj.h"

#ifndef TIXML_USE_TICPP
#define TIXML_USE_TICPP
#endif
#include "ticpp/ticpp.h"

#include "gAPI.h"

#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


using namespace std;

void picasaObj::markModified() { 
  needsSync = true;
}

picasaObj::picasaObj( gAPI *API, string cacheR ):
	api( API ), readOnly( true ), cacheFileName(""), cacheRoot(cacheR),
	createURL(""), editURL(""), selfURL(""), xml(NULL), needsSync(false) 
{
}

picasaObj::picasaObj( string user, string pass, string cacheR ):
	readOnly( true ), cacheFileName(""), cacheRoot(cacheR), 
	createURL(""), selfURL(""), editURL(""), xml(NULL), needsSync(false)
{ 
 api = new gAPI( user, pass );
}



bool picasaObj::fileExists( const string &fName ) { 
  struct stat st;
  int ret = stat( fName.c_str(), &st );
  if ( ret == -1 ) return false;
  else return S_ISREG(st.st_mode);
}

bool picasaObj::mkCacheDir( const string &dirName ) { 
  if ( dirName.find("../") != string::npos || dirName.find("/..") != string::npos ) return false;
  string dir = cacheRoot+"/"+dirName;
  return ( mkdir( dir.c_str(), 0777 ) == 0 );
}


bool picasaObj::loadFromXML( const string &xmlText ) { 
  try { 
    xml = new ticpp::Document();
    xml->Parse( xmlText );
    loadFromXML( xml );
    readOnly = false;
    needsSync = false;
    return true;
  } catch ( ticpp::Exception &ex ) { 
    readOnly = true;
    std::cerr << "picasaObj::loadFromXML( string ): " << ex.what() << "\n";
    std::cerr << xmlText << "\n";
    return false;
  }
}


bool picasaObj::loadFromXMLFile( const string &xmlFile ) { 
  try { 
    xml = new ticpp::Document( xmlFile );
    loadFromXML( xml );
    readOnly = false;
    needsSync = false;
    return true;
  } catch ( ticpp::Exception &ex ) { 
    readOnly = true;
    std::cerr << "picasaObj::loadFromXMLFile("<<xmlFile<<"): " << ex.what() << "\n";
    return false;
  }
};

bool picasaObj::loadFromCache() { 
  if ( isCached() ) { 
    return loadFromXMLFile( cacheRoot+"/"+cacheFileName );
  } else return false;
}

bool picasaObj::saveToCache() {
  if ( cacheFileName.compare("") == 0 || xml == NULL || readOnly ) return false;
  xml->SaveFile( cacheRoot+"/"+cacheFileName );
  return true;
}

bool picasaObj::loadFromPicasa() { 
  return loadFromXML( api->GET( selfURL ) );
}

bool picasaObj::saveToPicasa() { 
  if ( readOnly || ! api->loggedIn() ) return false;
  if ( ! needsSync ) return true;
  ostringstream oss( ostringstream::out );
  oss << *xml;
  return loadFromXML(api->PUT( editURL, oss.str() ));
}

bool picasaObj::createObj() { 
  if ( readOnly || ! api->loggedIn() ) return false;
  ostringstream oss( ostringstream::out );
  oss << *xml;
  return loadFromXML(api->POST( createURL, oss.str() ));
}

bool picasaObj::deleteObj() { 
  if ( readOnly || ! api->loggedIn() ) return false;
  string resp = api->DELETE( editURL );
  if ( resp.size() > 0 ) {
    cerr << resp;
    return false;
  } else return true;
}

bool picasaObj::load( enum cachePolicy pol ) { 
  switch( pol ) { 
	  case TRY_CACHE_FIRST:
		  if ( ! loadFromCache() ) return loadFromPicasa();
		  else return true;
	  case TRY_PICASA_FIRST:
		  if ( ! loadFromPicasa() ) return loadFromCache();
		  else return true;
	  case ONLY_CACHE:
		  return loadFromCache();
	  case ONLY_PICASA:
		  return loadFromPicasa();
  }
}

bool picasaObj::save( enum cachePolicy pol ) { 
  switch( pol ) { 
	  case TRY_CACHE_FIRST:
		  if ( ! saveToCache() ) return saveToPicasa();
		  else return true;
	  case TRY_PICASA_FIRST:
		  if ( ! saveToPicasa() ) return saveToCache();
		  else return true;
	  case ONLY_CACHE:
		  return saveToCache();
	  case ONLY_PICASA:
		  return saveToPicasa();
  }
}

bool picasaObj::isCached() { 
  return fileExists( cacheRoot+"/"+cacheFileName );
}




