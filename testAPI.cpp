/***************************************************************
 * testAPI.cpp
 * @Author:      Jonathan Verner (jonathan.verner@matfyz.cz)
 * @License:     GPL v2.0 or later
 * @Created:     2009-02-27.
 * @Last Change: 2009-02-27.
 * @Revision:    0.0
 * Description:
 * Usage:
 * TODO:
 *CHANGES:
 ***************************************************************/
#include <string>
#include <iostream>
#include <unistd.h>

#include "picasaAPI.h"
#include "picasaAlbum.h"
#include "picasaPhoto.h"

using namespace std;

int main( int argc, char **argv ) { 
  if ( argc < 3 ) { 
    cerr << " usage: " << argv[0] << " e-mail {list|album|get|albumEdit|delete|upload} [URL] [ [albumTitle] [description] | [fileName] ]\n";
    return -1;
  }
  string user( argv[1] ), appName( argv[9] ), command( argv[2] );
  string newTitle, description;
  string fileName;
  if ( command.compare("list") != 0 && argc == 3 ) { 
    cerr << " usage: " << argv[0] << " e-mail {list|album|get} [URL]\n";
    cerr << " the album and get commands need an URL\n";
    return -1;
  }  
  if ( command.compare("albumEdit") == 0 ) {
    if ( argc < 6 ) {
      cerr << " usage: " << argv[0] << " e-mail {list|album|get} [URL]\n";
      cerr << " the album and get commands need an URL\n";
      return -1;
    } 
    newTitle=argv[4];
    description=argv[5];
  }

  if ( command.compare("upload") == 0 ) { 
    if ( argc < 5 ) { 
      cerr << " usage: " << argv[0] << " e-mail {list|album|get|albumEdit|delete|upload} [URL] [ [albumTitle] [description] | [fileName] ]\n";
      return -1;
    }
    fileName = argv[4];
  }


  string password( getpass("Enter your password:") );
//  cout<<"Enter your password:";
//  cin>>password;
  picasaAPI api( user, password );
  picasaAlbum *album=NULL;
  if ( command.compare("list") == 0 ) { 
    cout<<"Album list:\n-------------------------\n";
    list<string> albums = api.albumList();
    for (list<string>::iterator it = albums.begin(); it != albums.end(); it++)
          cout <<"ALBUM URL:"<< *it << ";\n";
    cout<<"-------------------------\n";
  } else {
    string URL(argv[3]);
    if ( command.compare("album") == 0 ) { 
      album = new picasaAlbum( &api, URL );
      cout << *album;
      delete album;
    } else if ( command.compare("get") == 0 ) { 
      cout << api.GET( URL );
    } else if ( command.compare("albumEdit") == 0 ) { 
      album = new picasaAlbum( &api, URL );
      album->setUpdatePolicy( picasaAlbum::MANUAL_SYNC );
      cout << *album;
      album->setTitle( newTitle );
      album->setDescription( description );
      album->update();
      cout << *album;
      delete album;
    } else if ( command.compare("delete") == 0 ) { 
      album = new picasaAlbum( &api, URL );
      album->deleteAlbum();
      delete album;
    } else if ( command.compare("upload") == 0 ) { 
      album = new picasaAlbum( &api, URL );
      picasaPhoto *photo = new picasaPhoto( album, fileName );
    } else { 
      cerr << " usage: " << argv[0] << " e-mail {list|album|get} [URL]\n";
      return -1;
    }
  }
  return 0;
}



