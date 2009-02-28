/***************************************************************
 * curl_util.cpp
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


#include "curl_util.h"
 
#include <curl/curl.h>

#include <iostream>
#include <string>

using namespace std;

static size_t responseData(void *ptr, size_t size, size_t nmemb, void *data ) { 
  string *responseBuf = static_cast<string *>(data);
  responseBuf->append( (char *) ptr, size*nmemb );
  return size*nmemb;
}

struct requestData { 
  string data;
  size_t size,pos;
};

static size_t requestData(void *ptr, size_t size, size_t nmemb, void *Data ) { 
  struct requestData *data = (struct requestData *) Data;
  size_t availableData = data->size-data->pos, bufSize = size*nmemb, copySize;
  if ( availableData > bufSize ) copySize = bufSize;
  else copySize = availableData;
  cerr<<"requestData(...,"<<size<<","<<nmemb<<",...)\n";
  cerr<<"  availableData:"<<availableData<<"\n";
  cerr<<"  copySize:"<<copySize<<"\n";
  cerr<<"  pos:"<<data->pos<<"\n";
  cerr<<"  totalDataSize:"<<data->size<<"\n";
  data->data.copy( (char *) ptr, copySize, data->pos );
  data->pos+=copySize;
  return copySize;
}


string extractKey( const string &response, const string &key ) { 
  int tokenStart=response.find(key+"=");
  if ( tokenStart == string::npos ) return "";
  tokenStart += key.size()+1;
  int tokenEnd = response.find_first_of( "\n", tokenStart );
  if ( tokenEnd == string::npos ) return "";
  return response.substr( tokenStart, tokenEnd-tokenStart );
}

string deleteFeed( CURL *curl, const string &feedURL, const string authToken ) { 
  struct curl_slist *headers = NULL;
  string response="";
  string hdr="";
  string method="DELETE";
  long status;

  if ( authToken.compare("") != 0 ) { 
      hdr="Authorization: GoogleLogin auth="+authToken;
  }
  if ( curl ) { 
    headers = curl_slist_append( headers, hdr.c_str() );
    curl_easy_setopt( curl, CURLOPT_URL, feedURL.c_str() );
    curl_easy_setopt( curl, CURLOPT_HTTPHEADER, headers );
    curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, responseData );
    curl_easy_setopt( curl, CURLOPT_WRITEDATA, &response );
    curl_easy_setopt( curl, CURLOPT_CUSTOMREQUEST, method.c_str() );
    if ( curl_easy_perform(curl) ) {
      cerr << "getFeed() Error: CURL ERROR IN OPERATION\n";
      return "";
    }
  } else { 
    cerr << "getFeed() Error: INVALID CURL HANDLE\n";
    return "";
  }

  curl_easy_getinfo( curl, CURLINFO_RESPONSE_CODE, &status );

  if ( status != 200 ) { 
    cerr << "getFeed() Error: " << response << "\n";
    return "";
  } else { 
    return response;
  }
}

string getFeed( CURL *curl, const string &feedURL, const string authToken ) { 
  struct curl_slist *headers = NULL;
  string response="";
  string hdr="";
  long status;

  if ( authToken.compare("") != 0 ) { 
      hdr="Authorization: GoogleLogin auth="+authToken;
  }
  if ( curl ) { 
    headers = curl_slist_append( headers, hdr.c_str() );
    curl_easy_setopt( curl, CURLOPT_URL, feedURL.c_str() );
    curl_easy_setopt( curl, CURLOPT_HTTPHEADER, headers );
    curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, responseData );
    curl_easy_setopt( curl, CURLOPT_WRITEDATA, &response );
    curl_easy_setopt( curl, CURLOPT_HTTPGET, 1 );
    if ( curl_easy_perform(curl) ) {
      cerr << "getFeed() Error: CURL ERROR IN OPERATION\n";
      return "";
    }
  } else { 
    cerr << "getFeed() Error: INVALID CURL HANDLE\n";
    return "";
  }

  curl_easy_getinfo( curl, CURLINFO_RESPONSE_CODE, &status );

  if ( status != 200 ) { 
    cerr << "getFeed() Error: " << response << "\n";
    return "";
  } else { 
    return response;
  }
}

string postFeed( CURL *curl, enum postType type, const string &feedURL, const string &data, const string authToken ) { 
  struct curl_slist *headers = NULL;
  string response="";
  string hdr="";
  long status;
  struct requestData request;
  request.data = data;
  request.size = data.size();
  request.pos=0;
  cerr<<"postFeed(...): size= " << data.size() <<"\n";

  if ( authToken.compare("") != 0 ) { 
      hdr="Authorization: GoogleLogin auth="+authToken;
  }
  if ( curl ) { 
    headers = curl_slist_append( headers, hdr.c_str() );
    hdr="Content-Type: application/atom+xml";
    headers = curl_slist_append( headers, hdr.c_str() );
    curl_easy_setopt( curl, CURLOPT_VERBOSE, 1 );
    curl_easy_setopt( curl, CURLOPT_URL, feedURL.c_str() );
    curl_easy_setopt( curl, CURLOPT_HTTPHEADER, headers );
    curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, responseData );
    curl_easy_setopt( curl, CURLOPT_WRITEDATA, &response );
     if ( type == curlutil_PUT ) {
      curl_easy_setopt( curl, CURLOPT_READFUNCTION, requestData );
      curl_easy_setopt( curl, CURLOPT_READDATA, &request );
      curl_easy_setopt( curl, CURLOPT_UPLOAD, 1 );
      curl_easy_setopt( curl, CURLOPT_INFILESIZE, data.size() );
     } else {
       curl_easy_setopt( curl, CURLOPT_POST, 1 );
       curl_easy_setopt( curl, CURLOPT_POSTFIELDSIZE, data.size() );
       curl_easy_setopt( curl, CURLOPT_POSTFIELDS, data.c_str() );
     }
    if ( curl_easy_perform(curl) ) {
      cerr << "putFeed() Error: CURL ERROR IN OPERATION\n";
      return "";
    }
  } else { 
    cerr << "putFeed() Error: INVALID CURL HANDLE\n";
    return "";
  }

  curl_easy_getinfo( curl, CURLINFO_RESPONSE_CODE, &status );

  if ( status != 200 && status != 201) { 
    cerr << "postFeed() Error: " << response << "\n";
    return "";
  } else { 
    return response;
  }
}


string getAuthToken( CURL *curl, const string &email, const string &password, const string service, const string appName ) { 
  string response="", postFields="Email="+email+"&Passwd="+password+"&accountType=GOOGLE&source="+appName+"&service="+service;
  CURLcode res;
  long status;

  if(curl) {
    curl_easy_setopt( curl, CURLOPT_URL, "https://www.google.com/accounts/ClientLogin" );
    curl_easy_setopt( curl, CURLOPT_POSTFIELDS, postFields.c_str() );
    curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, responseData );
    curl_easy_setopt( curl, CURLOPT_WRITEDATA, &response );
    if ( curl_easy_perform(curl) ) {
      cerr << "CURL ERROR IN OPERATION\n";
      return "";
    }
  } else { 
    cerr << "INVALID CURL HANDLE\n";
    return "";
  }

  curl_easy_getinfo( curl, CURLINFO_RESPONSE_CODE, &status );

  if ( status != 200 ) { 
    cerr << "Error:" << extractKey( response, "Error" )<<"\n";
    return "";
  } else { 
    return extractKey( response, "Auth" );
  }
}


