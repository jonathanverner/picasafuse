/***************************************************************
 * testAlbumList.cpp
 * @Author:      Jonathan Verner (jonathan.verner@matfyz.cz)
 * @License:     GPL v2.0 or later
 * @Created:     2009-08-01.
 * @Last Change: 2009-08-01.
 * @Revision:    0.0
 * Description:
 * Usage:
 * TODO:
 *CHANGES:
 ***************************************************************/



#include <string>
#include <iostream>
#include <unistd.h>


#include "picasaService.h"
#include "picasaAlbum.h"

using namespace std;

int main( int argc, char **argv ) { 
  string user(""), email(""), password("");
  if ( argc < 2 || argc > 3 ) { 
    cerr << "usage: "<< argv[0] << " user [e-mail] \n";
    return -1;
  }
  user = argv[1];
  if ( argc > 2 ) {
    email = argv[2];
    password=getpass("Enter your password:");
  }
  picasaService service( email, password );
  set<picasaAlbum> albums  = service.albumList( user );
  cout << "###############################################\n";
  cout << "Album list for user " << user << "\n";
  cout << "(as seen by "<<email<<")\n";
  for(set<picasaAlbum>::iterator a = albums.begin(); a != albums.end(); ++a ) { 
    cout << "---------------------\n";
    cout << *a;
  }
  return 0;
}



 
