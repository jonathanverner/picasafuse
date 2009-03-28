/***************************************************************
 * atomEntry.cpp
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

#include "atomEntry.h"
#include "gAPI.h" 

#ifndef TIXML_USE_TICPP
#define TIXML_USE_TICPP
#endif
#include "ticpp/ticpp.h"

#include <iostream>


using namespace std;

atomEntry::atomEntry( gAPI *API ): api(API), xml(NULL) {};


atomEntry::atomEntry( gAPI *API, const ticpp::Element &entry ): api(API), xml(NULL) { 
    ticpp::Iterator< ticpp::Element > links("link");
    for( links = links.begin( &entry ); links != links.end(); links++ ) {
      if ( links->GetAttribute( "rel" ).compare("edit") == 0 )
	editURL = links->GetAttribute( "href" );
      if ( links->GetAttribute( "rel" ).compare("alternate") == 0 )
	altURL = links->GetAttribute( "href" );
      if ( links->GetAttribute( "rel" ).compare("self") == 0 )
	selfURL = links->GetAttribute( "href" );
    }
    loadFromURL( selfURL );
};


void atomEntry::setAuthor(std::string Author) { 
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

void atomEntry::setTitle(std::string Title) { 
  ticpp::Element *auth = xml->FirstChildElement()->FirstChildElement("title");
  if ( ! auth ) {
    ticpp::Element nA("title",Title);
    xml->FirstChildElement()->insertEndChild( nA );
  } else {
    auth->SetText( Author );
  }
}

bool atomEntry::UPDATE() { 
  return loadFromXML( api->PUT( editURL, getStringXML() ) );
}

bool atomEntry::DELETE() { 
  return api->DELETE( editURL );
  delete xml;
}

~atomEntry::atomEntry() { 
  if ( xml ) delete xml;
};
