#ifndef _atomObj_H
#define _atomObj_H

/***************************************************************
 * atomObj.h
 * @Author:      Jonathan Verner (jonathan.verner@matfyz.cz)
 * @License:     GPL v2.0 or later
 * @Created:     2009-03-27.
 * @Last Change: 2009-03-27.
 * @Revision:    0.0
 * Description:
 * Usage:
 * TODO:
 *CHANGES:
 ***************************************************************/

#include <string>

class gAPI;

namespace ticpp { 
  class Document;
  class Element;
}

class atomObj { 
	protected:
		gAPI *api;
		ticpp::Document *xml;
		std::string selfURL, editURL, altURL;
		void extractURLs();
		bool deleteXmlOnExit;


	protected:

		atomObj( gAPI *api );
		~atomObj();

		bool loadFromURL( const std::string & URL );
		bool loadFromFile( const std::string & fileName );
		bool loadFromXML( const std::string & xml );
		bool loadFromXML( ticpp::Document *doc ); // By default will not be deleted by the destructor of atomObj

		void writeToFile( const std::string & fileName );
		std::string getStringXML();
		ticpp::Document *getXML();

		std::string getXMLElentContent(const std::string &elementName);
		void addOrSet( ticpp::Element *where, const std::string name, const std::string value );



	public:

		std::string getAuthor() const;
		std::string getTitle() const;
		std::string getSelfURL() const;
		std::string getVersion() const;

		void setAuthor( const std::string &Author);
		void setTitle( const std::string &Author);

		friend class picasaPhoto;
};


#endif /* _atomObj_H */
