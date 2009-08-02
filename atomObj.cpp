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


atomObj::atomObj( gAPI *API ): api(API), xml(NULL), deleteXmlOnExit(false) {};
atomObj::atomObj( const atomObj& a ): 
  api(a.api), xml(a.xml), deleteXmlOnExit(a.deleteXmlOnExit),
  selfURL(a.selfURL), editURL(a.editURL), altURL(a.altURL)
{
}

bool atomObj::operator<( const atomObj &a ) const { 
  return selfURL < a.selfURL;
}

bool atomObj::loadFromXML( const string & data ) { 
  if ( xml && deleteXmlOnExit ) {
    delete xml;
    xml = NULL;
  }
  try { 
    xml = new ticpp::Document();
    xml->Parse( data );
    extractURLs();
    return true;
  } catch ( ticpp::Exception &ex ) { 
    cerr << "atomObj::loadFromXML(...):"<<ex.what()<<"\n";
    cerr << "------------ BEGIN XML --------------\n";
    cerr << data << "\n";
    cerr << "------------ END XML --------------\n";
    delete xml;
    xml=NULL;
    cerr.flush();
    return false;
  }
};
 
bool atomObj::loadFromURL( const string & URL ) { 
  string data  = api->GET( URL );
  return loadFromXML( data );
};

bool atomObj::loadFromFile( const string & fileName )  {
  if ( xml && deleteXmlOnExit ) {
    delete xml;
    xml = NULL;
  }
  try { 
    xml = new ticpp::Document( fileName );
    extractURLs();
    return true;
  } catch ( ticpp::Exception &ex ) { 
    cerr << "atomObj::loadFromFile(" << fileName <<"):"<<ex.what()<<"\n";
/*    cerr << "------------ BEGIN XML --------------\n";
    cerr << data << "\n";
    cerr << "------------ END XML --------------\n";*/
    delete xml;
    xml=NULL;
    return false;
  }
};

bool atomObj::loadFromXML( ticpp::Document *doc ) {
  if ( xml && deleteXmlOnExit ) {
    delete xml;
    xml = NULL;
  }
  try { 
    xml = doc;
    extractURLs();
    deleteXmlOnExit=false;
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

ticpp::Document *atomObj::getXML() { 
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

atomObj::~atomObj() { 
  if ( xml && deleteXmlOnExit ) delete xml;
};
