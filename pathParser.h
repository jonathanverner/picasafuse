#ifndef _pathParser_H
#define _pathParser_H

/***************************************************************
 * pathParser.h
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

class pathParser { 
	private:
		std::string userName, albumName, image;
		bool valid;
		bool hUser, hAlbum, hImage;
  
  public:
    
    enum objectType { USER, ALBUM, IMAGE, ROOT, INVALID_OBJECT };
    
  private:
    enum objectType typ;
		

	public:
		pathParser( const std::string &path );
		pathParser( const char *path );
		pathParser();

		void parse( const std::string &path );
		void append( const std::string &path );
		void toParent();


		bool isValid() const { return valid; }

		bool haveImage() const { return hImage; }
		bool haveAlbum() const { return hAlbum; }
		bool haveUser() const { return hUser; }
		enum objectType getType() const { return typ;}


		bool isImage() const { return hImage; }
		bool isAlbum() const { return hAlbum && ! hImage; }
		bool isUser() const { return hUser && ! hAlbum; }
		bool isRoot() const { return ! hUser; }

		std::string getUser() const { return userName; }
		std::string getAlbum() const { return albumName; }
		std::string getImage() const { return image; }
		std::string getLastComponent() const;
		std::string getFullName() const;
		
		pathParser chop() const;


		std::string getHash() const;

		bool operator ==(const pathParser &p) const;
		
		pathParser operator+( const std::string &name ) const;
};


#endif /* _pathParser_H */
