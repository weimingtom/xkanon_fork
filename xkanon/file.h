/*  file.h  : 
 *            
 *     class FILESEARCH : 
 *     class ARCINFO : 
 *     class PDTCONV : 
 */

/*
 *
 *  Copyright (C) 2000-   Kazunori Ueno(JAGARL) <jagarl@creator.club.ne.jp>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
*/

#ifndef __KANON_FILE_H__
#define __KANON_FILE_H__

#ifndef DIR_SPLIT
#define DIR_SPLIT '/'	/* UNIX */
#endif

// read 'KANON' compressed file

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#if defined(__sparc) || defined(sparc) || defined(__MINGW32__)
#  if !defined(WORDS_BIGENDIAN)
#    define WORDS_BIGENDIAN 1
#  endif
#endif

#ifdef WORDS_BIGENDIAN

#define INT_SIZE 4

static int read_little_endian_int(const char* buf) {
	const unsigned char *p = (const unsigned char *) buf;
	return (p[3] << 24) | (p[2] << 16) | (p[1] << 8) | p[0];
}

static int read_little_endian_short(const char* buf) {
	const unsigned char *p = (const unsigned char *) buf;
	return (p[1] << 8) | p[0];
}

static int write_little_endian_int(char* buf, int number) {
	int c = read_little_endian_int(buf);
	unsigned char *p = (unsigned char *) buf;
	unsigned int unum = (unsigned int) number;
	p[0] = unum & 255;
	unum >>= 8;
	p[1] = unum & 255;
	unum >>= 8;
	p[2] = unum & 255;
	unum >>= 8;
	p[3] = unum & 255;
	return c;
}

static int write_little_endian_short(char* buf, int number) {
	int c = read_little_endian_short(buf);
	unsigned char *p = (unsigned char *) buf;
	unsigned int unum = (unsigned int) number;
	p[0] = unum & 255;
	unum >>= 8;
	p[1] = unum & 255;
	return c;
}

#else // defined(WORDS_BIGENDIAN)

// assume little endian...
#define INT_SIZE 4

inline int read_little_endian_int(const char* buf) {
	return *(int*)buf;
}

inline int read_little_endian_short(const char* buf) {
	return *(short*)buf;
}

inline int write_little_endian_int(char* buf, int number) {
	int c = *(int*)buf; *(int*)buf = number; return c;
}

inline int write_little_endian_short(char* buf, int number) {
	int c = *(short*)buf; *(short*)buf = number; return c;
}
#endif // WORDS_BIGENDIAN


class ARCFILE;
class DIRFILE;
class SCN2kFILE;
class RaffresiaFILE;
class ARCINFO;
class FILESEARCH {
public:
#define TYPEMAX 13
	enum FILETYPE {
		ALL = 1, 
		ROOT= 2, 
		PDT = 3, 
		SCN = 4, 
		ANM = 5, 
		ARD = 6, 
		CUR = 7, 
		MID = 8, 
		WAV = 9, 
		KOE = 10, 
		BGM = 11, 
		MOV = 12  
	};
	enum ARCTYPE {ATYPE_DIR, ATYPE_ARC, ATYPE_SCN2k, ATYPE_Raffresia};
private:
	DIRFILE* root_dir;
	DIRFILE* dat_dir;
	ARCFILE* searcher[TYPEMAX];
	ARCTYPE is_archived[TYPEMAX];
	char* filenames[TYPEMAX];
	static ARCTYPE default_is_archived[TYPEMAX];
	static char* default_dirnames[TYPEMAX];
public:
	FILESEARCH(void);
	~FILESEARCH();
	int InitRoot(char* root);
	void SetFileInformation(FILETYPE type, ARCTYPE is_arc,
		char* filename);
	void AppendFileInformation(FILETYPE type, ARCTYPE is_arc,
		char* filename);
	ARCFILE* MakeARCFILE(ARCTYPE tp, char* filename);
	class ARCINFO* Find(FILETYPE type, const char* fname, const char* ext=0);
	char** ListAll(FILETYPE type);
};

class ARCINFO {
protected:
	char* arcfile;
	long offset;
	int arcsize; int filesize;
	enum {INIT, ERROR_, MMAP, READ, READ_BEFORE_EXTRACT, EXTRACT, EXTRACT_MMAP, DONE} status;
	int fd;
	char* readbuf;
	int read_count;
	int extract_size; char* destbuf;
	char* extract_src;
	int src_count, dest_count;
	char* mmapped_memory;
	char* retbuf;

protected:
	ARCINFO(char* arcf, long offset, int size); // only from ARCFILE
	friend class ARCFILE;
	friend class DIRFILE;
	friend class RaffresiaFILE;

	virtual void Start(void);
	virtual void Reading(void);
	virtual void Extracting(void);
public:
	static void Extract(char*& dest, char*& src, char* destend, char* srcend);
public:
	virtual ~ARCINFO();
	int Process(void) {
		if (status == INIT) Start();
		else if (status == READ || status == READ_BEFORE_EXTRACT) Reading();
		else if (status == EXTRACT || status == EXTRACT_MMAP) Extracting();
		else if (status == MMAP || status == DONE) return 0;
		if (status == ERROR_) return 0;
		return 1;
	}
	int Size(void) {
		if (status == INIT) Process();
		if (status == ERROR_) return 0;
		return filesize;
	}
	char* CopyRead(void); 
	const char* Read(void) {
		while(retbuf == 0 && status != ERROR_) Process();
		return retbuf;
	}
	const char* Path(void);
	FILE* OpenFile(int* length=0); 
};

class GRPCONV {
public:
	enum EXTRACT_TYPE { EXTRACT_INVALID, EXTRACT_32bpp, EXTRACT_16bpp};

	int width;
	int height;
	int mask;
	const char* filename;
	char* imagebuf;
	char* maskbuf;
	EXTRACT_TYPE imagetype;

	int Width(void) { return width;}
	int Height(void) { return height;}
	int IsMask(void) { return mask;}
	void SetFilename(const char*);

	GRPCONV(void);
	virtual ~GRPCONV();
	void Init(void);
	void SetImage(char* image, EXTRACT_TYPE type);
	void SetMask(char* m);

	virtual bool ReserveRead(EXTRACT_TYPE mode) = 0; 
	virtual bool Process(void) = 0; 

	const char* Read(EXTRACT_TYPE mode = EXTRACT_32bpp) {
		while(Process()) ;
		if (imagebuf && imagetype == mode) return imagebuf;

		if (ReserveRead(mode) == false) return 0;
		while(Process()) ;
		return imagebuf;
	}
	const char* ReadMask(void) {
		if (! mask) return 0;
		if (maskbuf) return maskbuf;
		Read();
		return maskbuf;
	}
	static GRPCONV* AssignConverter(const char* inbuf, int inlen, const char* fname);
	static GRPCONV* AssignConverter(ARCINFO* info) {
		const char* dat = info->Read();
		if (dat == 0) return 0;
		return AssignConverter(dat, info->Size(), "???");
	}
};

extern FILESEARCH file_searcher;

#endif // !defined(__KANON_FILE_H__)
