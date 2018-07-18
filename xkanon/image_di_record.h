/*  image_di_record.h
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

#ifndef __KANON_DI_IMAGE_RECORD_H__
#define __KANON_DI_IMAGE_RECORD_H__
#include<vector>
#include"image_di.h"

class DI_ImageRecord : virtual public DI_Image {
	int last_index;
	struct Region {
		int x;
		int y;
		int w;
		int h;
	};
	std::vector<Region> regions;
	Region allregion;
	int sum_area;
	int allregion_area;
	bool change_all;
public:
	DI_ImageRecord() : DI_Image() {
		last_index = -1;
		change_all = true;
	}
	virtual ~DI_ImageRecord() {
	}
	virtual void RecordChangedRegion(int x, int y, int width, int height);
	virtual void RecordChangedRegionAll(void);
	virtual bool IsChangedRegionAll(void);
	virtual bool GetChangedRegion(int index, int count, int& x, int& y, int& w, int& h);
	virtual void ClearChangedRegion(int index);
	virtual int GetLastIndex(void);
};

class DI_ImageMaskRecord : public DI_ImageMask, public DI_ImageRecord {
public:
	DI_ImageMaskRecord() : DI_ImageMask(), DI_ImageRecord(){
	}
	virtual ~DI_ImageMaskRecord() {}
};

#endif
