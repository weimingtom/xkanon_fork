/* main.cc
 *   
 *   
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

// #define MPROF

//#include <X11/Xlib.h>
#include <gtk/gtk.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#if defined(sun) && (defined(sparc) || defined(__sparc))
#  include <pthread.h>
#endif
#include "initial.h"
#include "window.h"
#include "senario.h"

extern "C" void cdrom_setDeviceName(char *);
extern "C" void mus_exit(int is_abort);

static int first_senario = 0;
static int first_point = 0;

//search SIGSEGV
#undef DEFAULT_SAVEPATH
#undef DEFAULT_DATAPATH
#define DEFAULT_SAVEPATH "D:/work_codelite/xkanon_new/kanon_save"
#define DEFAULT_DATAPATH "D:/work_codelite/xkanon_new/kanon"

#ifdef DEFAULT_SAVEPATH
	char* savepath = DEFAULT_SAVEPATH;
#else
	char* savepath = "~/.kanon";
#endif

static int is_quit = 0;
// #include<pthread.h>
void QuitAll(void)
{
	global_system.Finalize();
	// gtk_main_quit();
}
static void null_signal_handler(int sig_num) {
	is_quit = 1;
}
static void alarm_signal_handler(int sig_num) {
//	fprintf(stderr,"alarm : pid %d ; pgid %d\n",getpid(),getpgrp(0));
}

static void signal_handler(int sig_num) {
	if (is_quit) {
		if (global_system.MainSenario()) global_system.MainSenario()->WriteSaveHeader();
		mus_exit(0);
		_exit(0);
	}
	is_quit = 1;
	global_system.Intterupt();
}

static void signal_handler_segv(int sig_num) {
	fprintf(stderr, "Segmentation Fault.");
	if (is_quit) abort(); 
	is_quit = 1;
	if (global_system.MainSenario())
		global_system.MainSenario()->WriteSaveHeader();
	mus_exit(1); /* abort music */
	_exit(0);
}

static SENARIO* senario_f;
void SaveFile(int n) {
	if (senario_f == 0) return;
	if (n < 0) return;
	senario_f->WriteSaveFile(n,
		global_system.GetTitle());
	char** list = senario_f->ReadSaveTitle();
	global_system.UpdateMenu(list);
}

static int idle_isProcess = 0;
extern "C" gint idle_handler(gpointer arg) { // Gtk handler
	
	if (global_system.IsIntterupted() == 2) { gtk_main_quit(); return FALSE; }
	if (global_system.main_window == 0) return FALSE;
	if (! global_system.main_window->IsInitialized() ) return FALSE;
	if (! idle_isProcess) {
		if (is_quit) {
			QuitAll();
			gtk_main_quit();
			return FALSE;
		}
		
		idle_isProcess = 1;
		SENARIO* s = new SENARIO(savepath, global_system);
		senario_f = s;
		{ char** list = s->ReadSaveTitle();
		  global_system.main_window->UpdateMenu(list);
		}
		global_system.ClearSenarioNumber();
		GlobalStackItem item;
		item.SetGlobal(first_senario, first_point);
		if (is_quit) {
			delete s; senario_f = 0;
			QuitAll();
			idle_isProcess = 0;
			return FALSE;
		}
		signal(SIGINT, signal_handler);
		signal(SIGTERM, signal_handler);
		//signal(SIGSEGV, signal_handler_segv);
		global_system.SetMainSenario(s);
fprintf(stderr, "================global_system.SetMainSenario(s);\n");
		while(global_system.main_window != 0) {
            item = s->Play(item); 
			if (is_quit) break; 
			//fprintf(stderr, "================item.IsValid before\n");
			if (! item.IsValid()) break; 
			//fprintf(stderr, "================item.IsValid after\n");
			global_system.CallProcessMessages();
			if (global_system.SaveData() != -1) {
				s->WriteSaveFile(global_system.SaveData(),global_system.GetTitle());
				char** list = s->ReadSaveTitle();
				global_system.UpdateMenu(list);
			} else if (global_system.LoadData() != -1) {
				s->ReadSaveFile(global_system.LoadData(), item);
			} else if (global_system.GoSenario() != -1) {
				item.SetGlobal(global_system.GoSenario(),0);
			}
			global_system.ClearIntterupt();
			global_system.ClearSenarioNumber();
		}
		global_system.SetMainSenario(0);
fprintf(stderr, "================global_system.SetMainSenario(0);\n");
		delete s;
		senario_f = 0;
		global_system.Finalize();
		idle_isProcess = 0;
		gtk_main_quit();
	}
	return FALSE ;
}

extern "C" gint timer(gpointer arg) { // Gtk handler
	if (global_system.IsIntterupted() == 2) { gtk_main_quit(); return FALSE; }
	if (global_system.main_window == 0) return TRUE;
	if (! global_system.main_window->IsInitialized() ) return TRUE;
	global_system.main_window->TimerCall();
	if (! idle_isProcess)
		gtk_idle_add(idle_handler, 0);
	return TRUE;
}

void CgmInit(void);
void SetCgmFile(char* path, int flen);

#ifdef MPROF
extern "C" void mprof_stop(void);
extern "C" void mprof_restart(char*);
extern "C" void set_mprof_autosave(int);
#endif

int main(int argc, char *argv[])
{
	int i;
	Initialize::Exec(); 
	char* fontname = "Sans 24"; 
	int fontsize = 0;

#ifdef MPROF
	mprof_stop();
#endif
	signal(SIGINT, null_signal_handler);
	signal(SIGTERM, null_signal_handler);
	//signal(SIGALRM, alarm_signal_handler);
	//signal(SIGSEGV, signal_handler_segv);

	gtk_set_locale();
	CgmInit();

#if 0 && defined(sun) && (defined(sparc) || defined(__sparc))
	pthread_setconcurrency(10);
#endif
	bindtextdomain("ayusys_gtk2", LOCALEDIR);
	bind_textdomain_codeset ("ayusys_gtk2", "UTF-8");
	textdomain("ayusys_gtk2");

	global_system.SetTextSpeed(1000);

#ifdef DEFAULT_DATAPATH
	char* datpath = DEFAULT_DATAPATH;
#else
	char* datpath = "/tmp/kanon";
#endif
	for (i=1; i<argc; i++) {
		if (strcmp(argv[i], "--path") == 0) {
			datpath = argv[i+1];
		}
	}
	if (file_searcher.InitRoot(datpath) == -1) {
		parse_option(&argc, &argv, global_system,
		     &fontname, &fontsize, &savepath); 
		fprintf(stderr, "Cannot use %s as root directory ; it cannot be read or there is no dat/ directory.\n",datpath);
		return -1;
	}
	global_system.LoadInitFile();

	parse_option(&argc, &argv, global_system,
		     &fontname, &fontsize, &savepath);
	ARCINFO* cgminfo = file_searcher.Find(FILESEARCH::ALL,global_system.config->GetParaStr("#CGM_FILE"),".CGM");
	if (cgminfo != 0) {
		char* cgmdat = cgminfo->CopyRead();
		int clen = cgminfo->Size();
		if (cgmdat != 0) SetCgmFile(cgmdat, clen);
		delete cgminfo; delete[] cgmdat;
	} else {
		/* fprintf(stderr, "Cannot open cgm file : mode.cgm\n"); */
	}
	global_system.InitMusic();
#ifdef MPROF
	mprof_restart("xayusys.mem");
#endif

	gtk_init (&argc, &argv);
	AyuWindow* view = new AyuWindow(global_system);
	if (fontsize != 0)
		view->SetFontSize(fontsize);
	view->SetFont(fontname);
	
	if (argc>1) first_senario = atoi(argv[1]);
	if (argc>2) first_point = atoi(argv[2]);
	if (first_senario == 0) first_senario = global_system.config->GetParaInt("#SEEN_START");
	if (first_senario == 0) first_senario = global_system.config->GetParaInt("#SEEN_SRT");
	if (first_senario == 0) first_senario = 1;

	g_timeout_add(10, timer, 0);
fprintf(stderr, "====================>g_timeout_add\n");

        gtk_main();  
fprintf(stderr, "====================>g_timeout_add 2\n");

	delete view;

        return(0);
}
