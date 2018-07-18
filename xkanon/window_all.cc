/*  window_all.cc
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


//#include <gdk/gdkx.h>
#include <gdk/gdkwin32.h>
#include "window_all.h"
#include <stdio.h>
#include <unistd.h>

#define WINDOW_GEOM_X 0 
#define WINDOW_GEOM_Y 0 

//typedef Window XWindow; 

Window_AllScreen::Window_AllScreen() {
	window = 0;
	flags = 0;
	//dpy_restore = 0; 
	screen_restore = 0;
	screen_bpl = -1;
}
Window_AllScreen::~Window_AllScreen() {
	RestoreMode(); 
}
void Window_AllScreen::InitWindow(GtkWidget* new_win) {
	window = new_win;
	GetAllScrnMode();
        g_signal_connect(G_OBJECT(window), "enter_notify_event", G_CALLBACK(enterEvent), gpointer(this));
        g_signal_connect(G_OBJECT(window), "configure_event", G_CALLBACK(configureEvent), gpointer(this));
        g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(destroyEvent), gpointer(this));
	configureEvent(window, 0, this);
}

gboolean Window_AllScreen::destroyEvent(GtkWidget* w, GdkEventAny *event, gpointer pointer) {
	Window_AllScreen* cur = (Window_AllScreen*)pointer;
	cur->RestoreMode();
	return FALSE;
}

gboolean Window_AllScreen::configureEvent(GtkWidget* w, GdkEventConfigure* p1, gpointer pointer) {
	Window_AllScreen* cur = (Window_AllScreen*)pointer;

	if (! cur->IsAllScreen()) {
		int d;
		gdk_window_get_geometry(w->window, &cur->normal_x_pos, &cur->normal_y_pos, &cur->normal_width, &cur->normal_height, &d);
		gdk_window_get_origin(w->window, &cur->normal_x_pos, &cur->normal_y_pos);
	}
	return FALSE;
};

gboolean Window_AllScreen::enterEvent(GtkWidget* w, GdkEventCrossing* event, gpointer pointer) {
	Window_AllScreen* cur = (Window_AllScreen*)pointer;
	if (! cur->IsAllScreen()) return FALSE;

	gdk_pointer_grab(w->window, TRUE, GdkEventMask(0),
		w->window, 0, GDK_CURRENT_TIME); // CurrentTime is a constant in <X11/Xlib.h>
	gtk_window_activate_focus(GTK_WINDOW(w));


	//Display* xdisplay = GDK_WINDOW_XDISPLAY(w->window);
	//XWindow xwindow = GDK_WINDOW_XWINDOW(w->window);
	//XSetInputFocus(xdisplay, xwindow, RevertToPointerRoot, CurrentTime);
#ifdef HAVE_LIBXXF86VM

	int screen_num = DefaultScreen(xdisplay);
	int x=0,y=0;

	XF86VidModeGetViewPort(xdisplay, screen_num, &x, &y);
	if (x != 0 || y != 0)
		XF86VidModeSetViewPort(xdisplay, screen_num, WINDOW_GEOM_X, WINDOW_GEOM_Y);
#endif
	return FALSE;
}

void Window_AllScreen::RestoreMode(void)
{
    if (window == 0) return;
    if (IsAllScreen()) {
	gdk_window_move_resize(window->window, normal_x_pos, normal_y_pos, normal_width, normal_height);
	gdk_pointer_ungrab(GDK_CURRENT_TIME);
#ifdef HAVE_LIBXXF86VM
        XF86VidModeSwitchToMode(dpy_restore, screen_restore, modeinfos[0]);
        XSync(dpy_restore, False);
#endif
	UnsetAllScreen();
    }
}

XF86VidModeModeInfo *
Window_AllScreen::CheckModeLinesRes(int w, int h)
{
    int i;
#ifdef HAVE_LIBXXF86VM
    for (i=0;i<vid_count;i++) { 
        if (modeinfos[i]->hdisplay == w && modeinfos[i]->vdisplay == h)  
            return modeinfos[i];
    }
#endif
    return NULL;
}

void Window_AllScreen::GetAllScrnMode(void) {
#ifdef HAVE_LIBXXF86VM
        int major, minor;
        int eventBase, errorBase;
	Display* xdisplay = GDK_WINDOW_XDISPLAY(window->window);

        if (!XF86VidModeQueryVersion(xdisplay, &major, &minor)) {
            fprintf(stderr, "Can't query video extension version\n");
        } else  if (!XF86VidModeQueryExtension(xdisplay, &eventBase, &errorBase)) {
            fprintf(stderr, "Can't query video extension information\n");
        } else if (major < XF86VM_MINMAJOR ||
                   (major == XF86VM_MINMAJOR && minor < XF86VM_MINMINOR)) {
            fprintf(stderr, "Old  XFree86-VidModeExtension version(%d.%d)\n", major, minor);
            fprintf(stderr, "Required version %d.%d\n", XF86VM_MINMAJOR, XF86VM_MINMINOR);
        } else {
            if (!XF86VidModeGetAllModeLines(xdisplay, 
					    DefaultScreen(xdisplay),
                                            &vid_count, &modeinfos)) {
                fprintf(stderr, "Can't get modeinfos\n");
            } else {
		SetUsableAllScreen();
                dpy_restore = xdisplay;
                screen_restore = DefaultScreen(xdisplay);
            }
        }
#endif
}

int Window_AllScreen::ToAllScreen(int width, int height) {
#ifdef HAVE_LIBXXF86VM
	if (IsAllScreen()) {
		if (all_width == width && all_height == height)
			return 1; 
		RestoreMode();
	}

	XF86VidModeModeInfo* sel;
	Display* xdisplay = GDK_WINDOW_XDISPLAY(window->window);
	sel = CheckModeLinesRes(width, height);
	if (sel == NULL) { 
		fprintf(stderr, "Cannot found modeline : width = %d, height = %d\n",width,height);
		return 0;
	}
	int screen_num = DefaultScreen(xdisplay);
	SetAllScreen();
	gtk_widget_hide(window);
	if (! XF86VidModeSwitchToMode(xdisplay, screen_num, sel)) {
		fprintf(stderr, "Cannot change modeline : width = %d, height = %d\n", width, height);
		UnsetAllScreen();
		gtk_widget_show(window);
		return 0;
	}
	gtk_widget_show(window);
	all_width = width; all_height = height;
	XF86VidModeSetViewPort(xdisplay, screen_num, WINDOW_GEOM_X, WINDOW_GEOM_Y);
        /* XWarpPointer(XtDisplay(top),
        **             None, 
        **             RootWindowOfScreen(XtScreen(top)),
        **             0, 0, 0, 0, 0, 0);
	*/
	XSync(xdisplay, False);
	gdk_window_move_resize(window->window, WINDOW_GEOM_X,WINDOW_GEOM_Y,width,height);
	gdk_pointer_grab(window->window, True, GdkEventMask(0),
		window->window, 0, GDK_CURRENT_TIME); // CurrentTime is a constant in <X11/Xlib.h>
	gtk_window_activate_focus(GTK_WINDOW(window));
	return 1;
#else
	return 0;
#endif
}

