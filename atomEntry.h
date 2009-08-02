#ifndef _atomEntry_H
#define _atomEntry_H

/***************************************************************
 * atomEntry.h
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
#include "atomObj.h"

class gAPI;

namespace ticpp { 
  class Document;
  class Element;
}

class atomEntry : public atomObj { 

	protected:
	
		atomEntry( gAPI *api );
		atomEntry( gAPI *api, const ticpp::Element &entry );

	public:
		 atomEntry(const atomEntry& e);

		void setAuthor(std::string Author);
		void setTitle(std::string Title);

		bool UPDATE(); 
		bool DELETE();

		friend class atomFeed;
};




#endif /* _atomEntry_H */
