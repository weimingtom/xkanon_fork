/*  image_di_impl.h
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

#ifndef __KANON_DI_IMAGE_IMPL_H__
#define __KANON_DI_IMAGE_IMPL_H__

#include"image_di.h"
#include"typelist.h"

struct BlockFadeData {
	char* tables;
	char* old_tables;
	const FadeTableOrig** diftables;
	enum DIR { UtoD=0, DtoU=1, LtoR=0, RtoL=1} direction;
	enum DDIR { ULtoDR=0, DRtoUL=1, URtoDL=2, DLtoUR=3, ULtoDR1=0, DRtoUL1=1, URtoDL1=2, DLtoUR1=3, ULtoDR2=4, DRtoUL2=5, URtoDL2=6, DLtoUR2=7} diag_dir;
	int table_size;
	int blocksize_x;
	int blocksize_y;
	int blockwidth;
	int blockheight;
	int x0, y0;
	int width;
	int height;
	int max_x, min_x;
	int max_y, min_y;
	BlockFadeData* next;
	bool MakeSlideCountTable(int count, int max_count);
	bool MakeDiagCountTable(int count, int max_count);
	bool MakeDiagCountTable2(int count, int max_count);
	BlockFadeData(int blocksize_x, int blocksize_y, int x0, int y0, int width, int height);
	~BlockFadeData();
};

struct DifImage;

template <class T>
struct Drawer {
	enum {bpp = T::value};
	enum {BiPP = T::value};
	enum {ByPP = T::value/8};
	enum {DifByPP = T::value==16 ? 3 : sizeof(int) };
	static void SetMiddleColor(char* dest, char* src, int c);
	static void SetMiddleColorWithTable(char* dest, char* src,const FadeTable& table);
	static unsigned int CreateColor(int c1, int c2, int c3);
	static void Copy1Pixel(char* dest, char* src);
	static void Draw1PixelFromDif(char* dest, char* dif, const FadeTableOrig* table);
	static DifImage* MakeDifImage(DI_Image& dest, DI_Image& src, char* mask, SEL_STRUCT* sel);
	static char* CalcKido(char* data, int dbpl, int width, int height, int max);
	static void BlockDifDraw(DifImage* image, BlockFadeData* instance);

};

struct DifImage {
	int* draw_xlist;
	char* image;
	char* difimage;
	char* destimage;
	int difbpl;
	int destbpl;
	~DifImage() {
		delete[] draw_xlist;
		delete[] image;
	}
	DifImage(int image_size, int xlist_size) {
		draw_xlist = new int[xlist_size];
		image = new char[image_size];
	}
};

typedef Drawer<Int2Type<16> > Bpp16;
typedef Drawer<Int2Type<32> > Bpp32;

extern unsigned short middle16_data1[32*32];
extern unsigned short middle16_data2[64*64];
extern unsigned short middle16_data3[32*32];
extern unsigned short middle16_data4[64*64];

#if 0
inline void Bpp16::SetMiddleColor(char* dest, char* src, int c) {
	c &= 0xff; int c1 = (c&0xf8)<<2; int c2 = (c&0xfc)<<4;
	int dest_pix = *(short*)dest;
	int src_pix = *(short*)src;
	int dest_pix1 = dest_pix & 0xf800;
	int dest_pix2 = dest_pix & 0x07e0;
	int dest_pix3 = dest_pix & 0x001f;
	int src_pix1 = src_pix & 0xf800;
	int src_pix2 = src_pix & 0x07e0;
	int src_pix3 = src_pix & 0x001f;

	dest_pix1 >>= 11; dest_pix2 >>= 5; dest_pix1 &= 0x1f;

	dest_pix1 *= 0x1f - (c>>3); dest_pix2 *= 0x3f-(c>>2); dest_pix3 *= 0x1f-(c>>3);
	dest_pix1 /= 0x1f; dest_pix2 /= 0x3f; dest_pix3 /= 0x1f;

	src_pix1 += dest_pix1<<11;
	src_pix2 += dest_pix2<<5;
	src_pix3 += dest_pix3;
	if (src_pix1 > 0xf800) {src_pix1 = 0xf800;}
	if (src_pix2 > 0x07e0) {src_pix2 = 0x07e0;}
	if (src_pix3 > 0x001f) {src_pix3 = 0x001f;}
	*(short*)dest = src_pix1 | src_pix2 | src_pix3;
}
#else
template<>
inline void Bpp16::SetMiddleColor(char* dest, char* src, int c) {
	int c1 = (c&0xf8)<<2; int c2 = (c&0xfc)<<4;

	unsigned int dest_pix = *(unsigned short*)dest;
	unsigned int dest_pix1 = dest_pix >> 11;
	unsigned int dest_pix2 = (dest_pix >> 5) & 0x3f;
	unsigned int dest_pix3 = dest_pix & 0x1f;

	register unsigned int d =
		(middle16_data1[dest_pix1+c1] |
		middle16_data2[dest_pix2+c2] |
		middle16_data3[dest_pix3+c1]);
	register unsigned int s = *(unsigned short*) src;

/*	*(unsigned short*) dest = s + d; */

	register unsigned int m = ( ((s&d)<<1) + ( (s^d)&0xf7de ) ) & 0x10820;
	m = ( (((m*3)&0x20840)>>6) + 0x7bef) ^ 0x7bef;

	unsigned int res =  (s + d - m ) | m;
	*(unsigned short*) dest = res;
}
template<>
inline void Bpp32::SetMiddleColor(char* dest, char* src, int c) {
	unsigned int s,d; c &= 0xff; c = 0x100-c-(c>>7);
	s = *(unsigned char*)src++; d = *(unsigned char*)dest;
	d *= c; d>>=8;
	if (d+s > 0xff) *(unsigned char*)dest = 0xff;
	else *(unsigned char*)dest = d+s;
	dest++;

	s = *(unsigned char*)src++; d = *(unsigned char*)dest;
	d *= c; d>>=8;
	if (d+s > 0xff) *(unsigned char*)dest = 0xff;
	else *(unsigned char*)dest = d+s;
	dest++;

	s = *(unsigned char*)src++; d = *(unsigned char*)dest;
	d *= c; d>>=8;
	if (d+s > 0xff) *(unsigned char*)dest = 0xff;
	else *(unsigned char*)dest = d+s;

}
#endif

template<>
inline void Bpp16::SetMiddleColorWithTable(char* dest, char* src,const FadeTable& table) {
	int dest_pix = *(short*)dest;
	int src_pix = *(short*)src;
	dest_pix += table.table16_1[ ((src_pix&0xf800) - (dest_pix&0xf800))>>11]
		 +  table.table16_2[ ((src_pix&0x07e0) - (dest_pix&0x07e0))>>5]
		 +  table.table16_3[ ((src_pix&0x001f) - (dest_pix&0x001f))];
	*(short*)dest = dest_pix;
}

template<>
inline void Bpp32::SetMiddleColorWithTable(char* dest, char* src, const FadeTable& table) {
	*(unsigned char*)dest += table.table32_1[ int(*(unsigned char*) src++) - int(*(unsigned char*)dest) ]; dest++;
	*(unsigned char*)dest += table.table32_2[ int(*(unsigned char*) src++) - int(*(unsigned char*)dest) ]; dest++;
	*(unsigned char*)dest += table.table32_3[ int(*(unsigned char*) src++) - int(*(unsigned char*)dest) ];
}

template<>
inline unsigned int Bpp16::CreateColor(int c1, int c2, int c3) { 
	c1 &= 0xf8; c2 &= 0xfc; c3 &= 0xf8;
	c1 <<= 8; c2 <<= 3; c3 >>= 3;
	unsigned int col = c1 | c2 | c3;
	return (col<<16) | col;
}
template<>
inline unsigned int Bpp32::CreateColor(int c1, int c2, int c3) { 
	unsigned int a;
	char* mem = (char*)&a;
	mem[0] = c3; mem[1] = c2; mem[2] = c1; mem[3] = 0;
	return a;
}

template<>
inline void Bpp16::Copy1Pixel(char* dest, char* src) {
	*(short*)dest = *(short*)src;
}
template<>
inline void Bpp32::Copy1Pixel(char* dest, char* src) {
	*(int*)dest = *(int*)src;
}
template<>
inline void Bpp16::Draw1PixelFromDif(char* dest, char* dif,const FadeTableOrig* table) {
	*(unsigned short*)dest +=
	    table->table1_minus[(int)dif[0]]+
	  + table->table2_minus[(int)dif[1]]+
	  + table->table3_minus[(int)dif[2]];
}
template<>
inline void Bpp32::Draw1PixelFromDif(char* dest, char* dif,const FadeTableOrig* table) {
	int c = *(int*)dif; const int* mtable = table->table4_minus;
	dest[0] += mtable[c&0x1ff];
	dest[1] += mtable[(c>>9)&0x1ff];
	dest[2] += mtable[(c>>18)&0x1ff];
}
#endif /* __KANON_IMAGE_DI_IMPL__ */
