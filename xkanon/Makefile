PLAT := i686-w64-mingw32-
# i686-w64-mingw32-g++ simple.cc -o simple `i686-w64-mingw32-pkg-config gtkmm-2.4 --cflags --libs`


CC := $(PLAT)gcc 
CPP := $(PLAT)g++
LD := $(PLAT)ld
AR := $(PLAT)ar rcu
# clq 
RANLIB := $(PLAT)ranlib
STRIP := $(PLAT)strip
RM := rm -f

CFLAGS := 
CFLAGS += 
CFLAGS += -DDEFAULT_DATAPATH=\"/usr/local/share/xayusys_gtk2/xkanon\" 
CFLAGS += -DDEFAULT_SAVEPATH=\"~/.xkanon\" -DDEFAULT_VERSION=1 
CFLAGS += -DLOCALEDIR=\"/usr/local/share/locale\" -g3 -O0 
CFLAGS += -DCDROM_LINUX -DHAVE_CONFIG_H 
CFLAGS += -pthread 
CFLAGS += -I/usr/include/gtk-2.0 
CFLAGS += -I/usr/lib/i386-linux-gnu/gtk-2.0/include 
CFLAGS += -I/usr/include/atk-1.0 
CFLAGS += -I/usr/include/cairo 
CFLAGS += -I/usr/include/gdk-pixbuf-2.0 
CFLAGS += -I/usr/include/pango-1.0 
CFLAGS += -I/usr/include/gio-unix-2.0/ 
CFLAGS += -I/usr/include/freetype2 
CFLAGS += -I/usr/include/glib-2.0 
CFLAGS += -I/usr/lib/i386-linux-gnu/glib-2.0/include 
CFLAGS += -I/usr/include/pixman-1 
CFLAGS += -I/usr/include/libpng12 
CFLAGS += -I/usr/include/harfbuzz     
CFLAGS += `i686-w64-mingw32-pkg-config gtkmm-2.4 --cflags --libs`
CFLAGS += -fpermissive -pthread 
CFLAGS += -Wall -Wno-narrowing -O0

CFLAGS2 :=
CFLAGS2 += -I./
CFLAGS2 += -g -O2   
CFLAGS2 += -DCDROM_LINUX -DHAVE_CONFIG_H    
CFLAGS2 += -pthread -O2

LDFLAGS := 
#LDFLAGS += -lgtk-x11-2.0 -lgdk-x11-2.0 
LDFLAGS += -latk-1.0 
LDFLAGS += -lgio-2.0 -lpangoft2-1.0 -lpangocairo-1.0 
LDFLAGS += -lgdk_pixbuf-2.0 -lcairo -lpango-1.0 
LDFLAGS += -lfontconfig -lgobject-2.0 -lglib-2.0 -lfreetype   
LDFLAGS += -lz 
#LDFLAGS += -lXxf86vm   -lX11 -lXext -L/usr/X11R6/lib 
LDFLAGS += `i686-w64-mingw32-pkg-config gtkmm-2.4 --libs`     
LDFLAGS += -pthread -lm

objects :=
objects += main.o file.o initial.o 
objects += image_sys.o image_cursor.o image_stippledef.o image_di.o 
objects += image_icon.o image_di_16bpp.o image_di_32bpp.o image_di_nobpp.o 
objects += image_di_seldraw.o image_di_impl.o image_di_selimpl1.o 
objects += image_di_selimpl2.o image_di_selimpl3.o image_pdt.o 
objects += option.o game_opt.o check_cgm.o 
objects += system.o system_config.o system_graphics.o anm.o ard.o 
objects += senario.o senario_flags.o 
objects += senario_graphics.o 
objects += senario_save.o senario_patch.o senario_backlog.o 
objects += window.o window_text.o window_name.o window_menu.o 
objects += window_all.o contrib/system_music.o

objects2 := 
#objects2 += contrib/music.o 
#objects2 += contrib/pcm.o contrib/movie.o contrib/koedec.o contrib/koedec_wpd.o  
#objects2 += contrib/wavfile.o contrib/wavmix.o contrib/audio.o contrib/audioIO_oss.o 
#objects2 += contrib/LittleEndian.o contrib/counter.o contrib/cdrom.Linux.o contrib/cdrom.o
objects2 += contrib/_fake.o 

BIN := xayusys_gtk2

all : $(BIN)

$(BIN) : $(objects) contrib/contrib.a
	$(CPP) $(CFLAGS) -o $@ $^ $(LDFLAGS)

contrib/contrib.a : $(objects2)
	$(AR) $@ $^
	
contrib/koedec_wpd.o : contrib/koedec_wpd.cc
	$(CPP) -c -o $@ $<
	
%.o : %.cc
	$(CPP) $(CFLAGS) -c -o $@ $<

contrib/%.o : contrib/%.c
	$(CC) $(CFLAGS2) -c -o $@ $<

run: all
	./$(BIN) --path c:/ftp/kanon

clean :
	$(RM) $(objects) $(objects2) *.a contrib/*.a $(BIN)

