/***************************************************************
 * picasaService.cpp
 * @Author:      Jonathan Verner (jonathan.verner@matfyz.cz)
 * @License:     GPL v2.0 or later
 * @Created:     2009-07-12.
 * @Last Change: 2009-07-12.
 * @Revision:    0.0
 * Description:
 * Usage:
 * TODO:
 *CHANGES:
 ***************************************************************/

 
#include "picasaService.h"
#include "picasaAlbum.h"
#include "gAPI.h";

using namespace std;

list< picasaAlbum* > picasaService::getAlbums(const std::string& user) {
  list<string> albumList = api->albumList( user );
  picasaAlbum *pAlbum;
  for(list<string>::iterator album = albumList.begin(); album != albumList.end(); ++album) { 
    pAlbum = new picasaAlbum(api, 
  }
}

