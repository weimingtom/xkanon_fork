/* ard.h :  */

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

#ifndef __KANON_ARD_H__
#define __KANON_ARD_H__


class ARDDAT {
	class AyuSys& local_system;
	char* data;
	int* valid_data;
	int width, height;
	int max_index;

public:
	ARDDAT(char* file, class AyuSys& sys);
	~ARDDAT();
	void SetValid(int item); 
	void SetInvalid(int item); 
	int RegionNumber(int x, int y); 
};

#endif /* !defined(__KANON_ARD_H__) */
