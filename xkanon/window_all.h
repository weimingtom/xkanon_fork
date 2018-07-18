/*  window_all.h
 *
 ************************************************************
 *
 *     
 *
 ************************************************************
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


#ifndef __VIDMODE_GTKMM_WINDOW_ALLSCRN_H__
#define __VIDMODE_GTKMM_WINDOW_ALLSCRN_H__

// #define HAVE_LIBXXF86VM 
#ifdef HAVE_CONFIG_H
#include "config.h" 
#endif

//#include <X11/Xlib.h>
#ifdef HAVE_LIBXXF86VM
#include <X11/extensions/xf86vmode.h>
#define XF86VM_MINMAJOR 0
#define XF86VM_MINMINOR 5
#endif

#include<gtk/gtk.h>

#ifndef HAVE_LIBXXF86VM
struct XF86VidModeModeInfo {void* pointer;};
#endif

class Window_AllScreen {
public:
	GtkWidget*	window;
	void InitWindow(GtkWidget* window);
	Window_AllScreen(void);
	~Window_AllScreen();

	int screen_bpl; 
	int flags; 
	void SetAllScreen(void) { flags |= 1;}
	void UnsetAllScreen(void) { flags &= ~1;}
	void SetUsableAllScreen(void) { flags |= 2;}
	int IsUsableAllScreen(void) { return flags & 2; }

	//Display* dpy_restore;
	int screen_restore;
	int vid_count;                    
	XF86VidModeModeInfo **modeinfos;  
	int all_width, all_height;
	int normal_x_pos, normal_y_pos, normal_width, normal_height;

	void GetAllScrnMode(void);
	static gboolean configureEvent(GtkWidget* w, GdkEventConfigure* p1, gpointer pointer);
	static gboolean enterEvent(GtkWidget* w, GdkEventCrossing* event, gpointer pointer);
	static gboolean destroyEvent(GtkWidget* w, GdkEventAny *event, gpointer pointer);


	int IsAllScreen(void) { return (window != 0) && (flags & 1); }
	void RestoreMode(void);	
	int ToAllScreen(int width, int height); 
	XF86VidModeModeInfo* CheckModeLinesRes(int w, int h); 
};

#endif /* ! defined(__VIDMODE_GTKMM_WINDOW_ALLSCRN_H__) */
