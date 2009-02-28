#ifndef _curl_util_H
#define _curl_util_H

/***************************************************************
 * curl_util.h
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

enum postType { curlutil_PUT, curlutil_GET, curlutil_POST };

std::string deleteFeed( void *curl, const std::string &feedURL, const std::string authToken = "" );
std::string getFeed( void *curl, const std::string &feedURL, const std::string authToken = "" );
std::string postFeed( void *curl, const enum postType type, const std::string &feedURL, const std::string &data, const std::string authToken = "" );
std::string getAuthToken( void *curl, const std::string &email, const std::string &password, const std::string service = "lh2", const std::string appName = "testCURL" );

#endif /* _curl_util_H */
