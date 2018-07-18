/*  senario.h
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

#ifndef __KANON_SENARIO_H__
#define __KANON_SENARIO_H__

//#define SENARIO_DEBUG
//#define SENARIO_DUMP

#ifdef SENARIO_DUMP // DUMP mode
#  ifndef SENARIO_DEBUG
#    define SENARIO_DEBUG
#  endif
#  define SUPRESS_JUMP
#  define SUPRESS_KEY
#  define SUPRESS_MUSIC
#  define SUPRESS_GLOBAL_CALL
#  define SUPRESS_WAIT
#  define SUPRESS_RAND
//#define DEBUG_READDATA
//#define DEBUG_DATA
#endif

#ifdef SENARIO_DEBUG
#  define DEBUG_CalcVar
#  define DEBUG_CalcStr
#  define DEBUG_Select
#  define DEBUG_Condition
#  define DEBUG_GraphicsLoad
#  define DEBUG_Graphics
#  define DEBUG_0x0e
#  define DEBUG_TextWindow
#  define DEBUG_Wait
#  define DEBUG_Jump
#  define DEBUG_Other
#  define PrintLineNumber
#  define DEBUG_Graphics2
#endif

#include "file.h"
#include <string.h>
#include "system.h"
#include "ard.h"

#ifndef BACKLOG_LEN
#  define BACKLOG_LEN 65536*16 
#endif
#define LOG_DELETE_LEN 1024*16 

class SENARIO_BackLog {
	class SENARIO_DECODE& decoder;
	class SENARIO_FLAGS* old_flags;
	GosubStack old_stack;
	int& old_track;
	int& old_grp_point;
	int* grp_info; int* grp_hash; int& grp_info_len;
	int grp_info_orig[23];
	char* log; int bottom_point; char* log_orig;
	void Print(void);
	class SENARIO_BackLogInfo* backlog_info;
	char* exec_log_current;

	int PopInt(void); void PushInt(int);
	short PopShort(void); void PushShort(short);
	char PopByte(void); void PushByte(char);
	int PopCmd(void); void PushCmd(int); int GetCmd(void);
	void PopStr(char*,int,int); void PushStr(const char*);
	void PopStrZ(char*,unsigned int); void PushStrZ(const char*);
	char* PopBuffer(int); char* PushBuffer(int);

	void CutLog(void);
	void CutLog(unsigned int len);
	void CheckLogLen(void) {
		if (log < log_orig+LOG_DELETE_LEN) CutLog();
	}
	void CheckLogLen(unsigned int check_len) {
		if (log < log_orig+check_len) CutLog(check_len);
	}
	int CheckNextCmd(void); 
	int CheckPrevCmd(void); 
	void NextSkip(void);
	void PopSkip(void);
public:
	class AyuSys& local_system;
	int GetPoint(void) {
		return log_orig+BACKLOG_LEN-1-log + bottom_point;
	}
	int SetPoint(int point) { 
		if (point < bottom_point) return -1;
		if (point-bottom_point >= BACKLOG_LEN) return -1;
		log = log_orig + BACKLOG_LEN - 1 - (point-bottom_point);
		return 0;
	}
	void CutHash(void);
public:
	SENARIO_BackLog(class SENARIO_DECODE& decoder, class AyuSys&);
	~SENARIO_BackLog();
	GlobalStackItem View(void); 
	void  RestoreState(void); 

	int IsDirty(void);
	int SetFlagInfo(void);
	int SetGraphicsInfo(void);
	int SetMusicInfo(void);
	int SetStackInfo(void);
	int SetSystemInfo(void);
	void DoFlagInfo(void);
	void RestoreGraphicsInfo(void);
	void RestoreMusicInfo(void);
	void RestoreStackInfo(void);
public:
	void Dump(FILE* out, const char* tab, int len);
	void DumpOnecmd(FILE* out, const char* tab);
	void DumpText(FILE* out, const char* tab);
	void DumpSel(FILE* out, const char* tab);
	void DumpSelR(FILE* out, const char* tab);
	void DumpTitle(FILE* out, const char* tab);
	void DumpObsolete(FILE* out, const char* tab);
	void DumpFlagInf(FILE* out, const char* tab);
	void DumpMusicInf(FILE* out, const char* tab);
	void DumpStackInf(FILE* out, const char* tab);
	void DumpSysInf(FILE* out, const char* tab);
	void DumpSysInfOrig(FILE* out, const char* tab);
	void DumpGrpInf(FILE* out, const char* tab);
	void DumpGrpInf2(FILE* out, const char* tab);
public:
	void AddEnd(void);
	void AddEnd2(void);
	void AddEnd3(void);
	void AddRet(void);
	void AddWI(void);
	void AddGameSave(void);
	void AddSysChange(void);
	void AddText(int point, int seen_no, int is_sel);
	void AddText(int point, int seen_no) { AddText(point, seen_no, 0); }
	void AddKoe(int point, int seen_no) { AddText(point, seen_no, 3); }
	void AddSelect1Start(int point, int seen_no) { AddText(point, seen_no, 2); }
	void AddSelect2Start(int point, int seen_no) { AddText(point, seen_no, 1); }
	void AddSelect2(char* str);
	void AddSelect2Result(int result);
	void AddSetTitle(char* s, int point, int seen);
	void AddSavePoint(int p, int seen);
	void PopTextWI(int& p, int& seen, int& flag, int& grp, int& mus, int& stack);
	void DoMsgPos2New(AyuSys& sys);
	void DoMsgPosNew(AyuSys& sys);
	void DoMsgSizeNew(AyuSys& sys);
	void DoMojiSizeNew(AyuSys& sys);
	void DoIsWakuNew(AyuSys& sys);
	void DoTitleNew(AyuSys& sys);
	void DoGrpInfNew(AyuSys& sys);
	void DoMusInfNew(AyuSys& sys);
	void DoFlagInfNew(AyuSys& sys);
	void DoStackInfNew(AyuSys& sys);
	void DoSysInfNew(AyuSys& sys);
	void DoMsgPos2Old(AyuSys& sys);
	void DoMsgPosOld(AyuSys& sys);
	void DoMsgSizeOld(AyuSys& sys);
	void DoMojiSizeOld(AyuSys& sys);
	void DoIsWakuOld(AyuSys& sys);
	void DoTitleOld(AyuSys& sys);
	void DoGrpInfOld(AyuSys& sys);
	void DoMusInfOld(AyuSys& sys);
	void DoFlagInfOld(AyuSys& sys);
	void DoStackInfOld(AyuSys& sys);
	void DoSysInfOld(AyuSys& sys);
	void DoSysOrig(void);
	void DoFlag(int);
	void DoStack(int);
	void DoGraphics(int);
	void DoGraphicsNew(int);
	void DoMusic(int);
	void DoMusicNew(int);
	void DoSystem(int);
	void DoSystemNew(int);
	int CheckLogValid(void); 
	void GetInfo(int& grp, int& mus);
	int SkipNewMessage(void);
	int SkipOldMessage(void);
	int SkipNewMessages(int count, int skip_flag);
	int SkipOldMessages(int count, int skip_flag);
	void GetText(char* ret_str, unsigned int str_len, int* koebuf, int koebuf_len);
	void ResumeOldText(void);
	GlobalStackItem GetSenarioPoint(void);
	void PutLog(char* log, unsigned int len, int is_save);
	void SetLog(char* log, unsigned int len, int is_save);
	void ClearLog(void);
	void StartLog(int is_save);
	void GetSavePoint(int& seen, int& point, class SENARIO_FLAGS** save_flags=0, GosubStack** save_stack=0);
};

class SENARIO_DECODE {
	unsigned char* basedata; 
	unsigned char* data;
	int seen_no;
	int data_len;
	unsigned char cmd;
	class SENARIO& senario;
	class SENARIO_FLAGSDecode& flags;
	class SENARIO_MACRO& macro;
	class SENARIO_Graphics& graphics_save;
	class SENARIO_BackLog backlog;

	void* normal_timer; // for normal wait operation

	static unsigned char hankaku_to_zenkaku_table[0x60 * 2];

	unsigned char* gettext_cache_data; int gettext_cache_number;
public:
	AyuSys& local_system;
	SENARIO_DECODE(int _seen,unsigned char* d, int len, class SENARIO& _senario, class SENARIO_FLAGSDecode& _flag, class SENARIO_MACRO& _mac, class SENARIO_Graphics& _gs, class AyuSys& _sys) : senario(_senario), flags(_flag), macro(_mac), graphics_save(_gs), backlog(*this, _sys), local_system(_sys){
		seen_no = _seen;
		basedata = data = d;
		data_len = len;
		normal_timer = local_system.setTimerBase();
		gettext_cache_data = 0; gettext_cache_number = -1;
	}
	~SENARIO_DECODE() {
		local_system.freeTimerBase(normal_timer);
		if (gettext_cache_data) delete[] gettext_cache_data;
	}
	// int GetTime(void) { return senario.GetTime(); }
	class SENARIO& Senario(void) { return senario; }
	class SENARIO_FLAGSDecode& Flags(void) { return flags; }
	class SENARIO_Graphics& GrpSave(void) { return graphics_save; }
	class SENARIO_BackLog& BackLog(void) { return backlog; }
	class SENARIO_MACRO& Macro(void) { return macro; }

	unsigned char Cmd(void) { return cmd;}; 
	unsigned char NextChar(void) { return *data;}
	unsigned char NextCharwithIncl(void) { return *data++;}
	int ReadInt(void) { data+=4; return read_little_endian_int((char*)(data-4));}
	int ReadData(void); 
	static int ReadDataStatic(unsigned char*&, int& is_var);
	static int ReadDataStatic(unsigned char*&, SENARIO_FLAGSDecode& flags);
	char* ReadString(char*); 
	static char* ReadStringStatic(unsigned char*& senario_data,char*, SENARIO_FLAGSDecode& flags); 

	void ReadStringWithFormat(TextAttribute& text, int is_verbose = 1);

	void SetPoint(int point) {
		if (point >= data_len) point = 0;
		data = basedata + point;
	}
	int GetPoint(void) { return data-basedata; }
	int GetSeen(void) { return seen_no; }
	unsigned char* GetData(int point) { return basedata+point;}
	void GetText(char* ret_str, unsigned int str_len, int* koe, GlobalStackItem& point);

	int Decode(void); 
	void DumpData(void); 

	static char* Han2Zen(const char* str); 

	int Decode_Music(void);
	int Decode_Wait(void); // cmd 0x19
	int Decode_TextWindow(void); // cmd 0x01, 0x02, 0x03, 0xff
	GlobalStackItem Decode_Jump(void);
	int DecodeSkip(char** strlist, int& pt, int max);
	void DecodeSkip_Music(void);
	void DecodeSkip_Wait(void);
	void DecodeSkip_TextWindow(void);
	void DecodeSkip_Jump(void);

	void ReadGrpFile(char** filelist, int deal);
};


class SENARIO_FLAGS {
	unsigned int bit_variables[64]; // 2048 bits
	int variables[2048];
	char string_variables[128][0x40]; // 64 bytes * 128
	static unsigned int bits[32];
	int dirty;
	unsigned int bit_dirty; 
	unsigned int var_dirty[32]; // 
	unsigned int str_dirty[4]; // 128 bits
public:
	int IsDirty(void) { return dirty; }
	void ClearDirty(void);
	int GetBitDirty(int* var_list); 
	int GetVarDirty(int* var_list); 
	int GetStrDirty(int* var_list); 
	void SetBitDirty(int number) {
		number &= 0x1f;
		dirty = 1;
		bit_dirty |= 1U<<number;
	}
	void SetVarDirty(int number) {
		number &= 1023;
		dirty = 1;
		var_dirty[number>>5] |= 1U << (number&0x1f);
	}
	void SetStrDirty(int number) {
		number &= 127;
		dirty = 1;
		str_dirty[number>>5] |= 1U << (number&0x1f);
	}
	// operation of bit variables
	int GetBit(int number) {
		number &= 2047;
		return bit_variables[number>>5] & bits[number&0x1f];
	}
	void SetBit(int number, int bit) {
		if (number > 2000 | number < 0) return;
		number &= 2047;
		if (bit) bit_variables[number>>5] |= bits[number&0x1f];
		else bit_variables[number>>5] &= ~bits[number&0x1f];
		SetBitDirty(number>>5);
	}
	// assume little endian...
	unsigned char GetBitGrp(int num) {
		num &= 2047;
		num /= 8;
		int c = num & 3; int n = num >> 2;
		c *= 8;
		return (bit_variables[n] >> c) & 0xff;
	}
	void SetBitGrp(int num, unsigned char var) {
		num &= 2047;
		num /= 8;
		int c = num & 3; int n = num >> 2;
		c *= 8;
		unsigned int mask = 0xff; mask <<= c;
		unsigned int v = var; v <<= c;
		bit_variables[n] &= ~mask;
		bit_variables[n] |= v;
		SetBitDirty(n);
	}
	unsigned int GetBitGrp2(int num) {
		num &= 63;
		return bit_variables[num];
	}
	void SetBitGrp2(int num, unsigned int var) {
		num &= 63;
		bit_variables[num] = var;
		SetBitDirty(num);
	}
	void SetVar(int number, int var) {
		number &= 2047;
		variables[number] = var;
		SetVarDirty(number);
	}
	int GetVar(int number) {
		number &= 2047;
		return variables[number];
	}
	const char* StrVar(int number) { return string_variables[number]; }
	char* StrVar_nonConst(int number) { SetStrDirty(number); return string_variables[number]; }
	void SetStrVar(int number, const char* src);
	SENARIO_FLAGS(void);
	void Copy(const SENARIO_FLAGS& src);
	~SENARIO_FLAGS();
	int Var(int index) { return variables[index];}
	int Bit(int index) { return GetBit(index);}

};
class SENARIO_FLAGSDecode : public SENARIO_FLAGS {
public:
	//  cmd 0x37 - 0x57
	void DecodeSenario_CalcVar(class SENARIO_DECODE& decoder);
	void DecodeSkip_CalcVar(class SENARIO_DECODE& decoder);

	// cmd 0x59.
	void DecodeSenario_CalcStr(class SENARIO_DECODE& decoder);
	void DecodeSkip_CalcStr(class SENARIO_DECODE& decoder);

	// cmd 0x58.
	void DecodeSenario_Select(class SENARIO_DECODE& decoder);
	void DecodeSkip_Select(class SENARIO_DECODE& decoder);

public:
	// cmd 0x37 - 0x59
	void DecodeSenario_Calc(class SENARIO_DECODE& decoder) {
		if (decoder.Cmd() == 0x59) DecodeSenario_CalcStr(decoder);
		else if (decoder.Cmd() == 0x58) DecodeSenario_Select(decoder);
		else DecodeSenario_CalcVar(decoder);
	}

	void DecodeSkip_Calc(class SENARIO_DECODE& decoder) {
		if (decoder.Cmd() == 0x59) DecodeSkip_CalcStr(decoder);
		else if (decoder.Cmd() == 0x58) DecodeSkip_Select(decoder);
		else DecodeSkip_CalcVar(decoder);
	}

	int DecodeSenario_Condition(class SENARIO_DECODE& decoder, TextAttribute& attr);
	int DecodeSenario_Condition(class SENARIO_DECODE& decoder) {
		TextAttribute attr;
		return DecodeSenario_Condition(decoder, attr);
	}
	int DecodeSkip_Condition(class SENARIO_DECODE& decoder);

	void DecodeSenario_VargroupRead(class SENARIO_DECODE& decoder);
	void DecodeSkip_VargroupRead(class SENARIO_DECODE& decoder);
	void DecodeSenario_VargroupSet(class SENARIO_DECODE& decoder);
	void DecodeSkip_VargroupSet(class SENARIO_DECODE& decoder);
	void DecodeSenario_VargroupCopy(class SENARIO_DECODE& decoder);
	void DecodeSkip_VargroupCopy(class SENARIO_DECODE& decoder);

};

class SENARIO_MACRO {
	unsigned char** macros;
	int macro_deal;
public:
	SENARIO_MACRO(void);
	void SetMacro(int n, const unsigned char* str);
	const unsigned char* GetMacro(int n) {
		if (n < 0 || n >= macro_deal) return 0;
		return macros[n];
	}
	unsigned char* DecodeMacro(unsigned char* str, unsigned char* buf);
	~SENARIO_MACRO();
	void Save(FILE* out);
	void Load(FILE* out);
};

class SENARIO_Graphics;
struct SENARIO_GraphicsSaveBuf {
	int cmd;
	int filedeal;
	char filenames[0x200];
	int args[24];
	int arg2;
	int arg3;
	int arg4[64];
public:
	int StoreLen(void); 
	char* Store(char*); 
	void Restore(const char*); 
	void Dump(FILE*,const char*); 
	int Hash(void); 
	int Compare(const char*); 

	void SaveLoadDraw(char* str, SEL_STRUCT* sel);
	void DoLoadDraw(SENARIO_Graphics& drawer, int is_draw);
	void SaveLoad(char* str, int pdt);
	void DoLoad(SENARIO_Graphics& drawer);
	

	void SaveMultiLoad(int cmd,char* fnames, int fname_deal, int all_len, int sel_no, int* geos);
	void DoMultiLoad(SENARIO_Graphics& drawer, int is_draw);

	void SaveClear(int,int,int,int,int,int,int,int);
	void DoClear(SENARIO_Graphics& drawer);
	void SaveFade(int,int,int,int,int,int,int,int,int);
	void DoFade(SENARIO_Graphics& drawer);

	void SaveCopy(int,int,int,int,int,int,int,int,int);
	void DoCopy(SENARIO_Graphics& drawer);
	void SaveCopyWithMask(int,int,int,int,int,int,int,int,int);
	void DoCopyWithMask(SENARIO_Graphics& drawer);
	void SaveCopyWithoutColor(int,int,int,int,int,int,int,int,int,int,int);
	void DoCopyWithoutColor(SENARIO_Graphics& drawer);
	void SaveSwap(int,int,int,int,int,int,int,int);
	void DoSwap(SENARIO_Graphics& drawer);

	void SaveSaveScreen(void);
	void SaveCopytoScreen(void);
	void DoCopytoScreen(SENARIO_Graphics& drawer,int is_draw);
	int IsSaveScreen(void) { return cmd == 0xaa; }

	void Do(SENARIO_Graphics& drawer, int draw_flag);
	int IsDraw(void);
	SENARIO_GraphicsSaveBuf() {
	}
	void Save(FILE* out);
	int Load(char* buf);
};

class SENARIO_Graphics {
	SENARIO_GraphicsSaveBuf buf[32];
	int deal;
	int changed;
	int saved_count;
	void DeleteBuffer(int n);
	int BufferLength(void) { return deal; }
	int IsTopSaveScreen(void) {
		if (deal == 0) return 0;
		return buf[deal-1].IsSaveScreen();
	}
public:
	AyuSys& local_system;
	SENARIO_GraphicsSaveBuf& Alloc(void);
	void ClearBuffer(void);
	void operator =(SENARIO_Graphics& g) {
		memcpy(buf, g.buf, sizeof(buf));
		deal = g.deal;
	}
	int IsChange(void) { return changed;}
	void ClearChange(void) { changed = 0;}
	void Change(void) { changed = 1; }

	SENARIO_Graphics(AyuSys& sys);
	void DoClear(int,int,int,int,int,int,int,int);
	void DoFade(int,int,int,int,int,int,int,int,int);
	void DoLoadDraw(char* file, struct SEL_STRUCT* sel);
	void DoLoad(char* file, int pdt);
	void DoCopy(int,int,int,int,int,int,int,int,int);
	void DoCopyWithMask(int,int,int,int,int,int,int,int,int);
	void DoCopyWithoutColor(int,int,int,int,int,int,int,int,int,int,int);
	void DoSwap(int,int,int,int,int,int,int,int);
	void Save(FILE* out);
	int Load(char* buf);
	void Restore(int draw_flag = 1); 
	int HashBuffer(void);
	int StoreBufferLen(void);
	void StoreBuffer(char*, int);
	void RestoreBuffer(const char*, int);
	int CompareBuffer(const char*, int);
	void Dump(FILE*,const char*);
	int DecodeSenario_GraphicsLoad(SENARIO_DECODE& decoder);
	void DecodeSkip_GraphicsLoad(SENARIO_DECODE& decoder, char** filelist, int& list_pt, int max);
	int DecodeSenario_Graphics(SENARIO_DECODE& decoder);
	void DecodeSkip_Graphics(SENARIO_DECODE& decoder);
	void DecodeSenario_Fade(SEL_STRUCT* sel, int,int,int);
};

class SENARIO_PATCH {
	char* identifier;
	virtual unsigned char* Patch(int seen_no, unsigned char* origdata, int datalen, int version) = 0; 
	int is_used; 
	SENARIO_PATCH* next;
	static SENARIO_PATCH* head;
public:
	SENARIO_PATCH(char* identifier);
	const char* ID(void) const { return identifier; }
	static void AddPatch(char* identifier); 
	static unsigned char* PatchAll(int seen_no, const unsigned char* origdata, int datalen, int version); 
};

#define SENARIO_GRPREAD 8 
#define MAX_SEEN_NO 1000 
#define MAX_READ_FLAGS 8192 

#if SENARIO_GRPREAD > (MaxPDTImage-2)
#  undef SENARIO_GRPREAD
#  if MaxPDTImage <= 2
#    define SENARIO_GRPREAD 0
#  else
#    define SENARIO_GRPREAD (MaxPDTImage-2)
#  endif
#endif

class SENARIO {
	int seen_no;
	int save_head_size, save_block_size, save_tail_size;
	int isReadHeader;
	unsigned char* data_orig;
	SENARIO_DECODE* decoder;
	AyuSys& local_system;
	GlobalStackItem current_point;
	SENARIO_FLAGSDecode* flags;
	SENARIO_MACRO* macros;
	SENARIO_Graphics* grpsave;
	ARDDAT* arddata;
	AyuSys::GrpFastType old_grp_mode; int old_glen;
	char* old_grp_state; char old_cdrom_track[128]; char old_effec_track[128];

#define READ_FLAG_SIZE (MAX_SEEN_NO+MAX_READ_FLAGS+1+1) 
#define READ_FLAG_MAGIC 0xde491260

	int read_flag_table[MAX_SEEN_NO];
	int max_read_flag_number;
	int read_flags[MAX_READ_FLAGS];
	int max_read_flag;
	void ClearReadFlag(void); 
	void ReadReadFlag(void);
	void WriteReadFlag(void);

	int last_grp_read_point;
	char* grp_read_buf[SENARIO_GRPREAD];
	int grp_read_deal;
	int in_proc; 

	void* basetime;
	char* savefname;
	friend class IdleReadGrp;

	NameSubEntry* name_entry;
public:
	static int* ListSeens(void);
	SENARIO(char* savedir, AyuSys& sys);
	~SENARIO();
	int Init(void); 
	int IsValid(void) { if (data_orig == 0) return false; return true; }
	void ReadGrp(void);
	void CheckGrpMode(void);
	void RestoreGrp();

	void SetReadFlag(int flag_no) {
		if (flag_no > max_read_flag || flag_no < 0) return;
		if (read_flag_table[seen_no] == -1) return;
		int table_no = read_flag_table[seen_no] + flag_no/32;
		read_flags[table_no] |= 1 << (flag_no&0x1f);
	}
	int IsReadFlag(int flag_no) {
		if (flag_no > max_read_flag || flag_no < 0) return 0;
		if (read_flag_table[seen_no] == -1) return 0;
		int table_no = read_flag_table[seen_no] + flag_no/32;
		return (read_flags[table_no] & (1 << (flag_no&0x1f))) != 0;
	}
	void AssignReadFlag(void); 
	void SetMaxReadFlag(int no) { 
		if (max_read_flag < no) max_read_flag = no;
	}

	void ClearArd(void) { if (arddata) delete arddata; arddata = 0; }
	void AssignArd(char* fname) { ClearArd(); arddata = new ARDDAT(fname, local_system); }
	ARDDAT* ArdData(void) { return arddata; }

	int GetTimer(void) { return local_system.getTime(basetime); }
	void InitTimer(void) { local_system.freeTimerBase(basetime); basetime = local_system.setTimerBase(); }

	void SetNameEntry(NameSubEntry* e ) {
		if (name_entry) local_system.CloseNameEntry(name_entry);
		name_entry = e;
	}
	void SetNameToEntry(const char* s) {
		if (name_entry) local_system.SetNameToEntry(name_entry, s);
	}
	const char* GetNameFromEntry(void) {
		if (name_entry == 0) return "";
		else return local_system.GetNameFromEntry(name_entry);
	}
	void CloseNameEntry(void) {
		if (name_entry) local_system.CloseNameEntry(name_entry);
		name_entry = 0;
		return;
	}

	unsigned char* MakeSenarioData(int seen_no, int* slen);
	void PlayFirst(void); 
	GlobalStackItem Play(GlobalStackItem item); 
	char* GetTitle(int seen_no);
	void SetPoint(GlobalStackItem item) { current_point = item; }
	void PlayLast(void);
	void MakeSaveFile(char* dir);
	int IsSavefileExist(void);
	void CreateSaveFile(void);
	void ReadSaveHeader(void);
	void WriteSaveHeader(void);
	void ReadSaveFile(int n, GlobalStackItem& go);
	void WriteSaveFile(int n, char* title);
	int IsValidSaveData(int n, char* title);
	char** ReadSaveTitle(void);
};
class IdleReadGrp : public IdleEvent{
	SENARIO* senario;
public:
	IdleReadGrp(SENARIO* s) : IdleEvent(s->local_system) {
		senario = s;
	}
	int Process(void) {
		senario->ReadGrp();
		return 1;
	}
};

#endif // !defined( __KANON_SENARIO_H__)
