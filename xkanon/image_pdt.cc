/* image_pdt.cc
 *	
 *	
**/
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

#include "image_pdt.h"
#include "image_di.h"
#include "file.h"

#include <string>

using namespace std;

/* #define PTHREAD_VERBOSE */
#define PTHREAD_LOG_FILE "/tmp/pthread.log"

#ifdef HAVE_PTHREAD
#include <pthread.h>
#include <signal.h>
#endif
#include <fcntl.h>
#include <unistd.h>

class DI_ImageMaskPDT : public DI_ImageMask, public IdleEvent {
	ARCINFO* info;
	GRPCONV* conv;
	AyuSys& local_system;
	string filename;
	const char* fdata;
	int prevheight;
	int phase; 
	int in_thread; 
#ifdef HAVE_PTHREAD
	pthread_t thread_id;
	pthread_mutex_t mutex;
	static pthread_mutex_t working_mutex;
	static pthread_cond_t all_thread_waiting_condition;
	static int is_cond_init;
	static int cond_init(void);
	void StopThread(void); 
	void WaitForWork(void); 
	void EndWork(void); 
#endif

	static void OpenVerboseHandle(void);
	static int verbose_log_handle;
#ifdef PTHREAD_VERBOSE
	void PTHREAD_LOG(char* str);
public:
	static void PTHREAD_LOG_EXTERNAL(char* str,int arg1=0,int arg2=0);
private:
#else /* !defined(PTHREAD_VERBOSE) */
	void PTHREAD_LOG(char* str) {
		/* do nothing */
	}
public:
	static void PTHREAD_LOG_EXTERNAL(char* str,int arg1=0,int arg2=0) {
	}
private:
#endif
	void* thread_call(void) {
		PTHREAD_LOG("thread call");
#ifdef HAVE_PTHREAD
		pthread_mutex_lock(&mutex);
		WaitForWork();
#endif /* HAVE_PTHREAD */
		if (in_thread) {
			while(Process()) {
				PTHREAD_LOG("thread loop");
			}
		}
		PTHREAD_LOG("thread end");
#ifdef HAVE_PTHREAD
		EndWork();
		in_thread = 0;
		pthread_mutex_unlock(&mutex);
#else /* HAVE_PTHREAD */
		in_thread = 0;
#endif /* HAVE_PTHREAD */
		return NULL;
	}
public:
	void WaitForReading(void);
	static void* static_thread_call(void* instance) {
		((DI_ImageMaskPDT*)instance)->thread_call();
		return 0;
	}
	int Process(void);
	DI_ImageMaskPDT(AyuSys& sys, ARCINFO* file, const char* path);
	~DI_ImageMaskPDT();
};

int DI_ImageMaskPDT::verbose_log_handle = -1;

#ifdef HAVE_PTHREAD
int DI_ImageMaskPDT::is_cond_init = 0;
pthread_mutex_t DI_ImageMaskPDT::working_mutex;
pthread_cond_t DI_ImageMaskPDT::all_thread_waiting_condition;
int DI_ImageMaskPDT::cond_init(void) {
	if (is_cond_init) return 0; 
	if (pthread_mutex_init(&working_mutex, 0)) {
		return -1; /* error */
	}
	if (pthread_cond_init(&all_thread_waiting_condition, 0)) {
		pthread_mutex_destroy(&working_mutex);
		return -1; /* error */
	}
	is_cond_init = 1;
	return 0;
}
void DI_ImageMaskPDT::StopThread(void) {
	pthread_mutex_lock(&mutex); 
	in_thread = 0;
	pthread_mutex_unlock(&mutex);
	pthread_cond_broadcast(&all_thread_waiting_condition); 
}
void DI_ImageMaskPDT::WaitForWork(void) {
	while(in_thread && pthread_mutex_trylock(&working_mutex)) {
		pthread_cond_wait(&all_thread_waiting_condition, &mutex);
	}
}
void DI_ImageMaskPDT::EndWork(void) {
	if (in_thread) {
		pthread_mutex_unlock(&working_mutex);
	}
	pthread_cond_signal(&all_thread_waiting_condition); 
}
#endif /* HAVE_PTHREAD */

#ifdef PTHREAD_VERBOSE

void DI_ImageMaskPDT::OpenVerboseHandle(void) {
	if (verbose_log_handle != -1) return;
	verbose_log_handle = open(PTHREAD_LOG_FILE, O_RDWR | O_CREAT, 0644);
	if (verbose_log_handle != -1) {
		lseek(verbose_log_handle, 0, 2); 
	}
	PTHREAD_LOG_EXTERNAL("Open handle; pid = %d\n",getpid());
}
void DI_ImageMaskPDT::PTHREAD_LOG(char* str) {
	if (verbose_log_handle == -1) return;
	char buf[1024];
#ifdef HAVE_PTHREAD
	snprintf(buf, 1024, "%s ; %d:%x\n",str, in_thread, thread_id);
	write(verbose_log_handle, buf, strlen(buf));
#endif /* HAVE_PTHREAD */
	return;
}

void DI_ImageMaskPDT::PTHREAD_LOG_EXTERNAL(char* str, int arg1, int arg2) {
	if (verbose_log_handle == -1) return;
	char buf[1024];
	snprintf(buf, 1024, str, arg1, arg2);
	write(verbose_log_handle, buf, strlen(buf));
	return;
}

#else /* !defined(PTHREAD_VERBOSE) */
void DI_ImageMaskPDT::OpenVerboseHandle(void) {/* do nothing */ }
#endif /* defined(PTHREAD_VERBOSE) */

class PDT_Item {
	DI_ImageMask* image;
	PDT_Item* next;
	int hash;
	char* fname;
public:
	~PDT_Item();
	PDT_Item(char* path, int hash, AyuSys& sys);
	void SetNext(PDT_Item* _next) { next=_next;}
	PDT_Item* NextItem(void) { return next;}
	int Hash(void) { return hash;}
	char* FileName(void) { return fname;}
	DI_ImageMask* Image(void) { return image;}
};

int PDT_Reader::DeleteCache() {
	PDT_Item* current = head_cache;
	while(current != 0) {
		current->Image()->ClearUsed();
		current = current->NextItem();
	}
	local_system.SetPDTUsed();
	current = head_cache; PDT_Item* prev = 0;
	while(current != 0) {
		if (! current->Image()->IsUsed()) break;
		prev = current; current = current->NextItem();
	}
	if (current == 0) {
		return 0;
	}
	if (prev) prev->SetNext(current->NextItem());
	else head_cache = current->NextItem();
	delete current;
	return 1;
}
void PDT_Reader::ClearAllCache(void) {
	while(DeleteCache()) ;
}

void PDT_Reader::AppendCache(PDT_Item* item) {
	int len; PDT_Item* current; PDT_Item* prev;
	while(1) {
		current = head_cache; len = 0; prev = 0;
		while(current != 0) {
			prev = current;
			current = current->NextItem();
			len++;
		}
		if (len == 0) {
			head_cache = item;
			return;
		}
		if (len < max_image) break; 
		if (! DeleteCache() ) {
			int i; for (i=PDT_BUFFER_DEAL-1; i>0; i--) {
				if (local_system.IsPDTUsed(i)) {
					local_system.SyncPDT(i);
					if (DeleteCache()) break;
				}
			}
		}
	}
	prev->SetNext(item);
	item->SetNext(0);
}

PDT_Item* PDT_Reader::SearchItem(char* fname) {
	is_used = 1;
	int h = Hash(fname);
	PDT_Item* current = head_cache;
	while(current != 0) {
		if ( current->Hash() == h) {
			if (strcmp(current->FileName(), fname) == 0) break;
		}
		current = current->NextItem();
	}
	return current;
}

DI_ImageMask* PDT_Reader::Search(char* fname) {
	while (is_used) local_system.CallProcessMessages();
	is_used = 1;
	PDT_Item* item = SearchItem(fname);
	if (! item) {
		int h = Hash(fname);
		item = new PDT_Item(fname, h, local_system);
		AppendCache(item);
	}
	if (dynamic_cast<DI_ImageMaskPDT*>(item->Image()))
		dynamic_cast<DI_ImageMaskPDT*>(item->Image())->WaitForReading();
	is_used = 0;
	return item->Image();
}

void PDT_Reader::Preread(char* fname) {
	while (is_used) local_system.CallProcessMessages();
	is_used = 1;
	if (SearchItem(fname) == 0) {
		int h = Hash(fname);
		PDT_Item* item = new PDT_Item(fname, h, local_system);
		AppendCache(item);
	}
	is_used = 0;
	return;
}

int PDT_Reader::Hash(char* fname) {
	int h = strlen(fname);
	while(*fname != 0) {
		h *= 10681091;
		h += int(*fname) * 572108;
		fname++;
	}
	return h;
}

PDT_Reader::~PDT_Reader() {
	DI_ImageMaskPDT::PTHREAD_LOG_EXTERNAL("delete PDT reader\n");
	while(head_cache) {
		PDT_Item* next = head_cache->NextItem();
		DI_ImageMaskPDT::PTHREAD_LOG_EXTERNAL("delete PDT cache %08x\n",int(head_cache));
		delete head_cache;
		head_cache = next;
	}
	DI_ImageMaskPDT::PTHREAD_LOG_EXTERNAL("end of delete PDT reader\n");
	return;
}

void Copy32bpp_15bpp(char* dest, char* src, int width, int height, int bpl);
void Copy32bpp_16bpp(char* dest, char* src, int width, int height, int bpl);
void Copy32bpp_24bpp(char* dest, char* src, int width, int height, int bpl);
void Copy32bpp_32bpp(char* dest, char* src, int width, int height, int bpl);
void Copy32bpprev_15bpp(char* dest, char* src, int width, int height, int bpl);
void Copy32bpprev_16bpp(char* dest, char* src, int width, int height, int bpl);
void Copy32bpprev_24bpp(char* dest, char* src, int width, int height, int bpl);
void Copy32bpprev_32bpp(char* dest, char* src, int width, int height, int bpl);

PDT_Item::PDT_Item(char* path, int h, AyuSys& sys) {
	fname = new char[strlen(path)+10]; strcpy(fname, path);
	hash = h; next = 0;
	image = 0;

	ARCINFO* info = file_searcher.Find(FILESEARCH::PDT, path,".PDT.G00.GPD");
	if (info == 0) {
		fprintf(stderr, "Error : cannot open pdt file %s\n",fname);
		image = new DI_ImageMask;
		image->CreateImage(global_system.DefaultScreenWidth(), global_system.DefaultScreenHeight(), sys.DefaultBypp());
		return;
	}
	image = new DI_ImageMaskPDT(sys, info, path);
	return;
}

PDT_Item::~PDT_Item() {
	if (image) {
		DI_ImageMaskPDT::PTHREAD_LOG_EXTERNAL("delete PDT item; %08x\n",int(image));
		if (dynamic_cast<DI_ImageMaskPDT*>(image)) {
			dynamic_cast<DI_ImageMaskPDT*>(image)->WaitForReading(); 
		}
		delete image;
	}
	delete[] fname;
}

void Copy32bpp_Xbpp(char* dest, char* src, int width, int height, int bpl, int bpp, int rgbrev) {
	if (bpp == 15) {
		if (!rgbrev)
			Copy32bpp_15bpp( dest, src, width, height, bpl);
		else
			Copy32bpprev_15bpp( dest, src, width, height, bpl);
	} else if (bpp == 16) {
		if (!rgbrev)
			Copy32bpp_16bpp( dest, src, width, height, bpl);
		else
			Copy32bpprev_16bpp( dest, src, width, height, bpl);
	} else if (bpp == 24) {
		if (! rgbrev)
			Copy32bpp_24bpp( dest, src, width, height, bpl);
		else
			Copy32bpprev_24bpp( dest, src, width, height, bpl);
	} else if (bpp == 32) {
		if (! rgbrev)
			Copy32bpp_32bpp( dest, src, width, height, bpl);
		else
			Copy32bpprev_32bpp( dest, src, width, height, bpl);
	} else {
		if (! rgbrev)
			Copy32bpp_32bpp( dest, src, width, height, bpl);
		else
			Copy32bpprev_32bpp( dest, src, width, height, bpl);
	}
}
void Copy32bpp_15bpp(char* dest, char* src, int width, int height, int bpl) {
	int i,j;
	for (i=0; i<height; i++) {
		unsigned short* d = (unsigned short*)dest; unsigned char* s = (unsigned char*)src;
		for (j=0; j<width; j++) {
			register unsigned int c3=*s++;
			register unsigned int c2=*s++;
			register unsigned int c1=*s++;
			c1 &= 0xf8; c2 &= 0xf8; c3 &= 0xf8;
			c1 <<= 7; c2 <<= 3; c3 >>= 3;
			*d++ = c1 | c2 | c3;
			s++;
		}
		dest += bpl; src += width*4;
	}
}
void Copy32bpprev_15bpp(char* dest, char* src, int width, int height, int bpl) {
	int i,j;
	for (i=0; i<height; i++) {
		unsigned short* d = (unsigned short*)dest; unsigned char* s = (unsigned char*)src;
		for (j=0; j<width; j++) {
			register unsigned int c1=*s++;
			register unsigned int c2=*s++;
			register unsigned int c3=*s++;
			c1 &= 0xf8; c2 &= 0xf8; c3 &= 0xf8;
			c1 <<= 7; c2 <<= 3; c3 >>= 3;
			*d++ = c1 | c2 | c3;
			s++;
		}
		dest += bpl; src += width*4;
	}
}
void Copy32bpp_16bpp(char* dest, char* src, int width, int height, int bpl) {
	int i,j;
	for (i=0; i<height; i++) {
		unsigned short* d = (unsigned short*)dest; unsigned char* s = (unsigned char*)src;
		for (j=0; j<width; j++) {
			register unsigned int c3=*s++;
			register unsigned int c2=*s++;
			register unsigned int c1=*s++;
			c1 &= 0xf8; c2 &= 0xfc; c3 &= 0xf8;
			c1 <<= 8; c2 <<= 3; c3 >>= 3;
			*d++ = c1 | c2 | c3;
			s++;
		}
		dest += bpl; src += width*4;
	}
}
void Copy32bpprev_16bpp(char* dest, char* src, int width, int height, int bpl) {
	int i,j;
	for (i=0; i<height; i++) {
		unsigned short* d = (unsigned short*)dest; unsigned char* s = (unsigned char*)src;
		for (j=0; j<width; j++) {
			register unsigned int c1=*s++;
			register unsigned int c2=*s++;
			register unsigned int c3=*s++;
			c1 &= 0xf8; c2 &= 0xfc; c3 &= 0xf8;
			c1 <<= 8; c2 <<= 3; c3 >>= 3;
			*d++ = c1 | c2 | c3;
			s++;
		}
		dest += bpl; src += width*4;
	}
}
void Copy32bpp_24bpp(char* dest, char* src, int width, int height, int bpl) {
	int i;
	for (i=0; i<height; i++) {
		char *d = dest; char* s = src;
		int j; for (j=0; j<width; j++) {
			d[0] = s[0]; d[1] = s[1]; d[2]=s[2];
			d+=3; s+=4;
		}
		dest += bpl; src += width*4;
	}
}
void Copy32bpprev_24bpp(char* dest, char* src, int width, int height, int bpl) {
	int i;
	for (i=0; i<height; i++) {
		char *d = dest; char* s = src;
		int j; for (j=0; j<width; j++) {
			d[2] = s[0]; d[1] = s[1]; d[0]=s[2];
			d+=3; s+=4;
		}
		dest += bpl; src += width*4;
	}
}
void Copy32bpp_32bpp(char* dest, char* src, int width, int height, int bpl) {
	int i;
	for (i=0; i<height; i++) {
		char *d = dest; char* s = src;
		memcpy(d, s, width*4);
		dest += bpl; src += width*4;
	}
}
void Copy32bpprev_32bpp(char* dest, char* src, int width, int height, int bpl) {
	int i;
	for (i=0; i<height; i++) {
		char *d = dest; char* s = src;
		int j; for (j=0; j<width; j++) {
			d[2] = s[0]; d[1] = s[1]; d[0]=s[2];
			d+=4; s+=4;
		}
		dest += bpl; src += width*4;
	}
}

void DI_ImageMaskPDT::WaitForReading(void) {
#ifdef HAVE_PTHREAD
	PTHREAD_LOG("Ensure loaded");
	if (in_thread) { 
		void* value;
		PTHREAD_LOG("pthread join");
		StopThread();
		pthread_join(thread_id, &value);
		PTHREAD_LOG("pthread join end");
	}
	if (thread_id) {
		PTHREAD_LOG("pthread detach");
		pthread_detach(thread_id);
		PTHREAD_LOG("pthread detach end");
		thread_id = 0;
	}
#endif
	if (phase) {
		while(Process()) ;
	}
}

DI_ImageMaskPDT::DI_ImageMaskPDT(AyuSys& sys, ARCINFO* _info, const char* path) :
	DI_ImageMask(), IdleEvent(sys), local_system(sys), filename(path) {
	info = _info;
	phase = 1; 
	fdata = 0;
	OpenVerboseHandle();
	in_thread = 0;
#ifdef HAVE_PTHREAD
	thread_id = 0;
	in_thread = 1; 
	PTHREAD_LOG("pthread_create");
	sigset_t stop_all_sigset;
	sigset_t old_sigset;
	sigfillset(&stop_all_sigset);
	if (pthread_sigmask(SIG_BLOCK, &stop_all_sigset, &old_sigset) != 0) {
		/* error */
		fprintf(stderr, "Cannot set signal mask(all signals block)\n");
		sigemptyset(&old_sigset);
	}
	if (cond_init()) {
		in_thread = 0;
		PTHREAD_LOG("pthread DI_ImageMaskPDT::cond_init() error\n");
		goto error_create_thread;
	}
	if (pthread_mutex_init(&mutex, NULL) != 0) {
		in_thread = 0;
		PTHREAD_LOG("pthread_mutex_init error");
		goto error_create_thread;
	}
	if (pthread_create(&thread_id, NULL,
		&DI_ImageMaskPDT::static_thread_call, (void*)this) != 0) {
		in_thread = 0;
		PTHREAD_LOG("pthread_create error");
	}
error_create_thread:
	if (pthread_sigmask(SIG_SETMASK, &old_sigset, NULL) != 0) {
		fprintf(stderr, "Cannot set signal mask(original signal mask)\n");
	}
	PTHREAD_LOG("pthread_create end");
#endif
	if (in_thread == 0) sys.SetIdleEvent(this);
}

int DI_ImageMaskPDT::Process(void) {
	if (phase == 0) return 0; 
	switch(phase) {
		case 1: 
			fdata = 0;
			phase++;
			break;
		case 2: 
			if (info->Process()) break;
			phase++; break;
		case 3: { 
			fdata = info->Read();
			conv = GRPCONV::AssignConverter(fdata, info->Size(), filename.c_str());
			if (local_system.DefaultBypp() == 4)
				conv->ReserveRead(GRPCONV::EXTRACT_32bpp);
			else
				conv->ReserveRead(GRPCONV::EXTRACT_16bpp);
			phase = 11;
			break;
			}
		case 11: 
			if (conv->Process()) break;
			phase++; break;
		case 12:
			 {
			int w = conv->Width(); int h = conv->Height();
			const char* d;
			if (local_system.DefaultBypp() == 4)
				d = conv->Read(GRPCONV::EXTRACT_32bpp);
			else
				d = conv->Read(GRPCONV::EXTRACT_16bpp);
			SetImage((char*)d, w, h, local_system.DefaultBypp(), local_system.DefaultBypp()*w,true);
			if (conv->IsMask())
				SetMask((char*)conv->ReadMask(),true);
			RecordChangedRegionAll();
			delete info;
			info = 0;
			phase = 0;
			prevheight = 0;
			break;}
		default:
			fprintf(stderr," Invalid phase : %d\n",phase);
	}
	return 1;	
}

DI_ImageMaskPDT::~DI_ImageMaskPDT() {
	if (info) delete info;
	if (conv) delete conv;
};
