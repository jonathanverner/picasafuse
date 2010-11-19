#ifndef _atomFeed_H
#define _atomFeed_H

/***************************************************************
 * atomFeed.h
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

#include <string>
#include <list>
#include <boost/shared_ptr.hpp>

class gAPI;
class atomEntry;

namespace ticpp { 
  class Document;
}

typedef boost::shared_ptr<atomEntry> atomEntryPtr;

class atomFeed : public atomObj { 
	public:
		atomFeed( const atomFeed& a );
		atomFeed( gAPI *api );		
		bool addNewEntry( atomEntry *entry );
		std::list<atomEntryPtr> getEntries();
		

		friend class picasaAlbum;
		friend class picasaPhoto;
};




#endif /* _atomFeed_H */
