/***************************************************************
 * testPhotoList.cpp
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
#include "picasaPhoto.h"
#include "gAPI.h"

using namespace std;

int main( int argc, char **argv ) { 
  string user(""), album(""), email(""), password(""), authKey("");
  if ( argc < 3 || argc > 5 ) { 
    cerr << "usage: "<< argv[0] << " user [e-mail] \n";
    return -1;
  }
  user = argv[1];
  album = argv[2];
  if ( argc > 3 ) {
    email = argv[3];
    password=getpass("Enter your password:");
  }
  if ( argc > 4 ) {
    authKey = argv[4];
  }

  gAPI *api = new gAPI(email);
  api->login(password);
  picasaService service( api );
  picasaAlbum al = service.getAlbumByID( album, user, authKey );
  cout << "###############################################\n";
  cout << "(as seen by "<<email<<")\n";
  cout << al;
  list<picasaPhotoPtr> photos = al.getPhotos();
  for(list<picasaPhotoPtr>::iterator p = photos.begin(); p != photos.end(); ++p ) {
    cout << "---------------------\n";
    cout << **p;
  }
  return 0;
}



 
 
