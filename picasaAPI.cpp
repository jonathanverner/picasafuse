/***************************************************************
 * picasaAPI.cpp
 * @Author:      Jonathan Verner (jonathan.verner@matfyz.cz)
 * @License:     GPL v2.0 or later
 * @Created:     2009-02-27.
 * @Last Change: 2009-02-27.
 * @Revision:    0.0
 * Description:
 * Usage:
 * TODO:
 *CHANGES:
 ***************************************************************/

#include "picasaAPI.h"

#include "curl_util.h"


#ifndef TIXML_USE_TICPP
#define TIXML_USE_TICPP
#endif
#include "ticpp/ticpp.h"

#include <curl/curl.h>

#include <string>

using namespace std;


bool picasaAPI::haveToken() { 
  return (authToken.compare("") != 0);
}

picasaAPI::picasaAPI( const string &user, const string &password, const string app ) : 
	userName( user ), appName(app), authToken(""), curl_error_buf(NULL)
{
  curl = curl_easy_init();
  if ( curl ) { 
    curl_error_buf = new char[ CURL_ERROR_SIZE ];
    curl_easy_setopt( curl, CURLOPT_ERRORBUFFER, curl_error_buf );
  }
  
  login( password );
}

picasaAPI::~picasaAPI() { 
  curl_easy_cleanup(curl);
  delete curl_error_buf;
}

bool picasaAPI::login( const string &password, const string &user ) { 
  if ( password.compare("") == 0 ) return false;
  if ( user.compare("") != 0 ) userName = user;
  authToken = getAuthToken( curl, userName, password, "lh2", appName );
  return haveToken();
}

string picasaAPI::getUser() const { 
  int atPos = userName.find_first_of("@");
  return userName.substr(0,atPos);
}


string picasaAPI::GET( const string &URL ) { 
  return getFeed( curl, URL, authToken );
}

string picasaAPI::DELETE( const string &URL ) { 
  return deleteFeed( curl, URL, authToken );
}


string picasaAPI::PUT( const string &URL, const string &data ) { 
  return postFeed( curl, curlutil_PUT, URL, data, authToken );
}
string picasaAPI::POST( const string &URL, const string &data ) { 
  return postFeed( curl, curlutil_POST, URL, data, authToken );
}


list<string> picasaAPI::albumList( const string &user ) {
  string URL = "http://picasaweb.google.com/data/feed/api/user/"+user;
  list<string> ret;
  if (user.compare("") == 0) URL+=userName;
  string feedXML = GET( URL );
  ticpp::Document xml;
  try {
    xml.Parse( feedXML );
    ticpp::Iterator< ticpp::Element > albumItem("entry");
    for( albumItem = albumItem.begin( xml.FirstChildElement() ); albumItem != albumItem.end(); albumItem++ ) {
      ret.push_back( albumItem->FirstChildElement( "id" )->GetText() );
    }
  } catch ( ticpp::Exception &ex ) { 
    std::cerr << "picasaAPI::albumList (TinyXML++): " << ex.what();
  }
  return ret;
}

int picasaAPI::string2int( const string &number ) { 
  int num=0, sign=1;
  if ( number.size() <= 0 ) return 0;
  if ( number[0] == '-' ) sign=-1;
  for(int i=0;i<number.size();i++) { 
    if ( number[i] < '0' || number[i] > '9' ) return 0;
    num*=10;
    num+=number[i]-'0';
  }
  return num;
}



