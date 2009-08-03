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

atomEntry::atomEntry( gAPI *API ): atomObj( API ) {};

atomEntry::atomEntry(const atomEntry& e): atomObj(e) {
}

atomEntry::atomEntry( gAPI *API, const ticpp::Element &entry ): atomObj( API ) { 
  extractURLs( &entry );
  loadFromURL( selfURL );
};


void atomEntry::setAuthor(std::string Author) { 
  ticpp::Element *auth = xml->FirstChildElement()->FirstChildElement("author");
  if ( ! auth ) {
    ticpp::Element nm("name",Author), na("author");
    na.InsertEndChild( nm );
    xml->FirstChildElement()->InsertEndChild( na );
  } else {
    ticpp::Element *name = auth->FirstChildElement("name");
    if ( name ) name->SetText( Author );
    else { 
      ticpp::Element nm("name",Author);
      name->InsertEndChild( nm );
    }
  }
}

void atomEntry::setTitle(std::string Title) { 
  ticpp::Element *auth = xml->FirstChildElement()->FirstChildElement("title");
  if ( ! auth ) {
    ticpp::Element nA("title",Title);
    xml->FirstChildElement()->InsertEndChild( nA );
  } else {
    auth->SetText( Title );
  }
}

bool atomEntry::UPDATE() { 
  return loadFromXML( api->PUT( editURL, getStringXML() ) );
}

bool atomEntry::PULL_CHANGES() { 
  return loadFromURL( selfURL );
}

bool atomEntry::PUSH_CHANGES() { 
  return UPDATE();
}

bool atomEntry::DELETE() { 
  api->DELETE( editURL );
  delete xml;
  return true; // FIXME: Find out status from api->DELETE
}

