/***************************************************************
 * gAPI.cpp
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

#include "gAPI.h"

#include "curlRequest.h"


#ifndef TIXML_USE_TICPP
#define TIXML_USE_TICPP
#endif
#include "ticpp/ticpp.h"

#include <string>

using namespace std;


bool gAPI::haveToken() { 
  return (authToken.compare("") != 0);
}


void gAPI::dropAuthKey( std::string &URL ) { 
  int authKeyPos = URL.find( "?authkey=" );
  if ( authKeyPos != string::npos ) URL = URL.substr( 0, authKeyPos );
}

string gAPI::extractAuthKey( std::string &URL ) { 
  int authKeyPos = URL.find( "?authkey=" );
  string ret;
  if ( authKeyPos != string::npos ) {
    authKeyPos+=9;
    ret = URL.substr( authKeyPos );
    URL = URL.substr( 0, authKeyPos-9 );
    return ret;
  } else return "";
}

string gAPI::getAuthKey( const std::string &URL ) { 
  int authKeyPos = URL.find( "?authkey=" );
  if ( authKeyPos != string::npos )
    return URL.substr( authKeyPos + 9 );
  else return "";
}

bool gAPI::haveAuthKey( const std::string &URL ) {
  return ( URL.find( "?authkey=" ) != string::npos );
}


gAPI::gAPI( const string &user, const string &password, const string app ) : 
	userName( user ), appName(app), authToken("")
{
  login( password );
}

bool gAPI::login( const string &password, const string &user ) { 
  if ( password.compare("") == 0 ) return false;
  if ( user.compare("") != 0 ) userName = user;
  getAuthToken( password );
  return haveToken();
}

string gAPI::getUser() const { 
  int atPos = userName.find_first_of("@");
  return userName.substr(0,atPos);
}


string gAPI::extractVal( const string response, const string key ) { 
  int tokenStart=response.find(key+"=");
  if ( tokenStart == string::npos ) return "";
  tokenStart += key.size()+1;
  int tokenEnd = response.find_first_of( "\n", tokenStart );
  if ( tokenEnd == string::npos ) return "";
  return response.substr( tokenStart, tokenEnd-tokenStart );
}

void gAPI::getAuthToken(const string& password) { 
  curlRequest request;
  request.setURL( "https://www.google.com/accounts/ClientLogin" );
  request.setBody("Email="+userName+"&Passwd="+password+"&accountType=GOOGLE&source="+appName+"&service=lh2", "application/x-www-form-urlencoded");
  request.setType( curlRequest::POST );
  request.perform();
  if ( request.getStatus() != 200 ) {
    cerr << "gAPI::getAuthToken: "<< extractVal( request.getResponse(), "Error" ) << " (response status "<<request.getStatus() << ")\n";
  } else authToken = extractVal( request.getResponse(), "Auth" );
}


bool gAPI::DOWNLOAD( const std::string &URL, const std::string &fileName ) { 
  curlRequest request;
  if ( haveToken() )  request.addHeader( "Authorization: GoogleLogin auth="+authToken );
  request.setType( curlRequest::GET );
  request.setURL( URL );
  request.setOutFile( fileName );
  if ( ! request.perform() ) return false;
  if ( request.getStatus() != 200 ) {
    cerr << "gAPI::DOWNLOAD: "<<request.getResponse() << "(response status "<<request.getStatus() << ")\n";
    return false;
  }
  return true;
}


string gAPI::GET( const string &URL ) { 
  curlRequest request;
  if ( haveToken() )  request.addHeader( "Authorization: GoogleLogin auth="+authToken );
  request.setType( curlRequest::GET );
  request.setURL( URL );
  request.perform();
  if ( request.getStatus() != OK && request.getStatus() != CREATED && request.getStatus() != NOT_MODIFIED ) {
    cerr << "gAPI::GET: ERROR"<<endl;
    cerr << "   Response status: "<<request.getStatus() << endl;
    cerr << "   Offending URL: " << URL << endl;
    cerr << "   Response---------------------" <<endl;
    cerr << request.getResponse()<<endl;
    return "";
  }
  return request.getResponse();
}

string gAPI::DELETE( const string &URL ) { 
  curlRequest request;
  if ( haveToken() )  request.addHeader( "Authorization: GoogleLogin auth="+authToken );
  request.setType( curlRequest::DELETE );
  request.setURL( URL );
  request.perform();
  return request.getResponse();
}


string gAPI::PUT( const string &URL, const string &data ) { 
  curlRequest request;
  if ( haveToken() )  request.addHeader( "Authorization: GoogleLogin auth="+authToken );
  request.setType( curlRequest::PUT );
  request.setURL( URL );
  request.setBody( data, "application/atom+xml" );
  request.perform();
  if ( request.getStatus() != OK && request.getStatus() != CREATED && request.getStatus() != NOT_MODIFIED ) { 
    cerr << "gAPI::PUT: ERROR"<<endl;
    cerr << "   Response status: "<<request.getStatus() << endl;
    cerr << "   Offending URL: " << URL << endl;
    cerr << "   Response---------------------" <<endl;
    cerr << request.getResponse()<<endl;
    cerr << "   Request Body-----------------" <<endl;
    cerr << data <<endl;
    cerr << "--------------------------------" <<endl;
    return "";
  }
  return request.getResponse();
}

string gAPI::POST( const string &URL, const string &data ) { 
  curlRequest request;
  if ( haveToken() )  request.addHeader( "Authorization: GoogleLogin auth="+authToken );
  request.setType( curlRequest::POST );
  request.setURL( URL );
  request.setBody( data, "application/atom+xml" );
  request.perform();
  if ( request.getStatus() != 200 && request.getStatus() != 201 ) { 
    cerr << "gAPI::PUT: "<<request.getResponse() << "(response status "<<request.getStatus() << ")\n";
    return "";
  }
  return request.getResponse();
}

string gAPI::POST_FILE( const string &URL, const string &file, const string &contentType, list<string> &headers ) { 
 curlRequest request;
  if ( haveToken() )  request.addHeader( "Authorization: GoogleLogin auth="+authToken );
  request.setType( curlRequest::POST );
  request.setURL( URL );
  request.setInFile( file, contentType );
  for( list<string>::iterator hdr = headers.begin(); hdr != headers.end(); hdr++ )
    request.addHeader( *hdr );
  request.perform();
  if ( request.getStatus() != 200 && request.getStatus() != 201 ) { 
    cerr << "gAPI::POST_FILE: "<<request.getResponse() << " (response status "<<request.getStatus() << ")\n";
    cerr << "gAPI::POST_FILE to " << URL << " of " << file << " ("<<contentType<<")\n";
    return "";
  }
  return request.getResponse();
}



set<string> gAPI::albumList( const string &user ) throw (enum gAPI::exceptionType ) {
  string URL = "http://picasaweb.google.com/data/feed/api/user/"+user;
  set<string> ret;
  if (user.compare("") == 0) URL+=userName;
  string feedXML = GET( URL );
  ticpp::Document xml;
  try {
    xml.Parse( feedXML );
    ticpp::Iterator< ticpp::Element > albumItem("entry");
    for( albumItem = albumItem.begin( xml.FirstChildElement() ); albumItem != albumItem.end(); albumItem++ ) {
      ret.insert( albumItem->FirstChildElement( "title" )->GetText() );
    }
  } catch ( ticpp::Exception &ex ) { 
    throw GENERAL_ERROR;
  }
  return ret;
}

int gAPI::string2int( const string &number ) { 
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



