/***************************************************************
 * testPathParser.cpp
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

#include <iostream>
#include <stdlib.h>

#include "pathParser.h"

bool testPathParser() { 
  pathParser t1("/"), t2("/jonathan.verner"), t3("/jonathan.verner/lide a zvirata"), t4("/jonathan.verner/lide a zvirata/joni2.jpg"),
	     t5("//jonathan.verner"), t6("/jonathan.verner/test?authkey=ahoj");
  bool validityTest = true;
  bool t1a = true, t2a = true, t3a = true, t4a = true, t5a = true, t6a=true;
  bool t2b = true, t3b = true, t4b = true, t5b = true, t6b = true;
  if ( !t1.isValid() || !t2.isValid() || !t3.isValid() || !t4.isValid() || !t5.isValid() || !t6.isValid() ) validityTest=false;
  if ( t1.haveUser() || t1.haveAlbum() || t1.haveImage() ) t1a=false;
  if ( !t2.haveUser() || t2.haveAlbum() || t2.haveImage() ) t2a=false;
  if ( !t3.haveUser() || !t3.haveAlbum() || t2.haveImage() ) t3a=false;
  if ( !t4.haveUser() || !t4.haveAlbum() || !t4.haveImage() ) t4a=false;
  if ( !t5.haveUser() || t5.haveAlbum() || t5.haveImage() ) t5a=false;
  if ( !t6.getType() == pathParser::ALBUM ) t6a=false;
  if ( t2.getUser() != "jonathan.verner" ) t2b=false;
  if ( t3.getUser() != "jonathan.verner" || t3.getAlbum() != "lide a zvirata" ) t3b=false;
  if ( t4.getUser() != "jonathan.verner" || t4.getAlbum() != "lide a zvirata" || t4.getImage() != "joni2.jpg" ) t4b=false;
  if ( t5.getUser() != "jonathan.verner" ) t5b=false;
  if ( t6.getUser() != "jonathan.verner" || t6.getAlbum() != "test?authkey=ahoj" ) t6b=false;
  if ( validityTest && t1a && t2a && t3a && t4a && t5a && t6a && t2b && t3b && t4b && t5b && t6b) {
    std::cerr << "testPathParser PASSED\n";
    return true;
  }
  std::cerr << "testPathParser FAILED\n";
  return false;
}



int main() { 
  if ( testPathParser() ) {
    return 0;
  } else {
    exit(1);
  } 
}

 
