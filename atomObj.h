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
#include <list>
#include <boost/shared_ptr.hpp>

class gAPI;

namespace ticpp { 
  class Document;
  class Element;
}

typedef boost::shared_ptr<ticpp::Document> ticppDocumentPtr;

class atomObj { 
	protected:
		gAPI *api;
		ticppDocumentPtr xml;
		std::string selfURL, editURL, altURL;
		void extractURLs();
		void extractURLs( const ticpp::Element *root );


	protected:
		atomObj( const atomObj &atom );

		atomObj( gAPI *api );
		~atomObj();

		bool loadFromURL( const std::string & URL );
		bool loadFromFile( const std::string & fileName );
		bool loadFromXML( const std::string & xml );
		bool loadFromXML( ticppDocumentPtr doc );

		void writeToFile( const std::string & fileName );
		ticppDocumentPtr getXML();

		std::string getXMLElentContent(const std::string &elementName);
		void addOrSet( ticpp::Element *where, const std::string name, const std::string value );


	public:

		enum exceptionType { GENERAL_ERROR, ERROR_CREATING_OBJECT }; // Unrecoverable error conditions

	public:

		std::string getAuthor() const;
		std::string getTitle() const;
		std::string getSelfURL() const;
		std::string getVersion() const;

		std::string getAttr( const std::string &attrName ) const;
		std::list<std::string> listAttr() const;

		std::string getStringXML();
		

		void setAuthor( const std::string &Author);
		void setTitle( const std::string &Author);

		virtual bool operator<(const atomObj &a) const;

		friend class picasaPhoto;
		friend class picasaService;
};


#endif /* _atomObj_H */
