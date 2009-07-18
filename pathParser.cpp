/***************************************************************
 * pathParser.cpp
 * @Author:      Jonathan Verner (jonathan.verner@matfyz.cz)
 * @License:     GPL v2.0 or later
 * @Created:     2009-07-18.
 * @Last Change: 2009-07-18.
 * @Revision:    0.0
 * Description:
 * Usage:
 * TODO:
 *CHANGES:
 ***************************************************************/

#include "pathParser.h"

#include <list>

using namespace std;

list<string> chopString( const string &s, const char delimiter ) { 
  int pos = 0;
  string curSub = "";
  list<string> ret;
  for( int pos = 0; pos < s.size(); ++pos ) { 
    if ( s[pos] == delimiter ) {
      if ( curSub.size() > 0 ) { 
	ret.push_back( curSub );
	curSub = "";
      }
    } else {
      curSub +=s[pos];
    }
  }
  if ( curSub.size() > 0 ) { 
    ret.push_back(curSub);
  }
  return ret;
}


/* 
 * A valid path is of the form:
 *
 *   /[username[/[albumname[/[imagename]]]]]
 *
 */
pathParser::pathParser( const std::string &path ): 
	valid(false), hUser(false), hAlbum(false), hImage(false)
{
  parse( path );
}


string pathParser::getHash() const { 
  return userName+"/"+albumName+"/"+image;
}


pathParser::pathParser( const char *path ):
	valid(false), hUser(false), hAlbum(false), hImage(false)
{ 
  std::string p(path);
  parse( p );
}

pathParser::pathParser():
	valid(false)
{
}


void pathParser::parse( const string &path ) { 
  list<string> componentList = chopString( path, '/' );
  switch ( componentList.size() ) { 
	  case 3:
		  hImage = true;
		  image = componentList.back();
		  componentList.pop_back();
	  case 2:
		  hAlbum = true;
		  albumName = componentList.back();
		  componentList.pop_back();
	  case 1:
		  hUser = true;
		  userName = componentList.back();
		  componentList.pop_back();
	  case 0:
		  valid = true;
  }
}



