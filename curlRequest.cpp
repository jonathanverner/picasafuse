/***************************************************************
 * curlRequest.cpp
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


#include "curlRequest.h"
 
#include <curl/curl.h>

#include <stdio.h>
#include <iostream>

using namespace std;

map<boost::thread::id, void*> curlRequest::curl_handles;
int curlRequest::handles_count = 0;


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
/*  cerr<<"requestData(...,"<<size<<","<<nmemb<<",...)\n";
  cerr<<"  availableData:"<<availableData<<"\n";
  cerr<<"  copySize:"<<copySize<<"\n";
  cerr<<"  pos:"<<data->pos<<"\n";
  cerr<<"  totalDataSize:"<<data->size<<"\n"; */
  data->data.copy( (char *) ptr, copySize, data->pos );
  data->pos+=copySize;
  return copySize;
}




curlRequest::curlRequest(): 
	request( GET ), body(""), URL(""), status(-1),
	response(""), outFile(""),network_down(false)
{
}

void *curlRequest::getThreadCurlHandle() { 
  void *ret;
  boost::thread::id tID = boost::this_thread::get_id();
  if ( curl_handles.find( tID ) == curl_handles.end() ) { 
    ret = curl_easy_init();
    handles_count++;
    if ( ret )
      curl_handles[tID] = ret;
    return ret;
  } else {
    return curl_handles[tID];
  }
}

bool curlRequest::checkNetworkConnection() {
  string temp;
  void *curl = getThreadCurlHandle();
  curl_easy_reset( curl );
  curl_easy_setopt( curl, CURLOPT_URL, "http://www.google.com" );
  curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, responseData );
  curl_easy_setopt( curl, CURLOPT_WRITEDATA, &temp );
  CURLcode cd = curl_easy_perform(curl);
  if ( cd ) {
    cerr << "networkUP(): NO NETWORK CONNECTION (" << cd <<")\n";
    network_down = true;
  } else network_down = false;
  return network_down;
}

bool curlRequest::perform() throw (enum exceptionType) {
  cerr<<"Performing request for " << URL << endl;
  void *curl = getThreadCurlHandle();
  if ( ! getThreadCurlHandle() ) { 
    cerr << "getFeed() Error: INVALID CURL HANDLE\n";
    return false;
  }
  curl_easy_reset( curl );
  FILE *outfl = NULL, *infl=NULL;

  struct curl_slist *curlHDRS = NULL;
  for( list<string>::iterator hdr = headers.begin(); hdr != headers.end(); hdr++ ) {
    curlHDRS = curl_slist_append( curlHDRS, hdr->c_str() );
  }
  if ( inFile.compare("") != 0 )
    curlHDRS = curl_slist_append( curlHDRS, "Transfer-Encoding: chunked");

  curl_easy_setopt( curl, CURLOPT_URL, URL.c_str() );
  curl_easy_setopt( curl, CURLOPT_HTTPHEADER, curlHDRS );

  if ( outFile.compare("") == 0 ) { 
    curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, responseData );
    curl_easy_setopt( curl, CURLOPT_WRITEDATA, &response );
  } else { 
    outfl = fopen( outFile.c_str(), "w" );
    if ( outfl == NULL ) { 
      cerr << "curlRequest::perform(): cannot open file '"<<outFile<<"' for writing.";
      return false;
    }
    curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, NULL );
    curl_easy_setopt( curl, CURLOPT_WRITEDATA, outfl );
  }

  struct requestData bodyData;
  bodyData.data = body;
  bodyData.size = body.size();
  bodyData.pos=0;
  string method="DELETE";
  

  switch( request ) { 
	  case GET:
		  curl_easy_setopt( curl, CURLOPT_HTTPGET, 1 );
		  break;
	  case POST:
		  curl_easy_setopt( curl, CURLOPT_POST, 1 );
		  if ( inFile.compare("") == 0 ) { 
		    curl_easy_setopt( curl, CURLOPT_POSTFIELDSIZE, body.size() );
		    curl_easy_setopt( curl, CURLOPT_POSTFIELDS, body.c_str() );
		  } else {
		    infl = fopen( inFile.c_str(), "r" );
		    if ( infl == NULL ) { 
		      cerr << "curlRequest::perform(): cannot open file '"<<inFile<<"' for reading.";
		      return false;
		    }
		    method = "POST";
		    curl_easy_setopt( curl, CURLOPT_UPLOAD, 1 );
		    curl_easy_setopt( curl, CURLOPT_CUSTOMREQUEST, method.c_str() );
		    curl_easy_setopt( curl, CURLOPT_READFUNCTION, NULL );
		    curl_easy_setopt( curl, CURLOPT_READDATA, infl );
		  }
		  break;
	  case PUT:
		  method = "PUT";
		  //curl_easy_setopt( curl, CURLOPT_PUT, 1 );
		  curl_easy_setopt( curl, CURLOPT_CUSTOMREQUEST, method.c_str() );
		  curl_easy_setopt( curl, CURLOPT_READFUNCTION, requestData );
		  curl_easy_setopt( curl, CURLOPT_READDATA, &bodyData );
		  curl_easy_setopt( curl, CURLOPT_UPLOAD, 1 );
		  curl_easy_setopt( curl, CURLOPT_INFILESIZE, body.size() );
		  break;
	  case DELETE:
		  curl_easy_setopt( curl, CURLOPT_CUSTOMREQUEST, method.c_str() );
		  break;
	  case MULTIPART_POST:
		  cerr << "MULTIPART POST NOT IMPLEMENTED YET";
		  if ( outfl ) fclose( outfl );
		  if ( infl ) fclose( infl );
		  curl_slist_free_all( curlHDRS );
		  return false;
  }
  CURLcode cd = curl_easy_perform(curl);
  cerr<<"DONE Performing request for " << URL <<endl;
  if ( cd ) {
      if ( outfl ) fclose( outfl );
      if ( infl ) fclose( infl );
      curl_slist_free_all( curlHDRS );
      cerr << "getFeed() Error: CURL ERROR IN OPERATION(" <<cd <<") \n";
      switch ( cd ) {
	case CURLE_COULDNT_CONNECT:
	case CURLE_COULDNT_RESOLVE_HOST:
	case CURLE_OPERATION_TIMEDOUT:
	case CURLE_SEND_ERROR:
	case CURLE_RECV_ERROR:
	  network_down = true;
	  throw NO_NETWORK_CONNECTION;
      }
      return false;
  }
  network_down=false;
  curl_easy_getinfo( curl, CURLINFO_RESPONSE_CODE, &status );
  if ( outfl ) fclose( outfl ); 
  if ( infl ) fclose( infl );
  curl_slist_free_all( curlHDRS );
};


