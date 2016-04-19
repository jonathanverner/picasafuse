#include "config.h"

#include <stdlib.h>
#include <unistd.h>

#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;


#ifdef HAVE_DBUS

#include <dbus-c++/dbus.h>
#include "org.kde.kwallet.h"


const char* KWALLET_SERVICE_NAME = "org.kde.kwalletd";
const char* KWALLET_OBJECT_PATH = "/modules/kwalletd";

class KWallet
            : public org::kde::KWallet_proxy,
            public DBus::IntrospectableProxy,
            public DBus::ObjectProxy
{
public:
    KWallet(DBus::Connection& connection, const char* name, const char* path)
            : DBus::ObjectProxy(connection, path, name) {
    }
};

DBus::BusDispatcher dispatcher;

#endif // HAVE_DBUS



bool getKeyValue( char *str, string &key, string &value ) {
  int pos=0;
  bool rdKey=true;
  key="";
  value="";
  while ( rdKey ) {
    switch( str[pos] ){
      case '#': return false;
      case '\0': return false;
      case ' ':
      case '=':
	rdKey=false;
	break;
      default:
	key+=str[pos];
    }
    pos++;
  }
  while ( str[pos] && ( str[pos]==' ' || str[pos]=='=' ) ) pos++;
  if ( ! str[pos] ) return false;
  while ( str[pos] ) {
    switch( str[pos] ){
      case '#': return (value != "" );
      default:
	value+=str[pos];
    }
    pos++;
  }
  return true;
}

void picasaConfig::loadCF() {
  string cf_file = configDir + "/config";

  ifstream cf_ss(cf_file.c_str());
  char ln[300];

  if ( ! cf_ss.is_open() ) return;
  
  while ( ! cf_ss.eof() ) {
    string key, value;
    cf_ss.getline(ln,300);
    if ( getKeyValue( ln, key, value ) ) {
      cf_vals[key]=value;
      stringstream ss(value);
      if ( key == "cacheDir" ) cacheDir = value;
      else if ( key == "userName" ) userName = value;
      else if ( key == "password" ) password = value;
      else if ( key == "maxPixels" ) ss >> maxPixels;
      else if ( key == "updateInterval" ) ss >> updateInterval;
      else if ( key == "offline" ) offline = (value == "true");
#ifdef HAVE_DBUS
      else if ( key == "useKeyRing" ) useKeyRing = (value != "false" );
#endif
    }
  }

  cf_ss.close();

}
  
picasaConfig::picasaConfig(myfs_config& cf):
  userName(""),password(""),updateInterval(86400),maxPixels(900000),
#ifdef HAVE_DBUS  
  useKeyRing(true),
#endif
  offline(false)
{
  if ( cf.cfdir != NULL ) configDir=cf.cfdir;
  else {
    string homeDir = getenv("HOME");
    configDir = homeDir + "/.picasaFUSE";
  };
  cacheDir = configDir;
  loadCF();
  if ( cf.cacheDir != NULL ) cacheDir = cf.cacheDir;
  if ( cf.userName != NULL ) userName = cf.userName;
  if ( cf.password != NULL ) password = cf.password;
  if ( cf.maxPixels > 0 ) maxPixels = cf.maxPixels;
  if ( cf.updateInterval > 0 ) updateInterval = cf.updateInterval;
  if ( cf.offline ) offline = true;
#ifdef HAVE_DBUS  
  if ( ! cf.useKeyRing ) useKeyRing = false;
#endif
  if ( userName != ""  && password == "" ) inputPassword();
}

#ifdef HAVE_DBUS

void picasaConfig::saveUIKR() const {
  string key_file = configDir+"/keyring_saved";
  ofstream key_ss(key_file.c_str());
  if ( ! key_ss.is_open() ) return;
  for( std::set<string>::const_iterator it = usersInKeyRing.begin(); it != usersInKeyRing.end(); ++it ) {
    key_ss << *it << "\n";
  }
    key_ss.close();
}

void picasaConfig::loadUIKR() {
  string key_file = configDir+"/keyring_saved";
  ifstream key_ss(key_file.c_str());
  char ln[300];
  if ( ! key_ss.is_open() ) return;
  while( ! key_ss.eof() ) {
    key_ss.getline(ln,300);
    string user(ln);
    usersInKeyRing.insert(user);
  }
    key_ss.close();
}


string picasaConfig::readFromKeyring(string key) const {
  try {
    DBus::default_dispatcher = &dispatcher;
    DBus::Connection bus = DBus::Connection::SessionBus();
    KWallet wallet(bus, KWALLET_SERVICE_NAME, KWALLET_OBJECT_PATH);
    int handle = wallet.open("kdewallet",0,"picasaFUSE");
    if ( handle > 0 ) {
      return wallet.readPassword(handle,"picasaFUSE",key,"picasaFUSE");
    }
  } catch ( ... ) {
  }
  return "";
}

bool picasaConfig::saveToKeyring( const string &key, const string &value ) const {
  try {
    DBus::default_dispatcher = &dispatcher;
    DBus::Connection bus = DBus::Connection::SessionBus();
    KWallet wallet(bus, KWALLET_SERVICE_NAME, KWALLET_OBJECT_PATH);
    int handle = wallet.open("kdewallet",0,"picasaFUSE");
    if ( handle > 0 ) {
      int ret = wallet.writePassword(handle,"picasaFUSE",key,value,"picasaFUSE");
      if ( ret == -1 ) return false;
      return true;
    }
    return false;
  } catch ( ... ) {
    return false;
  }
  return false;
}
#endif

void picasaConfig::inputPassword() {
  string prompt = "Enter password for "+userName + ": ";

#ifdef HAVE_DBUS

  if ( ! useKeyRing ) {
    password = getpass(prompt.c_str());
    return;
  }
  loadUIKR();
  if ( usersInKeyRing.find(userName) != usersInKeyRing.end()) {
    password = readFromKeyring( userName );
  }
  if ( password == "" ) {
    password = getpass(prompt.c_str());
    if ( saveToKeyring( userName, password ) ) {
      usersInKeyRing.insert(userName);
      saveUIKR();
    }
  }

#else

  password = getpass(prompt.c_str());

#endif
}


void picasaConfig::saveConfig() { 
}

ostream &operator<<(ostream &out, const picasaConfig &conf) {
  out << "#################  CONFIG  ##################\n";
  out << " userName       = " << conf.getUser() << "\n";
  out << " cacheDir       = " << conf.getCacheDir() << "\n";
  out << " maxPixels      = " << conf.getMaxPixels() << "\n";
  out << " updateInterval = " << conf.getUpdateInterval()<<"\n";
#ifdef HAVE_DBUS  
  out << " useKeyRing	  = ";
  if ( conf.useKeyRing ) out << "true\n";
  else out << "false\n";
#endif
  out << "#############################################\n";
  return out;
}
