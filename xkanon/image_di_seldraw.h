/*  image_di_seldraw.h
 *      
 */
/*
 *
 *  Copyright (C) 2002-   Kazunori Ueno(JAGARL) <jagarl@creator.club.ne.jp>
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

#ifndef __KANON_DI_IMAGE_SELDRAW_H__
#define __KANON_DI_IMAGE_SELDRAW_H__

#include <vector>
#include "image_di.h"
#include "image_di_impl.h"
#include "typelist.h"

struct Bpp16Mask : Bpp16 { enum {with_mask = 1}; };
struct Bpp16NoMask : Bpp16 { enum {with_mask = 0}; };
struct Bpp32Mask : Bpp32 { enum {with_mask = 1}; };
struct Bpp32NoMask : Bpp32 { enum {with_mask = 0}; };

enum { NoMask = 0, WithMask = 1};

struct SelDrawBase {
	int sel_no;
	bool use_special_maskfunc;

	SelDrawBase(int _sel, const selno_list& sellist, bool use_m);
	virtual int Exec(DI_Image& dest, DI_Image& src, char* mask, SEL_STRUCT* sel, int count) = 0;
};


#define RegisterSelMacro(sel_id, sel_list, is_mask) \
	template<int Bpp, int IsMask> struct SelDrawImpl ## sel_id : Drawer< Int2Type<Bpp> > { \
	enum {bpp = Bpp}; enum {BiPP = Bpp}; enum {ByPP = Bpp/8}; enum {DifByPP = Bpp==16 ? 3 : sizeof(int) };\
		int Exec(DI_Image& dest, DI_Image& src, char* mask, SEL_STRUCT* sel, int count); \
	}; \
	struct SelDraw ## sel_id : SelDrawBase { \
		SelDraw ## sel_id(void) : SelDrawBase(sel_id, sel_list, is_mask) {} \
		SelDrawImpl ## sel_id<16,0> draw16; \
		SelDrawImpl ## sel_id<16,1> draw16m; \
		SelDrawImpl ## sel_id<32,0> draw32; \
		SelDrawImpl ## sel_id<32,1> draw32m; \
		int Exec(DI_Image& dest, DI_Image& src, char* mask, SEL_STRUCT* sel, int count) { \
			int ret = 2; \
			if (src.bypp == 2) { \
				if (mask) ret = draw16m.Exec(dest, src, mask, sel, count); \
				else ret = draw16.Exec(dest, src, mask, sel, count); \
			} else { \
				if (mask) ret = draw32m.Exec(dest, src, mask, sel, count); \
				else ret = draw32.Exec(dest, src, mask, sel, count); \
			} \
			return ret; \
		} \
	}; \
	static SelDraw ## sel_id instance_sel ## sel_id; \
	template<int Bpp, int IsMask> \
	int SelDrawImpl ## sel_id<Bpp,IsMask>


#endif /* __KANON_DI_IMAGE_SELDRAW_H__ */
