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


pathParser pathParser::chop() const {
  pathParser ret;
  if ( ! valid ) return ret;
  ret.valid=true;
  switch( typ ) {
    case IMAGE:
      ret.albumName=albumName;
      ret.userName=userName;
      ret.hUser=true;
      ret.hAlbum=true;
      ret.hImage=false;
      ret.typ=ALBUM;
      ret.hash=userName+"/"+albumName+"/";
      ret.fullname=userName+"/"+albumName;
      break;
    case ALBUM:
      ret.userName=userName;
      ret.hUser=true;
      ret.hAlbum=false;
      ret.hImage=false;
      ret.typ=USER;
      ret.hash=userName+"//";
      ret.fullname=userName;
      break;
    case USER:
      ret.hUser=false;
      ret.hAlbum=false;
      ret.hImage=false;
      ret.typ=ROOT;
      ret.hash="//";
      ret.fullname="";
      break;
    default:
      ret.typ=INVALID_OBJECT;
      ret.valid=false;
  }
  return ret;
}


void pathParser::parse( const string &path ) {
  list<string> componentList = chopString( path, '/' );
  typ = INVALID_OBJECT;
  valid = false;

  if ( componentList.size() == 3 ) {
      hImage = true;
      image = componentList.back();
      componentList.pop_back();
      typ = IMAGE;
  }

  if ( componentList.size() == 2 ) {
      hAlbum = true;
      albumName = componentList.back();
      componentList.pop_back();
      if ( ! hImage ) typ = ALBUM;
  }

  if ( componentList.size() == 1 ) {
      hUser = true;
      userName = componentList.back();
      componentList.pop_back();
      if ( ! hAlbum) typ = USER;
  }

  if ( componentList.size() == 0 ) {
      valid = true;
      if ( ! hUser ) typ = ROOT;
  }

  // Some special paths are invalid
  if ( ( hImage && image.find(".directory.lock") != -1 ) ||
       ( hUser && userName == ".git" || userName == ".svn") ||
       ( hAlbum && albumName == ".git" || albumName == ".svn" )
     )
  {
      valid = false;
      typ = INVALID_OBJECT;
  }

  hash=userName+"/"+albumName+"/"+image;
  if ( hImage ) fullname= userName + "/" + albumName + "/" + image;
  else if ( hAlbum ) fullname=userName + "/" + albumName;
  else if ( hUser ) fullname=userName;
}

void pathParser::append( const std::string &path ) {
  if ( ! hUser ) {
    hUser = true;
    userName = path;
    hash=userName+"//";
    fullname=userName;
    typ = pathParser::USER;
    return;
  }
  if ( ! hAlbum ) {
    hAlbum = true;
    albumName = path;
    hash=userName+"/"+albumName+"/";
    fullname=userName+"/"+albumName;
    typ = pathParser::ALBUM;
    return;
  }
  if ( ! hImage ) {
    hImage = true;
    image = path;
    hash=userName+"/"+albumName+"/"+image;
    fullname=userName+"/"+albumName+"/"+image;
    typ = pathParser::IMAGE;
    return;
  }
}

void pathParser::toParent() {
  if ( hImage ) {
    hImage = false;
    image = "";
    hash=userName+"/"+albumName+"/";
    fullname=userName+"/"+albumName;
    typ = pathParser::ALBUM;
    return;
  }
  if ( hAlbum ) {
    hAlbum = false;
    albumName = "";
    hash=userName+"//";
    fullname=userName;
    typ = pathParser::USER;
    return;
  }
  if ( hUser ) {
    hUser == false;
    userName = "";
    hash="//";
    fullname="";
    typ = pathParser::ROOT;
    return;
  }
}

std::string pathParser::getLastComponent() const {
  if ( hImage ) return image;
  if ( hAlbum ) return albumName;
  if ( hUser ) return userName;
  return "";
}


bool pathParser::operator==( const pathParser &p ) const {
  if ( p.hUser != hUser || p.hAlbum != hAlbum || p.hImage != hImage || p.valid != valid ) return false;
  return ( p.image == image && p.albumName == albumName && p.userName == userName );
}

bool pathParser::operator<(const pathParser& p) const {
  return (getHash() < p.getHash());
}



pathParser pathParser::operator+( const string& name ) const {
  pathParser ret(*this);
  ret.append( name );
  return ret;
}

