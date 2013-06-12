#ifndef _config_H
#define _config_H

/***************************************************************
 * getPassword.h
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

#include <string>
#include <map>
#include <iosfwd>

#ifdef HAVE_DBUS
#include <set>
#endif

struct myfs_config {
  char *cacheDir, *userName, *cfdir, *password;
  int updateInterval, maxPixels;
  int offline;
  #ifdef HAVE_DBUS
  int useKeyRing;
  #endif
};

class picasaConfig {

private:
  std::string cacheDir, userName, password;
  std::string configDir;
  std::map<std::string,std::string> cf_vals;
  int updateInterval, maxPixels;
  bool offline;


  void inputPassword();
  void loadCF();

#ifdef HAVE_DBUS
  std::set<std::string> usersInKeyRing;
  bool useKeyRing;

  void loadUIKR();
  void saveUIKR() const;
  std::string readFromKeyring( std::string key ) const;
  bool saveToKeyring( const std::string &key, const std::string &value ) const;
#endif

public:

  picasaConfig( myfs_config &cf );

  void saveConfig();
  std::string getUser() const {return userName;};
  std::string getPass() const {return password; };
  std::string getCacheDir() const {return cacheDir;};

  int getUpdateInterval() const {return updateInterval;};
  int getMaxPixels() const {return maxPixels;};
  bool getOffline() const {return offline;};

  friend std::ostream &operator<<(std::ostream &out, const picasaConfig &conf );

};


#endif // _getPassword_H
