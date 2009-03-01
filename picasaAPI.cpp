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


void picasaAPI::dropAuthKey( std::string &URL ) { 
  int authKeyPos = URL.find( "?authkey=" );
  if ( authKeyPos != string::npos ) URL = URL.substr( 0, authKeyPos );
}

string picasaAPI::extractAuthKey( std::string &URL ) { 
  int authKeyPos = URL.find( "?authkey=" );
  string ret;
  if ( authKeyPos != string::npos ) {
    authKeyPos+=9;
    ret = URL.substr( authKeyPos );
    URL = URL.substr( 0, authKeyPos-9 );
    return ret;
  } else return "";
}

string picasaAPI::getAuthKey( const std::string &URL ) { 
  int authKeyPos = URL.find( "?authkey=" );
  if ( authKeyPos != string::npos )
    return URL.substr( authKeyPos + 9 );
  else return "";
}

bool picasaAPI::haveAuthKey( const std::string &URL ) {
  return ( URL.find( "?authkey=" ) != string::npos );
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
  //authToken = getAuthToken( curl, userName, password, "lh2", appName );
  getAuthToken( password );
  return haveToken();
}

string picasaAPI::getUser() const { 
  int atPos = userName.find_first_of("@");
  return userName.substr(0,atPos);
}


string picasaAPI::extractVal( const string response, const string key ) { 
  int tokenStart=response.find(key+"=");
  if ( tokenStart == string::npos ) return "";
  tokenStart += key.size()+1;
  int tokenEnd = response.find_first_of( "\n", tokenStart );
  if ( tokenEnd == string::npos ) return "";
  return response.substr( tokenStart, tokenEnd-tokenStart );
}

void picasaAPI::getAuthToken(const string& password) { 
  curlRequest request( curl );
  request.setURL( "https://www.google.com/accounts/ClientLogin" );
  request.setBody("Email="+userName+"&Passwd="+password+"&accountType=GOOGLE&source="+appName+"&service=lh2", "application/x-www-form-urlencoded");
  request.setType( curlRequest::POST );
  request.perform();
  if ( request.getStatus() != 200 ) {
    cerr << "picasaAPI::getAuthToken: "<< extractVal( request.getResponse(), "Error" ) << " (response status "<<request.getStatus() << ")\n";
  } else authToken = extractVal( request.getResponse(), "Auth" );
}


bool picasaAPI::DOWNLOAD( const std::string &URL, const std::string &fileName ) { 
  curlRequest request( curl );
  if ( haveToken() )  request.addHeader( "Authorization: GoogleLogin auth="+authToken );
  request.setType( curlRequest::GET );
  request.setURL( URL );
  request.setOutFile( fileName );
  if ( ! request.perform() ) return false;
  if ( request.getStatus() != 200 ) {
    cerr << "picasaAPI::DOWNLOAD: "<<request.getResponse() << "(response status "<<request.getStatus() << ")\n";
    return false;
  }
  return true;
}


string picasaAPI::GET( const string &URL ) { 

  curlRequest request( curl );
  if ( haveToken() )  request.addHeader( "Authorization: GoogleLogin auth="+authToken );
  request.setType( curlRequest::GET );
  request.setURL( URL );
  request.perform();
  if ( request.getStatus() != 200 ) {
    cerr << "picasaAPI::GET: "<<request.getResponse() << "(response status "<<request.getStatus() << ")\n";
    return "";
  }
  return request.getResponse();
}

string picasaAPI::DELETE( const string &URL ) { 
  curlRequest request( curl );
  if ( haveToken() )  request.addHeader( "Authorization: GoogleLogin auth="+authToken );
  request.setType( curlRequest::DELETE );
  request.setURL( URL );
  request.perform();
  return request.getResponse();
}


string picasaAPI::PUT( const string &URL, const string &data ) { 
  curlRequest request( curl );
  if ( haveToken() )  request.addHeader( "Authorization: GoogleLogin auth="+authToken );
  request.setType( curlRequest::PUT );
  request.setURL( URL );
  request.setBody( data, "application/atom+xml" );
  request.perform();
  if ( request.getStatus() != 200 && request.getStatus() != 201 ) { 
    cerr << "picasaAPI::PUT: "<<request.getResponse() << "(response status "<<request.getStatus() << ")\n";
    return "";
  }
  return request.getResponse();
}

string picasaAPI::POST( const string &URL, const string &data ) { 
  curlRequest request( curl );
  if ( haveToken() )  request.addHeader( "Authorization: GoogleLogin auth="+authToken );
  request.setType( curlRequest::POST );
  request.setURL( URL );
  request.setBody( data, "application/atom+xml" );
  request.perform();
  if ( request.getStatus() != 200 && request.getStatus() != 201 ) { 
    cerr << "picasaAPI::PUT: "<<request.getResponse() << "(response status "<<request.getStatus() << ")\n";
    return "";
  }
  return request.getResponse();
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



