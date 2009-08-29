/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/


#include <iostream>
#include <sstream>

#define HAVE_MAGICK

#ifdef HAVE_MAGICK
  #include <math.h>
  #include <Magick++.h>
#else // ! HAVE_MAGICK++
  #include <sys/types.h>
  #include <unistd.h>
  #include <wait.h>
#endif // HAVE_MAGICK++


#include "convert.h"


using namespace std;

convert::convert(): 
  pathToConvert( "/usr/bin/convert" )
{
}

string convert::getComment( const string &image ) { 
  #ifdef HAVE_MAGICK
  Magick::Image img;
  try { 
    img.read( image );
    return img.attribute("comment");
  } catch ( Magick::Exception &ex ) { 
    cerr << " convert::getComment( " << image << " ): ERROR:" << ex.what() << endl;
    return "";
  }
  #else // ! HAVE_MAGICK
  return "";
  #endif // HAVE_MAGICK
}

bool convert::resize( long numOfPix, const string &fIN, const string &fOUT ) {
  string outf;
  if ( fOUT == "" ) outf = fIN;
  else outf = fOUT;
#ifdef HAVE_MAGICK
  Magick::Image img;
  try {
    img.ping( fIN );
    Magick::Geometry sz = img.size();
    uint w = sz.width(), h = sz.height();
    if ( w*h <= numOfPix ) return true;
    double factor = sqrt((double)numOfPix/(double)(w*h));
    stringstream ss;
    ss << floor(w*factor) << "x" << floor(h*factor) << ">";   
    img.read( fIN );
    img.scale( ss.str() );
    img.write( outf );
    return true;
  } catch ( Magick::Exception &ex ) {
    cerr << " Exception while reading/resizing image: " << ex.what() << endl;
    return false;
  }
#else // ! HAVE_MAGICK++
  string cmd("-resize");
  stringstream ss;
  ss << numOfPix << "@> ";
  
  int pid = fork();
  int status;
  switch( pid ) { 
    case 0:
      execl( pathToConvert.c_str(), fIN.c_str(), cmd.c_str(), ss.str().c_str(), outf.c_str(), (char *) NULL );
      return false;
      break;
    default:
      waitpid( pid, &status, 0 );
      return WIFEXITED( status );
  }
#endif // HAVE_MAGICK
}


