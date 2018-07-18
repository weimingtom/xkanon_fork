#include<string.h>
#include<stdio.h>
#include"image_di_impl.h"

template<class T>
DifImage* Drawer<T>::MakeDifImage(DI_Image& destimage, DI_Image& srcimage, char* mask, SEL_STRUCT* sel) {
	int width = sel->x2-sel->x1+1;
	int height = sel->y2-sel->y1+1;
	char* dest = destimage.data + sel->x3*ByPP + sel->y3*destimage.bpl;
	char* src = srcimage.data+sel->x1*ByPP + sel->y1*srcimage.bpl;
	if (mask) mask += sel->x1 + sel->y1*srcimage.width;
	int dbpl = destimage.bpl;
	int sbpl = srcimage.bpl;
	int mbpl = srcimage.width;

	DifImage* retimage = new DifImage(width*height*DifByPP, height*2);
	char* dif_buf = retimage->image;
	int* draw_xlist = retimage->draw_xlist;
	char* masked_line = new char[width*ByPP];

	retimage->destimage = dest;
	retimage->destbpl = destimage.bpl;
	retimage->difimage = retimage->image;
	retimage->difbpl = width*DifByPP;

	int i,j;

	for (i=0; i<height; i++) {
		if (mask) {
			char* s = src + i*sbpl;
			char* d = masked_line;
			char* m = mask + i*mbpl;
			memcpy(masked_line, dest+i*dbpl, width*ByPP);
			for (j=0; j<width; j++) {
				char mask_char = *m;
				if (mask_char) {
					if (mask_char == -1) {
						Copy1Pixel(d, s);
					 }else {
						SetMiddleColor(d, s, mask_char);
					}
				}
				s += ByPP; d += ByPP; m++;
			}
		}
		char* d = dest + i*dbpl;
		char* s = mask ? masked_line : src + i*sbpl;
		if (BiPP == 16) {
			char* dif = dif_buf + i*width*3;
			for (j=0; j<width; j++) {
				int c1, c2, cc;
				c1 = *(short*)d;
				c2 = *(short*)s;
				cc = ((c2>>11)&0x1f)-((c1>>11)&0x1f); dif[0] = cc&0x3f;
				cc = ((c2>> 5)&0x3f)-((c1>> 5)&0x3f); dif[1] = cc&0x7f;
				cc = ((c2    )&0x1f)-((c1    )&0x1f); dif[2] = cc&0x3f;
				s += 2; d += 2; dif += 3;
			}
			dif = dif_buf + i*width*3;
			for (j=0; j<width; j++) {
				if (dif[0]|dif[1]|dif[2]) break;
				dif += 3;
			}
			draw_xlist[0] = j;
			dif = dif_buf + i*width*3 + width*3;
			for (j=0; j<width; j++) {
				dif -= 3;
				if (dif[0]|dif[1]|dif[2]) break;
			}
			draw_xlist[1] = width-j;
			draw_xlist += 2;
		} else { /* BiPP == 32 */
			int* dif = ((int*)dif_buf) + i*width;
			for (j=0; j<width; j++) {
				int c, cc;
				c = int(*(unsigned char*)(s+0)) - int(*(unsigned char*)(d+0)); cc = c&0x1ff;
				c = int(*(unsigned char*)(s+1)) - int(*(unsigned char*)(d+1)); cc |= (c&0x1ff)<<9;
				c = int(*(unsigned char*)(s+2)) - int(*(unsigned char*)(d+2)); cc |= (c&0x1ff)<<18;
				*dif = cc;
				s += ByPP; d += ByPP; dif++;
			}
			dif = ((int*)dif_buf) + i*width;
			for (j=0; j<width; j++) {
				if (*dif) break;
				dif++;
			}
			draw_xlist[0] = j;
			dif = ((int*)dif_buf) + i*width + width;
			for (j=0; j<width; j++) {
				dif--;
				if (*dif) break;
			}
			draw_xlist[1] = width-j;
			draw_xlist += 2;
		}
	}
	delete[] masked_line;
	return retimage;
}


template<class T>
char* Drawer<T>::CalcKido(char* data, int dbpl, int width, int height, int max) {
	if (max == 0) max = 16;
	char* kido = new char[width*height];
	char kido_table[256];
	int i,j;
	for (i=0; i<256; i++) {
		kido_table[i] = i * max / 256;
	}
	for (i=0; i<height; i++) {
		char* line = data; char* k = kido+i*width;
		data += dbpl;
		for (j=0; j<width; j++) {
			int c;
			if (BiPP == 16) {
				short col = *(short*)line;
				int r = (col>>11)&0x1f;
				int g = (col>>5)&0x3f;
				int b = (col)&0x1f;
				c = r * 20065550 + g * 19696451 + b * 7659419 + 23710710;
				c >>= 23;
			} else { /* BiPP == 32 */
				int r = *(unsigned char*)(line+2);
				int g = *(unsigned char*)(line+1);
				int b = *(unsigned char*)(line);
				c = r * (20065550/8) + g * (19696451/8) + b * (7659419/8) + 23710710;
				c >>= 23;
			}
			*k++ =  kido_table[c]; line += ByPP;
		}
	}
	return kido;
}

static void MakeSlideTable(char* table, int dif, int x0, int x1, int x2) {
	int i;
	int fade = 32;
	if (x0 > x2) x0 = x2;
	if (x1 > x2) x1 = x2;
	for (i=0; i<x0; i++) {
		*table = fade;
		table += dif;
	}
	if (x0 < 0) fade += x0;
	for (; i<x1; i++) {
		*table = --fade;
		table += dif;
	}
	for (; i<x2; i++) {
		*table = 0;
		table += dif;
	}
}
bool BlockFadeData::MakeSlideCountTable(int count, int max_count) {
	int xl = (table_size+32) * count / max_count;
	int xf = xl - 32;
	int dif = direction==UtoD ? 1 : -1;
	char* cur_table = direction==UtoD ? tables : tables+table_size-1;

	MakeSlideTable(cur_table, dif, xf, xl, table_size);

	if (count >= max_count) return true;
	return false;
}

bool BlockFadeData::MakeDiagCountTable(int count, int max_count) {
	DIR hdir;
	if (diag_dir == ULtoDR1 || diag_dir == ULtoDR2 || diag_dir == DLtoUR1 || diag_dir == DLtoUR2) hdir = UtoD;
	else hdir = DtoU;


	char* htable = tables + ( (hdir == UtoD) ? 0 : blockwidth - 1);;
	int xl = (blockwidth+32) * count / max_count;
	int xf = xl - 32;
	MakeSlideTable(htable, hdir==UtoD ? 1 : -1, xf, xl, blockwidth);

	htable = tables;
	char* curtable = tables+blockwidth;

	int i,j;
	for (i=1; i<blockheight; i++) {
		int xd = i*blockwidth/blockheight;
		switch(diag_dir){
		case ULtoDR1: case DRtoUL2:
			for (j=0; j<xd; j++) *curtable++ = htable[xd];
			for (; j<blockwidth; j++) *curtable++ = htable[j];
			break;
		case DRtoUL1: case ULtoDR2:
			for (j=0; j<xd; j++) *curtable++ = htable[j];
			for (; j<blockwidth; j++) *curtable++ = htable[xd];
			break;
		case URtoDL1: case DLtoUR2:
			xd = blockwidth - 1 - xd;
			for (j=0; j<xd; j++) *curtable++ = htable[j];
			for (; j<blockwidth; j++) *curtable++ = htable[xd];
			break;
		case DLtoUR1: case URtoDL2:
			xd = blockwidth - 1 - xd;
			for (j=0; j<xd; j++) *curtable++ = htable[xd];
			for (; j<blockwidth; j++) *curtable++ = htable[j];
			break;
		}
	}
	if (diag_dir == DRtoUL1 || diag_dir == ULtoDR2) {
		for (j=0; j<blockwidth; j++) tables[j] = htable[0];
	} else if (diag_dir == DLtoUR1 || diag_dir == URtoDL2) {
		for (j=0; j<blockwidth; j++) tables[j] = htable[blockwidth-1];
	}
	if (count >= max_count) return true;
	else return false;
}
bool BlockFadeData::MakeDiagCountTable2(int count, int max_count) {
	DIR hdir;
	if (diag_dir >= ULtoDR2) { diag_dir = DDIR(diag_dir-4);}
	if (diag_dir == ULtoDR1 || diag_dir == URtoDL1) hdir = UtoD;
	else hdir = DtoU;


	char* htable = tables + ( (hdir == UtoD) ? 0 : blockwidth*2 - 1);;
	int xl = (blockwidth*2+32) * count / max_count;
	int xf = xl - 32;
	MakeSlideTable(htable, hdir==UtoD ? 1 : -1, xf, xl, blockwidth*2);

	htable = tables;
	char* curtable = tables+blockwidth*2;

	int i,j;
	for (i=2; i<blockheight; i++) {
		int y0 = i*blockwidth/blockheight;
		if (diag_dir <= DRtoUL1) {
			for (j=0; j<blockwidth; j++) curtable[j] = htable[y0+j];
		} else {
			for (j=0; j<blockwidth; j++) curtable[blockwidth-1-j] = htable[y0+j];
		}
		curtable += blockwidth;
	}
	if (diag_dir <= DRtoUL1) {
		for (j=0; j<blockwidth; j++) tables[j+blockwidth] = tables[j];
	} else {
		for (j=0; j<blockwidth; j++) tables[j+blockwidth] = tables[blockwidth-1-j];
		for (j=0; j<blockwidth; j++) tables[j] = tables[j+blockwidth];
	}
	if (count >= max_count) return true;
	else return false;
}

#if 0 /* obsolete */
static void CalcDifTable(char* table, char* old_table, const FadeTableOrig** dif_table, int table_size, int& ret_min_count, int& ret_max_count) {
	int i;
	int min_flag = 1;
	int min_count = 0; int max_count = 0;
	for (i=0; i<table_size; i++) {
		dif_table[i] = FadeTableOrig::DifTable(old_table[i], table[i]);
		old_table[i] = table[i];
		if (dif_table[i] == 0) {
			if (min_flag == 1) min_count = i;
		} else {
			max_count = i;
			min_flag = 0;
		}
	}
	ret_min_count = min_count;
	ret_max_count = max_count;
}
#endif /* obsolete */

BlockFadeData::BlockFadeData(int _blocksize_x, int _blocksize_y, int _x0, int _y0, int _width, int _height) {
	diftables = 0; tables = 0; old_tables = 0;
	width = 0; height = 0; blocksize_x = 1; blocksize_y = 1;
	blockwidth = 0; blockheight = 0; table_size = 0;
	next = 0;
	if (_blocksize_x <= 0) return;
	if (_blocksize_y <= 0) return;
	if (_width <= 0) return;
	if (_height <= 0) return;
	x0 = _x0;
	y0 = _y0;
	width = _width;
	height = _height;
	blocksize_x = _blocksize_x;
	blocksize_y = _blocksize_y;
	blockwidth =  (_width +_blocksize_x-1) / _blocksize_x;
	blockheight = (_height+_blocksize_y-1) / _blocksize_y;
	table_size = blockwidth * blockheight;
	tables = new char[table_size];
	old_tables = new char[table_size];
	diftables = new const FadeTableOrig*[table_size];
	memset(old_tables, 0, table_size);
	return;
}
BlockFadeData::~BlockFadeData() {
	if (diftables) delete[] diftables;
	if (tables) delete[] tables;
	if (old_tables) delete[] old_tables;
}

static void CalcDifTable(BlockFadeData* instance) {
	if (instance == 0) return;
	int w = instance->blockwidth;
	int h = instance->blockheight;
	char* tables = instance->tables;
	char* old_tables = instance->old_tables;
	const FadeTableOrig** diftables = instance->diftables;
	int i,j; int k=0;
	int min_x = w, max_x = 0, min_y = -1, max_y = 0;
	int min_y_flag = 1;
	for (i=0; i<h; i++) {
		int min_count = -1, max_count = 0, min_flag = 1;
		for (j=0; j<w; j++) {
			diftables[k] = FadeTableOrig::DifTable(old_tables[k], tables[k]);
			old_tables[k] = tables[k];
			if (diftables[k] == 0) {
				if (min_flag == 1) min_count = i;
			} else {
				max_count = j;
				min_flag = 0;
			}
			k++;
		}
		min_count++;
		if (min_count == w) { 
			if (min_y_flag == 1) min_y = i;
		} else {
			max_y = i;
			min_y_flag = 0;
			if (min_x > min_count) min_x = min_count;
			if (max_x < max_count) max_x = max_count;
		}
	}
	min_y++;
	instance->max_x = max_x;
	instance->min_x = min_x;
	instance->max_y = max_y;
	instance->min_y = min_y;
}

template<class T>
void Drawer<T>::BlockDifDraw(DifImage* image, BlockFadeData* blockdata) {

	int i,j;
	CalcDifTable(blockdata);
	int dest_bpl = image->destbpl;
	int dif_bpl = image->difbpl;
	int x0 = blockdata->x0 + blockdata->min_x * blockdata->blocksize_x;
	int y0 = blockdata->y0 + blockdata->min_y * blockdata->blocksize_y;
	char* dest_buf = image->destimage + x0*ByPP + y0*dest_bpl;
	char* dif_buf  = image->difimage  + x0*DifByPP + y0*dif_bpl;
	const FadeTableOrig** diftable = blockdata->diftables + blockdata->min_x + blockdata->min_y * blockdata->blockwidth;

	int ylen = blockdata->max_y - blockdata->min_y + 1;
	int xlen = blockdata->max_x - blockdata->min_x + 1;
	if (ylen <= 0 || xlen <= 0) return; 
	int cur_y = blockdata->min_y * blockdata->blocksize_y;
	for (i=0; i<ylen; i++) {
		int next_y = cur_y + blockdata->blocksize_y;
		if (next_y > blockdata->height) next_y = blockdata->height;
		for (; cur_y < next_y; cur_y++) {
			char* d = dest_buf;  dest_buf += dest_bpl;
			char* dif = dif_buf; dif_buf += dif_bpl;
			int cur_x = blockdata->min_x * blockdata->blocksize_x;
			for (j=0; j<xlen; j++) {
				const FadeTableOrig* table = diftable[j];
				int next_x = cur_x + blockdata->blocksize_x;
				if (next_x > blockdata->width) next_y = blockdata->width;
				if (table == 0) {
					d += (next_x-cur_x) * ByPP;
					dif += (next_x-cur_x)*DifByPP;
				} else {
					for (; cur_x < next_x; cur_x++) {
						Draw1PixelFromDif(d, dif, table);
						d += ByPP; dif += DifByPP;
					}
				}
			}
		}
		diftable += blockdata->blockwidth;
	}
}
void XXX_this_is_image_di_impl_local_XXX_register_templates(void) {
	DI_Image im;
	Bpp16::MakeDifImage(im,im,0,0);
	Bpp32::MakeDifImage(im,im,0,0);
	Bpp16::CalcKido(0,0,0,0,0);
	Bpp32::CalcKido(0,0,0,0,0);
	Bpp16::BlockDifDraw(0,0);
	Bpp32::BlockDifDraw(0,0);
}
