#ifndef _curlRequest_H
#define _curlRequest_H

/***************************************************************
 * curlRequest.h
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

#include <string>
#include <list>




class curlRequest { 
	public:
		enum requestType { GET, POST, PUT, DELETE, MULTIPART_POST };

	private:
		void *curl;
		enum requestType request;
		std::string URL, body, postFields, outFile, inFile;
		std::list< std::string > headers;

		std::string response;
		int status;

	public:
		curlRequest(void *curl);

		void addHeader( const std::string &header ){ headers.push_back(header);};
		void setBody( const std::string &Body, const std::string contentType="application/atom+xml" ) { body=Body; headers.push_back("Content-Type: "+contentType); };
		void setURL( const std::string &url ) { URL = url; };
		void setType( const enum requestType reqType ) { request=reqType; };
		void setOutFile( const std::string &fileName ) { outFile = fileName; };
		void setInFile( const std::string &fileName, const std::string contentType ){ inFile=fileName; headers.push_back("Content-Type: "+contentType);};

		bool perform();

		std::string getResponse() { return response; }
		int getStatus() { return status; };
};


#endif /* _curlRequest_H */
