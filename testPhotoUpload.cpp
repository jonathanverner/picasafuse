
/***************************************************************
 * testPhotoUpload.cpp
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

#include "convert.h"

using namespace std;

int main( int argc, char **argv ) { 
  string email(""), album(""), photo(""), password("");
  if ( argc != 4 ) { 
    cerr << "usage: "<< argv[0] << " email album photo_filename\n";
    return -1;
  }
  email = argv[1];
  album = argv[2];
  photo = argv[3];
  password=getpass("Enter your password:");
 

  picasaService service( email, password );
  picasaAlbum al = service.getAlbumByID( album, email );
  convert c;
  c.resize( 800000, photo, "/tmp/test_photo.jpg" );
  string comment = c.getComment( "/tmp/test_photo.jpg" );
  try {
    picasaPhoto *p = al.addPhoto( "/tmp/test_photo.jpg", comment );
    cout << "###############################################\n";
    cout << *p;
  } catch ( atomObj::exceptionType &ex ) {
    cerr << "Error adding photo" << endl;
    exit(-1);
  }
  return 0;
}