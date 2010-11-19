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

using namespace std;


atomObj::atomObj( gAPI *API ): api(API) {};
atomObj::atomObj( const atomObj& a ): 
  api(a.api), xml(a.xml), 
  selfURL(a.selfURL), editURL(a.editURL), altURL(a.altURL)
{
}

bool atomObj::operator<( const atomObj &a ) const { 
  return selfURL < a.selfURL;
}

bool atomObj::loadFromXML( const string & data ) {
  xml.reset();
  try {
    ticppDocumentPtr p(new ticpp::Document());
    xml = p;
    xml->Parse( data );
    extractURLs();
    return true;
  } catch ( ticpp::Exception &ex ) { 
    cerr << "atomObj::loadFromXML(...):"<<ex.what()<<"\n";
    cerr << "------------ BEGIN XML --------------\n";
    cerr << data << "\n";
    cerr << "------------ END XML --------------\n";
    xml.reset();
    cerr.flush();
    return false;
  }
};
 
bool atomObj::loadFromURL( const string & URL ) { 
  string data  = api->GET( URL );
  return loadFromXML( data );
};

bool atomObj::loadFromFile( const string & fileName )  {
  xml.reset();
  try { 
    ticppDocumentPtr p(new ticpp::Document( fileName ) );
    xml = p;
    extractURLs();
    return true;
  } catch ( ticpp::Exception &ex ) { 
    cerr << "atomObj::loadFromFile(" << fileName <<"):"<<ex.what()<<"\n";
/*    cerr << "------------ BEGIN XML --------------\n";
    cerr << data << "\n";
    cerr << "------------ END XML --------------\n";*/
    xml.reset();
    return false;
  }
};

bool atomObj::loadFromXML( ticppDocumentPtr doc ) {
  xml.reset();
  try { 
    xml = doc;
    extractURLs();
    return true;
  } catch ( ticpp::Exception &ex ) { 
    cerr << " atomObj::loadFromXML( ticpp::Document *doc ): Error while extracting URLs: " << ex.what() << "\n";
     return false;
  }
}

void atomObj::extractURLs() {
    extractURLs( xml->FirstChildElement() );
}

void atomObj::extractURLs( const ticpp::Element* root ) { 
    ticpp::Iterator< ticpp::Element > links("link");
    for( links = links.begin( root ); links != links.end(); links++ ) {
      if ( links->GetAttribute( "rel" ).compare("edit") == 0 )
	editURL = links->GetAttribute( "href" );
      if ( links->GetAttribute( "rel" ).compare("alternate") == 0 )
	altURL = links->GetAttribute( "href" );
      if ( links->GetAttribute( "rel" ).compare("self") == 0 )
	selfURL = links->GetAttribute( "href" );
    }
}
    

void atomObj::writeToFile( const string & fileName ) {
  xml->SaveFile( fileName );
}

string atomObj::getStringXML() { 
  ostringstream oss( ostringstream::out );
  oss << *xml;
  return oss.str();
}

ticppDocumentPtr atomObj::getXML() {
  return xml;
}


void atomObj::addOrSet( ticpp::Element *where, const string name, const string value ) { 
  try { 
  ticpp::Element *elm = where->FirstChildElement(name);
  if ( ! elm ) { 
    ticpp::Element nelm(name, value);
    elm->InsertEndChild( nelm );
  } else { 
    elm->SetText( value );
  }
  } catch ( ticpp::Exception &ex ) { 
    cerr << "atomObj::addOrSet( ...,"<<name<<", "<<value<<" ): "<<ex.what();
  }
}


string atomObj::getVersion() const { 
  try { 
    ticpp::Element *root = xml->FirstChildElement();
    if ( root->HasAttribute("gd:etag") ) return root->GetAttribute( "gd:etag" );
    return root->FirstChildElement("gphoto:version")->GetText();
  } catch ( ticpp::Exception &ex ) { 
    return "";
  }
}

string atomObj::getAttr( const string &attrName ) const {
  if ( attrName == "SelfURL" ) return selfURL;
  if ( attrName == "EditURL" ) return editURL;
  try {
    if ( attrName == "Author" ) return getAuthor();
    if ( attrName == "Title" ) return getTitle();
    ticpp::Element *root = xml->FirstChildElement();
    return root->FirstChildElement( attrName )->GetText();
  } catch ( ticpp::Exception &ex ) {
    throw GENERAL_ERROR;
  }
}

list<string> atomObj::listAttr() const {
  list<string> ret;
  ret.push_back( "Author" );
  ret.push_back( "Title" );
  ret.push_back( "SelfURL" );
  ret.push_back( "EditURL" );
  ticpp::Iterator< ticpp::Element > attrs;
  for( attrs = attrs.begin( xml->FirstChildElement() ); attrs != attrs.end(); attrs++ ) {
    cerr << "+ " << attrs->Value() <<";";
    cerr.flush();
    ret.push_back( attrs->Value() );
  }
  return ret;
}


string atomObj::getAuthor() const { 
  return xml->FirstChildElement()->FirstChildElement("author")->FirstChildElement("name")->GetText(false);
}
string atomObj::getTitle() const { 
  return xml->FirstChildElement()->FirstChildElement("title")->GetText(false);
}


void atomObj::setAuthor(const string &Author) { 
  ticpp::Element *auth = xml->FirstChildElement()->FirstChildElement("author",false);

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

void atomObj::setTitle(const string &Title) { 
  addOrSet( xml->FirstChildElement(), "title", Title );
}

atomObj::~atomObj() {};
