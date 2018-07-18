/*  system.h
 *     
 *     
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


#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif


#ifndef	__KANON_SYSTEM_H__
#define __KANON_SYSTEM_H__
#include<stdio.h>
#include <string.h>


#define DIR_SPLIT '/'	/* UNIX */

class DI_Image;
class DI_ImageMask;

#define LOAD_SENARIO 0x3e8      

/* Subroutine call stack */
class GlobalStackItem {
	int seen_no;
	int local_point;
public:
	void SetLocal(int local) { seen_no = -1; local_point = local;}
	void SetGlobal(int s, int local) { seen_no = s;  local_point = local;}
	int GetLocal(void) { return local_point;}
	int GetSeen(void) { return seen_no; }
	GlobalStackItem() {
		seen_no = -1;
		local_point = -1;
	}
	int IsGlobal(void) { return seen_no != -1; }
	int IsValid(void) { return local_point != -1; }
};

class GosubStack {
	int stack_deal;
	int dirty; 
	int cur_stack_deal;
	GlobalStackItem* stack;
public:
	GosubStack(int num) {
		stack_deal = num; cur_stack_deal = 0;
		stack = new GlobalStackItem[num+1];
		dirty = 0;
	}
	~GosubStack() {
		delete[] stack;
	}
	void ClearDirty(void) { dirty = 0; }
	int IsDirty(void) { return dirty; }
	void InitStack(void) {
		dirty = 1;
		cur_stack_deal = 0;
	}
	void DeleteFirstStack(void) { // delete first stack
		int i; for (i=1; i<stack_deal; i++) {
			stack[i-1] = stack[i];
		}
		cur_stack_deal--;
	}
	GlobalStackItem& PushStack(void) { // return = stack
		dirty = 1;
		if (cur_stack_deal >= stack_deal) DeleteFirstStack();
		return stack[cur_stack_deal++];
	}
	GlobalStackItem PopStack(void) { // return = stack
		dirty = 1;
		if (cur_stack_deal <= 0) {
			stack[0].SetLocal(-1);
			return stack[0];
		}
		cur_stack_deal--;
		GlobalStackItem ret = stack[cur_stack_deal];
		stack[cur_stack_deal].SetLocal(-1);
		return ret;
	}
	GlobalStackItem& operator[] (int n) {
		dirty = 1;
		if (n >= cur_stack_deal) {
			stack[stack_deal].SetLocal(-1);
			return stack[stack_deal];
		} else {
			return stack[n];
		}
	}
	int StackDeal(void) {
		return stack_deal;
	}
	int CurStackDeal(void) {
		return cur_stack_deal;
	}
};

class TextAttribute {
	int attribute;
	int value;
	int condition;
	char* text;
public:
	int Condition(void) { return condition; }
	int Attribute(void) { return attribute; }
	int Value(void) { return value; }
	char* Text(void) { return text; }
	void SetText(char* t) { ReleaseText(); text = new char[strlen(t)+1]; strcpy(text, t); }
	void ReleaseText(void) { if (text) delete[] text; text = 0; }
	void SetAttr(int attr, int v) { attribute = attr; value = v;}
	void SetCondition(int c) { if (c) condition=1; else condition=0; }
	TextAttribute() { attribute = 0; value = 0; text = 0; condition = 0; }
	TextAttribute(char* t) { attribute = 0; value = 0; text = 0; condition = 0; SetText(t); }
	TextAttribute(int attr, int v) { attribute = attr; value = v; condition = 0;}
	~TextAttribute() { ReleaseText(); return; }
};

class TrackName {
	char** track;
	int* track_num;
	char** track_wave;
	int deal;
	void Expand(void);
	char** se_track;
	int se_deal;
	void ExpandSE(int num);
public:
	TrackName(void);
	~TrackName(void);
	void AddCDROM(char* name, int track);
	void AddWave(char* name, char* wave);
	void AddSE(int num, char* se);
	int CDTrack(char* name);
	const char* WaveTrack(char* name);
	const char* SETrack(int num);
};

class IdleEvent {
	IdleEvent* next;
	class AyuSys& local_system;
public:
	IdleEvent(AyuSys& sys) : local_system(sys) {
		next = 0;
	}
	virtual ~IdleEvent();
	void SetNext(IdleEvent* ev) {
		next = ev;
	}
	IdleEvent* Next(void) { return next;}
	virtual int Process(void) = 0;
};

class AyuWindow; 
class SENARIO; 
class NameSubEntry; 

#ifndef PDT_BUFFER_DEAL
#  define PDT_BUFFER_DEAL 32
#endif
#define TMP_PDT_BUFFER (PDT_BUFFER_DEAL-1)
#define WAKU_PDT_BUFFER (PDT_BUFFER_DEAL-2)

#define SEL_DEAL 512
#define SHAKE_DEAL 32
#define COLOR_TABLE_DEAL 30
struct SEL_STRUCT;
struct COLOR_TABLE {
	int c1, c2, c3;
	COLOR_TABLE() { c1=c2=c3=0; }
	void SetColor(int _c1,int _c2,int _c3) { c1=_c1&0xff; c2=_c2&0xff; c3=_c3&0xff;}
};

class AyuSysConfig {
	friend class Conf2; 
	int change_flag;
	int dirty_flag;
	class AyuSysConfigString* str_config;
	class AyuSysConfigIntlist* int_config;
public:
	AyuSysConfig(void);
	int SearchParam(const char* name);

	const char* GetParaStr(const char* name); /* str */
	int GetParam(const char* name, int deal, ...); /* int, error -> return -1, no error -> return 0 */
	int GetParaInt(const char* name) {
		int n;
		if (GetParam(name,1,&n)) return 0;
		return n;
	}

	void SetParaStr(const char* name, const char* var); /* str */
	void SetParam(const char* name, int deal, ...); /* int */
private:
	friend class AyuSys;

	void SetOrigParaStr(const char* name, const char* var); /* str */
	void SetOrigParam(const char* name, int para_deal, ...); /* int */
	void SetOrigParamArray(const char* name, int deal, int* array); 
public:

	void SetOriginal(void);
	int DiffOriginalLen(void);
	char* DiffOriginal(char*);
	const char* PatchOriginal(const char*);
	const char* DumpPatchOriginal(FILE*, const char*, const char*) const;
	void ClearChange(void) { change_flag = 0; }
	int IsChanged(void) { return change_flag; }
	void ClearDiff(void);
	int IsDiff(void) { return dirty_flag; }
	int DiffLen(void);
	char* Diff(char*);
	const char* PatchOld(const char*);
	const char* PatchNew(const char*);
	const char* DumpPatch(FILE*, const char*, const char*) const;
	void Dump(FILE* f) const;
};

class AyuSys {
	int timer_deal; 
	void** timers; 
	int* timer_used; 
	void ExpandTimer(void); 
	void* UnusedTimer(void); 
	void DestroyAllTimer(void); 
	void InitTimer(void);
	void FinalizeTimer(void);

	static void InitRand(void);

	class GosubStack call_stack;

	// intterupt
	int intterupted;

	int backlog_count;

	TrackName track_name;
	int is_cdrom_track_changed;
	int music_mode; // continue mode / once mode
	int movie_id; // movie window id
#define MUSIC_ONCE 1
#define MUSIC_CONT 2

	int MusicStatusLen(void);
	char* MusicStatusStore(char* buf, int len);
	char* MusicStatusRestore(char* buf);

	char cdrom_track[128]; char effec_track[128];
	int music_enable;
	int koe_mode;

	DI_ImageMask* orig_pdt_buffer_orig[PDT_BUFFER_DEAL+1];
	DI_ImageMask* orig_pdt_buffer[PDT_BUFFER_DEAL+1];
	DI_ImageMask* orig_pdt_image[PDT_BUFFER_DEAL+1];
	DI_ImageMask** pdt_buffer_orig;
	DI_ImageMask** pdt_buffer;
	DI_ImageMask** pdt_image;
	DI_ImageMask* anm_pdt; 
	int pdt_bypp;
	int scn_w, scn_h;
	SEL_STRUCT* sels[SEL_DEAL];
	int* shake[SHAKE_DEAL];
	COLOR_TABLE colors[COLOR_TABLE_DEAL];
	COLOR_TABLE fades[COLOR_TABLE_DEAL];
	class Gdk_Visual* visual;
	class PDT_Reader* pdt_reader;
	char* title;
	
	int text_wait;
	
	int TextStatusStoreLen(void);
	char* TextStatusStore(char*, int);
	char* TextStatusRestore(char*);

	class IdleEvent* idle_event;
	int stop_flag;

	char* ini_macroname[26];
	int mouse_pos_x ,mouse_pos_y; 

	int goto_senario;

	int text_fast_mode;

	int debug_flag;

	int version;
public:
	AyuSysConfig* config;
	AyuWindow* main_window;
	SENARIO* main_senario;

	int Version() {
		if (version<0)
#ifdef DEFAULT_VERSION
			return DEFAULT_VERSION;
#else
			return 1;
#endif
		else return version;
	}
	void SetVersion(int v) { version = v; }

	void* setTimerBase(void); 
	void waitUntil(void* handle, int msec); 
	int getTime(void* handle); // time from setTimeBase(msec)
	void freeTimerBase(void* handle); 

	void SetStopProcess(int state) { stop_flag = state; }

	static int Rand(int max);

	class GosubStack& CallStack() { return call_stack; }

	// Intterupt current operation
	void Intterupt(void) { intterupted = 1; backlog_count = 0; stop_flag = 0;}
	void NoMaskableIntterupt(void) { Intterupt(); intterupted = 2; }
	int IsIntterupted(void) { return intterupted; }
	void ClearIntterupt(void) {if (intterupted == 1) { backlog_count = 0; intterupted = 0; }}

	// backlog
	void SetBacklog(int count);
	int GetBacklog(void);

	void SetCDROMDevice(char* dev);
	void SetPCMDevice(char* dev);
	void SetPCMRate(int rate);
	void SetMixDevice(char* dev);
	void PlayCDROM(char* track);
	void StopCDROM(void);
	void FadeCDROM(int time);
	void WaitStopCDROM(void);
	int IsTrackChange(void) { return is_cdrom_track_changed;}
	void ClearTrackChange(void) { is_cdrom_track_changed = 0; }
	char* GetCDROMTrack(void) { return cdrom_track; }
	char* GetEffecTrack(void) { return effec_track; }
	void PlayMovie(char* fname, int x1, int y1, int x2, int y2,int loop_count);
	void StopMovie(void);
	void PauseMovie(void);
	void ResumeMovie(void);
	void WaitStopMovie(int is_click);
	void InitMusic(void);
	void FinalizeMusic(void);
	void DisableMusic(void);
	void SetWaveMixer(int is_mix);
	void SyncMusicState(void);
	void ReceiveMusicPacket(void);
	void SetCDROMOnce(void) { is_cdrom_track_changed = 1; music_mode &= ~0x7; music_mode |= (MUSIC_ONCE);}
	void SetCDROMCont(void) { is_cdrom_track_changed = 1; music_mode &= ~0x7; music_mode |= (MUSIC_CONT);}
	int GetCDROMMode(void) { return music_mode & 7; }
	void SetEffecOnce(void) { is_cdrom_track_changed = 1;music_mode &= ~0x38; music_mode |= (MUSIC_ONCE<<3);}
	void SetEffecCont(void) { is_cdrom_track_changed = 1;music_mode &= ~0x38; music_mode |= (MUSIC_CONT<<3);}
	int GetEffecMode(void) { return (music_mode>>3)&7; }
	void SetUseBGM(void) { music_mode |= 0x40; }
	void SetUseCDROM(void) { music_mode &= ~0x40; }
	int IsUseBGM(void) { return (music_mode & 0x40)>>6; }

	void PlayWave(char* fname);
	void StopWave(void);
	void WaitStopWave(void);
	void PlayKoe(const char* fname);
	void StopKoe(void);
	bool IsStopKoe(void);
	void SetKoeMode(int mode) { if (mode) koe_mode = 1; else koe_mode = 0;}
	void PlaySE(int number);
	void StopSE(void);
	void WaitStopSE(void);
	DI_ImageMask* ScreenImage(void) { return pdt_buffer[0];}
	int SyncPDT(int pdt_num); 
	int DisconnectPDT(int pdt_num); 
	void DeleteAllPDT(void); 
	int DeletePDT(int pdt_num); 
  	int CheckPDT(int pdt_num); 
	int GetUnusedPDT(void); 
	void ClearAllPDT(void); 
	DI_ImageMask* ReadPDTFile(char* f);
	void PrereadPDTFile(char* f);
	void SetPDTUsed(void);
	int IsPDTUsed(int pdt_num);
	class Gdk_Visual* Visual(void) { return visual; }
	int DefaultBypp(void) { return pdt_bypp; }
	int DefaultScreenWidth(void) { return scn_w;}
	int DefaultScreenHeight(void) { return scn_h;}
	void SetDefaultScreenSize(int w, int h);

	int SetDefaultBypp_565(void) { int old = pdt_bypp; pdt_bypp = 2; return old;}
	int SetDefaultBypp_32(void) {int old = pdt_bypp;  pdt_bypp = 4; return old;}
	void ClearPDTBuffer(int n, int c1, int c2, int c3);
	void ClearPDTRect(int n, int x1, int y1, int x2, int y2, int c1, int c2, int c3);
	void ClearPDTWithoutRect(int n, int x1, int y1, int x2, int y2, int c1, int c2, int c3);
	bool check_para_sg(int& src_x, int& src_y, int& src_x2, int& src_y2, int& src_pdt,
		int& dest_x, int& dest_y, int& dest_pdt, int& width, int& height);
	void CopyPDTtoBuffer(int src_x, int src_y, int src_x2, int src_y2, int src_pdt,
		int dest_x, int dest_y, int dest_pdt, int flag);
	void SwapBuffer(int src_x, int src_y, int src_x2, int src_y2, int src_pdt,
		int dest_x, int dest_y, int dest_pdt);
	void CopyBuffer(int src_x, int src_y, int src_x2, int src_y2, int src_pdt,
		int dest_x, int dest_y, int dest_pdt, int count);
	void CopyWithoutColor(int src_x, int src_y, int src_x2, int src_y2, int src_pdt,
		int dest_x, int dest_y, int dest_pdt, int c1, int c2, int c3);
	void StretchBuffer(int src_x, int src_y, int src_x2, int src_y2, int src_pdt,
		int dest_x, int dest_y, int dest_x2, int dest_y2, int dest_pdt);
	void FadePDTBuffer(int pdt_number, int x1, int y1, int x2, int y2, 
		int c1, int c2, int c3, int count);
	void ChangeMonochrome(int pdt, int x1, int y1, int x2, int y2);
	void InvertColor(int pdt, int x1, int y1, int x2, int y2);
	void BlinkScreen(int c1, int c2, int c3, int wait_time, int count);
	void DrawTextPDT(int x, int y, int pdt, const char* text, int c1, int c2, int c3);
	void DeletePDTMask(int pdt_number);
	void DrawPDTBuffer(int pdt_number, SEL_STRUCT* sel);
	SEL_STRUCT* DrawSel(int sel_no) {
		if (sel_no < 0 || sel_no >= SEL_DEAL) sel_no = 0;
		if (sels[sel_no] == 0) sel_no = 0;
		return sels[sel_no];
	}
	void LoadPDTBuffer(int pdt_number, char* path);
	int PDTWidth(int pdt_number);
	int PDTHeight(int pdt_number);
	void LoadToExistPDT(int pdt_number, char* path, int x1, int y1, int x2, int y2, int x3, int y3, int fade = -1);
	void LoadAnmPDT(char* path);
	void ClearAnmPDT(void);
	void DrawAnmPDT(int dest_x, int dest_y, int src_x, int src_y, int width, int height);
	void FlushScreen(void);
	void Shake(int num);
	int GetWindowID(void);
	int MakePartWindow(int x, int y, int w, int h);
	void DeletePartWindow(int id);

	COLOR_TABLE& ColorTable(int n) { if (n>=0 && n<COLOR_TABLE_DEAL) return colors[n];  return colors[0];}
	COLOR_TABLE& FadeTable(int n) { if (n>=0 && n<COLOR_TABLE_DEAL) return fades[n];  return fades[0];}

	void DrawText_(char* str);
	void SetDrawedText(char* str);
	int DrawTextEnd(int flag);
	void DeleteText(void);
	int SelectItem(TextAttribute* texts, int item, int select_type);
	void DrawTextWindow(void);
	void DeleteTextWindow(void);
	void SetTextSpeed(int char_num) { text_wait = 1000/char_num; } 
	int TextSpeed(void) { return text_wait; }
	void DrawReturnCursor(int type);
	void DeleteReturnCursor(void);
	int SaveData(void) { 
		if (goto_senario >= LOAD_SENARIO*2) return goto_senario-LOAD_SENARIO*2;
		else return -1;
	}
	int LoadData(void) { 
		if (goto_senario >= LOAD_SENARIO*2 ||
			goto_senario < LOAD_SENARIO) return -1;
		return goto_senario-LOAD_SENARIO;
	}
	int GoSenario(void) { 
		if (goto_senario < LOAD_SENARIO) return goto_senario;
		else return -1;
	}
	void ClearSenarioNumber(void) {
		goto_senario = -1;
	}
	void SetSenarioNumber(int n) {
		if (n < 0 || n > LOAD_SENARIO*3) goto_senario = -1;
		else goto_senario = n;
	}
	void SetLoadData(int n) { 
		if (n < 0 || n > LOAD_SENARIO) return;
		Intterupt();
		SetSenarioNumber(n+LOAD_SENARIO);
	}
	void SetSaveData(int n) { 
		if (n < 0 || n > LOAD_SENARIO) return;
		Intterupt();
		SetSenarioNumber(n+LOAD_SENARIO*2);
	}
	void SetGoMenu(void) { 
		Intterupt();
		StopCDROM(); StopWave();
		int seen_no = config->GetParaInt("#SEEN_MENU");
		if (seen_no == 0) seen_no = config->GetParaInt("#SEEN_START");
		if (seen_no == 0) seen_no = config->GetParaInt("#SEEN_SRT");
		if (seen_no == 0) seen_no = 1;
		SetSenarioNumber(seen_no);
	}

	
	void CallUpdateFunc(void); 
	void CallUpdateFunc(int x1, int y1, int x2, int y2); 
	void CallProcessMessages(void); 
	void WaitNextEvent(void);
	void CallIdleEvent(void); 
	void SetIdleEvent(IdleEvent* ev); 
	void DeleteIdleEvent(IdleEvent* ev);

	void InitWindow(AyuWindow* main);
	void FinalizeWindow(void);
	void InitPDTBuffer(class Gdk_Visual*, class Gdk_Image*);
	void FinalizePDTBuffer(void);

	void GetMouseInfo(int& x, int& y, int& clicked);
	void ClearMouseInfo(void);
	void SetMouseMode(int is_use_key);
	void GetMouseInfoWithClear(int& x, int& y, int& clicked) {
		GetMouseInfo(x,y,clicked);
		ClearMouseInfo();
	}
	void GetKeyCursorInfo(int& left, int& right, int& up, int& down, int& esc);
	void DrawMouse(void);
	void DeleteMouse(void);
	int MouseCursorX(void) { return mouse_pos_x; }
	int MouseCursorY(void) { return mouse_pos_y; }
	void SetMouseCursorPos(int x, int y) { mouse_pos_x = x; mouse_pos_y = y; }
	bool LoadInitFile(void);
	void DestroyWindow(void) {
		NoMaskableIntterupt();
		main_window = 0;
	}
	void Finalize(void); 
	void SetTitle(char* title);
	char* GetTitle(void) {
		return title;
	}
	struct NameInfo {
		const char* title;
		const char* old_name;
		int name_index;
		char* new_name;
		NameInfo() {
			title = 0; old_name = 0; name_index = -1; new_name = 0;
		}
		void SetInfo(const char* t, const char* o, int n) {
			title = t; old_name = o; name_index = n; new_name = 0;
		}
		~NameInfo() {
			if (new_name) delete[] new_name;
		}
	};
	int OpenNameDialog(NameInfo* names, int list_deal);
	NameSubEntry* OpenNameEntry(int x, int y, int width, int height, const COLOR_TABLE& fore_color, const COLOR_TABLE& back_color);
	void SetNameToEntry(NameSubEntry* entry, const char* name);
	const char* GetNameFromEntry(NameSubEntry* entry);
	void CloseNameEntry(NameSubEntry* entry);
	void CloseAllNameEntry(void);

	void InitConfig(void);
	char* IniMacroName(int n) { if (n<0 || n>26) return 0; return ini_macroname[n];}

	void SetMainSenario(SENARIO* new_s) {
		main_senario = new_s;
	}
	SENARIO* MainSenario(void) {
		return main_senario;
	}
	int SelectLoadWindow(); 
	enum TextFastState { TF_NORMAL, TF_FAST, TF_AUTO, TF_SKIP };
	void ChangeMenuTextFast(void);
	TextFastState TextFastMode(void);
	bool is_pressctrl;
	void PressCtrl(void) { is_pressctrl = true; }
	void ReleaseCtrl(void) {
		is_pressctrl = false;
		RestoreGrp();
	}
	bool IsPressCtrl(void) { return is_pressctrl;}

	bool is_allskip;
	void SetForceFast(int state) {
		if (state) is_allskip = true;
		else {
			TextFastState state = TextFastMode();
			is_allskip = false;
			if (state != TextFastMode() && TextFastMode() == TF_NORMAL) RestoreGrp();
		}
	}
	bool now_in_kidoku;
	void SetKidoku(void) {now_in_kidoku = true;}
	bool NowInKidoku(void) { return now_in_kidoku || is_allskip; }
	void ResetKidoku(void) {
		now_in_kidoku = false;
		RestoreGrp();
	}

	enum CLICKEVENT { NO_EVENT=1, END_TEXTFAST=2} click_event_type;
	bool is_text_fast;
	bool is_text_auto;
	bool is_text_dump;
	bool is_restoring_grp;
	void SetTextFastMode(bool mode) {
		is_text_fast = mode;
		RestoreGrp();
		ChangeMenuTextFast();
	}
	void SetTextAutoMode(bool mode) {
		is_text_auto = mode;
		RestoreGrp();
		ChangeMenuTextFast();
	}
	void SetIsRestoringFlag(bool mode) {
		is_restoring_grp = mode;
	}
	bool IsTextFast(void) { return is_text_fast;}
	bool IsTextAuto(void) { return is_text_auto;}
	void SetTextDump(void) {
		is_text_dump = true;
	}
	bool IsDumpMode(void) {
		return is_text_dump;
	}
	int text_skip_count;
	int text_skip_count_end;
	enum SKIPTYPE { SKIPCOUNT, SKIPSELECT, SKIPSCENE} text_skip_type;
	void StopTextSkip(void) {
		text_skip_count = -1;
		RestoreGrp();
		ChangeMenuTextFast();
	}
	void StartTextSkipMode(int count);
	void InclTextSkipCount(void);
	void TitleEvent(void);
	void SelectEvent(void);
	void ClickEvent(void);
	void SetClickEvent(CLICKEVENT ev) {
		click_event_type = ev;
	}

	enum GrpFastType {
		GF_Normal=0, GF_Fast=1, GF_NoEff=2, 
		GF_NoGrp=3} grp_fast_mode;
	void SetGrpFastMode(GrpFastType gmode) {
		grp_fast_mode = gmode;
	}
	GrpFastType GrpFastMode(void) {
		if (is_restoring_grp) return GF_Normal;
		TextFastState state = TextFastMode();
		if (state == TF_NORMAL || state == TF_AUTO) return GF_Normal;
		else if (state == TF_FAST) {
			return grp_fast_mode;
		} else if (state == TF_SKIP) {
			if (grp_fast_mode == GF_Normal || grp_fast_mode == GF_Fast) return GF_NoEff;
			else return grp_fast_mode;
		} else return grp_fast_mode;
	}
	void RestoreGrp(void);

	void MakePopupWindow(void);

	void ShowMenuItem(const char* items, int active); 
	void UpdateMenu(char** save_titles); 

private:
	void SetDebug(int n) { if (n >= 32 || n < 0) return;
		debug_flag |= 1<<n;
	}
	void ResetDebug(int n) { if (n >= 32 || n < 0) return;
		debug_flag &= ~(1<<n);
	}
	int IsDebug(int n) { if (n >= 32 || n < 0) return 0;
		return debug_flag & (1<<n);
	}
public:
	void SetRandomSelect(int state) { if (state) SetDebug(2); else ResetDebug(2);}
	int IsRandomSelect(void) { return IsDebug(2); }
	void SetGraphicEffectOff(int state) { if (state) SetDebug(1); else ResetDebug(1);}
	int IsGraphicEffectOff(void) { return IsDebug(1); }

	int StatusStoreLen(void);
	char* StatusStore(char*,int);
	char* StatusRestore(char*);

	AyuSys(void);
	~AyuSys(void);
	
};
inline int AyuSys::GetBacklog(void) { return backlog_count; }
extern AyuSys global_system;

extern void kconv(const unsigned char* sjis_src, unsigned char* euc_dest);
extern void kconv_rev(const unsigned char* sjis_src, unsigned char* euc_dest);

void parse_option(int* argc, char*** argv, AyuSys& local_system,
		  char** fontname, int* _fontsize, char** _savepath);
void set_game(const char* key, AyuSys& local_system);

/* config.h */
#if HAVE_MKDIR == 0
extern "C" int mkdir(const char *, unsigned short);
#endif
#if HAVE_SNPRINTF == 0
extern "C" int snprintf(char *str, size_t size, const char *format, ...);
#endif

#endif /* !defined(__KANON_SYSTEM_H__) */
