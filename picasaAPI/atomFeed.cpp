/***************************************************************
 * atomFeed.cpp
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
 
#include "atomFeed.h"
#include "atomEntry.h"
#include "gAPI.h"

#ifndef TIXML_USE_TICPP
#define TIXML_USE_TICPP
#endif
#include "ticpp/ticpp.h"


using namespace std;

atomFeed::atomFeed( gAPI *API ): atomObj(API) {}

bool atomFeed::addNewEntry( atomEntry *entry ) { 
  return entry->loadFromXML( api->POST( selfURL, entry->getStringXML() ) );
};

std::list<atomEntry *> atomFeed::getEntries() {
  atomEntry *ent;
  ticpp::Iterator< ticpp::Element > entry("entry");
  list<atomEntry *> ret;
  for( entry = entry.begin( xml->FirstChildElement() ); entry != entry.end(); entry++ ) {
    ent = new atomEntry( api, *entry );
    ent->deleteXmlOnExit=false;
    ret.push_back( ent );
  }
  return ret;
}



  


