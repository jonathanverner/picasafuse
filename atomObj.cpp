/***************************************************************
 * atomObj.cpp
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
#include "atomObj.h"
#include "gAPI.h" 

#ifndef TIXML_USE_TICPP
#define TIXML_USE_TICPP
#endif
#include "ticpp/ticpp.h"

#include <iostream>



atomObj::atomObj( gAPI *API ): api(API), xml(NULL) {};

bool atomObj::loadFromXML( const std::string & data ) { 
  try { 
    xml = new ticpp::Document();
    xml->Parse( data );
    extractURLs();
    return true;
  } catch ( ticpp::Exception &ex ) { 
    std::cerr << "atomObj::loadFromULR(" << URL <<"):"<<ex.what()<<"\n";
    std::cerr << "------------ BEGIN XML --------------\n";
    std::cerr << data << "\n";
    std::cerr << "------------ END XML --------------\n";
    return false;
  }
};
 
bool atomObj::loadFromURL( const std::string & URL ) { 
  string data  = api->GET( URL );
  return loadFromXML( data );
};

bool atomObj::loadFromFile( const std::string & fileName )  {
  try { 
    xml = new ticpp::Document( fileName );
    extractURLs();
    return true;
  } catch ( ticpp::Exception &ex ) { 
    std::cerr << "atomObj::loadFromULR(" << URL <<"):"<<ex.what()<<"\n";
    std::cerr << "------------ BEGIN XML --------------\n";
    std::cerr << data << "\n";
    std::cerr << "------------ END XML --------------\n";
    return false;
  }
};

bool atomObj::loadFromXML( ticpp::Document *doc ) {
  xml = doc;
  extractURLs();
  return true;
}

void atomObj::extractURLs() { 
    ticpp::Element *root = xml->FirstChildElement();
    ticpp::Iterator< ticpp::Element > links("link");
    for( links = links.begin( root ); links != links.end(); links++ ) {
      if ( links->GetAttribute( "rel" ).compare("edit") == 0 )
	editURL = links->GetAttribute( "href" );
      if ( links->GetAttribute( "rel" ).compare("alternate") == 0 )
	altURL = links->GetAttribute( "href" );
      if ( links->GetAttribute( "rel" ).compare("self") == 0 )
	selfURL = links->GetAttribute( "href" );
    }
};

void atomObj::writeToFile( const std::string & fileName ) {
  xml->SaveFile( fileName );
}

std::string atomObj::getStringXML() { 
  ostringstream oss( ostringstream::out );
  oss << *xml.;
  return xml;
}

ticcp::Document *atomObj::getXML() { 
  return xml;
}


void atomObj::addOrSet( ticpp::Element *where, const std::string name, const std::string value ) { 
  ticpp::Element *elm = where->FirstChildElement(name);
  if ( ! elm ) { 
    ticpp::Element nelm(name, value);
    elm->insertEndChild( nelm );
  } else { 
    elm->SetText( value );
  }
}


std::string atomObj::getVersion() const { 
  ticpp::Element *root = xml->FirstChildElement();
  return root->GetAttribute( "gd:etag" );
}

std::string atomObj::getAuthor() const { 
  return xml->FirstChildElement()->FirstChildElement("author")->FirstChildElement("name")->GetText(false);
}
std::string atomObj::getTitle() const { 
  return xml->FirstChildElement()->FirstChildElement("title")->GetText(false);
}


void atomObj::setAuthor(std::string Author) { 
  ticpp::Element *auth = xml->FirstChildElement()->FirstChildElement("author");

  if ( ! auth ) {
    ticpp::Element nm("name",Author), na("author");
    na.insertEndChild( nm );
    xml->FirstChildElement()->insertEndChild( na );
  } else {
    ticpp::Element *name = auth->FirstChildElement("name");
    if ( name ) name->SetText( Author );
    else { 
      ticpp::Element nm("name",Author);
      name->insertEndChild( nm );
    }
  }
}

void atomObj::setTitle(std::string Title) { 
  addOrSet( xml->FirstChildElement(), "title", Title );
}

~atomObj::atomObj() { 
  if ( xml ) delete xml;
};
