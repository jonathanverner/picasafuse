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

atomFeed::atomFeed(const atomFeed& a): atomObj(a)
{
}

atomFeed::atomFeed( gAPI *API ): atomObj(API) {}

bool atomFeed::addNewEntry( atomEntry *entry ) { 
  return entry->loadFromXML( api->POST( selfURL, entry->getStringXML() ) );
};

std::list<atomEntryPtr> atomFeed::getEntries() {
  ticpp::Iterator< ticpp::Element > entry("entry");
  list<atomEntryPtr> ret;
  for( entry = entry.begin( xml->FirstChildElement() ); entry != entry.end(); entry++ ) {
    atomEntryPtr ent( new atomEntry( api, *entry ) );
    ret.push_back( ent );
  }
  return ret;
}



  


