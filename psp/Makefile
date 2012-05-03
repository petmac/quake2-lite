TARGET = Quake2
OBJS = sys_psp.o

INCDIR = 
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

;LIBDIR = $(PSPSDK)/lib
LIBS = -lm
LDFLAGS =

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Quake 2 ported by Peter Mackay

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

deploy: all
	$(MKDIR) -p F:/PSP/GAME/Quake2
	$(CP) EBOOT.PBP F:/PSP/GAME/Quake2/EBOOT.PBP
	$(CP) PARAM.SFO F:/PSP/GAME/Quake2/PARAM.SFO
