#ifndef _picasaAPI_H
#define _picasaAPI_H

/***************************************************************
 * picasaAPI.h
 * @Author:      Jonathan Verner (jonathan.verner@matfyz.cz)
 * @License:     GPL v2.0 or later
 * @Created:     2009-02-26.
 * @Last Change: 2009-02-26.
 * @Revision:    0.0
 * Description:
 * Usage:
 * TODO:
 *CHANGES:
 ***************************************************************/

#include <string>
#include <list>


class picasaAPI { 	
	private:
		void *curl;
		char *curl_error_buf;
		std::string userName, authToken, appName;

		bool haveToken();

	public:

		std::string GET( const std::string &feedURL );
		std::string DELETE( const std::string &feedURL );
		std::string PUT( const std::string &feedURL, const std::string &data );
		std::string POST( const std::string &feedURL, const std::string &data );

		friend class picasaAlbum;

		picasaAPI( const std::string &user = "", const std::string &password = "", const std::string app = "picasaAPI" );
		~picasaAPI();

		bool login( const std::string &password, const std::string &user = "");
		bool loggedIn() { return haveToken(); };
		std::string getUser() const;

		std::list<std::string> albumList( const std::string &user = "" );

		static int string2int(const std::string &number);

};




#endif /* _picasaAPI_H */
