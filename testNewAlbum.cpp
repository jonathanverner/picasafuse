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
  string albumTitle(""), email(""), password("");
  if ( argc < 3 || argc > 4 ) { 
    cerr << "usage: " << argv[0] << " e-mail albumTitle [password]\n";
    return -1;
  }
  
  email = argv[1];
  albumTitle = argv[2];
  if ( argc == 4 ) password = argv[3];
  else password = getpass("Enter your password:");
  
  picasaService service( email, password );
  picasaAlbum album = service.newAlbum( albumTitle );
  
  if ( album.PUSH_CHANGES() ) { 
    std::cerr << "Success.\n";
    std::cerr << album;
    return 0;
  } else {
    std::cerr << "Failure.\n";
    return -1;
  }
}



 
