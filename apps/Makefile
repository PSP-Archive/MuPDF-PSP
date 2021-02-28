TARGET=psppdf
OBJS:=psppdf.o pdfapp.o
OBJS+=fzrefcount.o fzimage.o fzimagepng.o fztexture.o fzscreenpsp.o pg.o filer.o
#OBJS+=tinystr.o tinyxmlerror.o tinyxml.o 

INCDIR =
CFLAGS = -I../include -G0 -Wall -O2 -I/usr/local/pspdev/psp/include/freetype2
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR=../libs
LDFLAGS =
LIBS= -lmupdf -lworld -lfonts -lstream -lraster -lbase -lpspgu -lpng -ljpeg -lz -lm -lfreetype -lstdc++ -lsupc++
#LIBS= -lpspgu -lm -lfreetype -lstdc++ -lsupc++

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = PSPPDF

PSPSDK=$(shell psp-config --pspsdk-path)
#USE_PSPSDK_LIBC=1
include $(PSPSDK)/lib/build.mak
