/* image_cursor.h : 
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


#ifndef __KANON_IMAGE_CURSOR_H__
#define __KANON_IMAGE_CURSOR_H__

#include <gdk/gdktypes.h>
#define DELETED_MOUSE_X 100000

class PIX_CURSOR_SAVEBUF {
	char* dest;
	char* src;
	int bpl, dbpl;
	int height;
public:
	PIX_CURSOR_SAVEBUF(GdkImage* dest, GdkImage* src, char* mask, int x, int y);
	~PIX_CURSOR_SAVEBUF();
};

class P_CURSOR { 
	GdkWindow* p_window;
	GdkCursor* null_cursor;
	GdkCursor* arrow_cursor;
public:
	P_CURSOR(GdkWindow* win);
	virtual ~P_CURSOR();
	virtual void DrawImage(GdkImage* im, int x, int y) {}
	virtual void DrawImage(GdkImage* im) {}
	virtual void DrawImageRelative(GdkImage* im, int xx, int yy) {}
	virtual void DrawPixmapRelative(GdkPixmap* pix, int xx, int yy) {}
	virtual void RestoreImage(void) {}
	virtual void UpdateBuffer(void) {}
	virtual void Draw(void);
	virtual void DrawCursor(GdkWindow* window);
	virtual void Delete(void);
	virtual void DeleteCursor(GdkWindow* window);
	virtual void Draw(int x, int y) { Draw();}
	virtual void Draw_without_Delete(void) {Draw();}
};

class PIX_CURSOR : public P_CURSOR { 
	GdkWindow* window; 
	GdkPixmap* pixmap; 
	GdkBitmap* mask; 
	GdkPixmap* background; 
	GdkImage* image; 
	char* image_mask; 

	GdkPixmap* buffer_pixmap; 
	GdkGC* masked_gc,* gc;
	PIX_CURSOR_SAVEBUF* savebuf; 

	int x, y;
public:
	PIX_CURSOR(GdkWindow* win, GdkPixmap* pix, GdkBitmap* bitmap, char* mask, GdkPixmap* background);
	~PIX_CURSOR();
	void DrawImage(GdkImage* im, int x, int y) { 
		if (savebuf) delete savebuf;
		savebuf = 0;
		if (x == DELETED_MOUSE_X) return;
		savebuf = new PIX_CURSOR_SAVEBUF(im, image, image_mask, x, y);
	}
	void DrawImage(GdkImage* im) {
		DrawImage(im,x,y);
	}
	void DrawImageRelative(GdkImage* im, int xx, int yy) {
		DrawImage(im, x+xx, y+yy);
	}
	void DrawPixmapRelative(GdkPixmap* pix, int xx, int yy);
	void RestoreImage(void) { if (savebuf != 0) delete savebuf; savebuf = 0;}
	void UpdateBuffer(void); 
	void Draw(int x, int y);
	void Draw(void) { if (x != DELETED_MOUSE_X) Draw(x,y);}
	void Draw_without_Delete(void) { 
		if (x == DELETED_MOUSE_X) return;
		int orig_x = x;
		x = DELETED_MOUSE_X;
		Draw(orig_x,y);
	}
	void Delete(void);
};

#endif /* __KANON_IMAGE_CURSOR_H__ */
