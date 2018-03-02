TOOLPATH = ../z_tools/
INCPATH  = ../z_tools/haribote/

MAKE     = make
NASK     = $(TOOLPATH)nask
CC1      = $(TOOLPATH)gocc1 -I$(INCPATH) -Os -Wall -quiet
GAS2NASK = $(TOOLPATH)gas2nask -a
BIN2OBJ  = $(TOOLPATH)bin2obj
OBJ2BIM  = $(TOOLPATH)obj2bim
BIM2HRB  = $(TOOLPATH)bim2hrb
RULEFILE = $(TOOLPATH)haribote/haribote.rul
MAKEFONT = $(TOOLPATH)makefont
EDIMG    = $(TOOLPATH)edimg
DEL      = rm -R
COPY     = cp
QEMU     = /usr/local/bin/qemu-system-x86_64

OBJS_BOOTPACK = gobang.obj app_nask.obj

default :
	$(MAKE) clean
	$(MAKE) gobang.bex

# .bim

gobang.bim : $(OBJS_BOOTPACK) Makefile
	$(OBJ2BIM) @$(RULEFILE) out:gobang.bim map:gobang.map \
		$(OBJS_BOOTPACK)

# .hrb

gobang.bex : gobang.bim Makefile
	$(BIM2HRB) gobang.bim gobang.bex 0

# General

# .gas / .nas / .obj
# bootpack, graphic, dstcbl

%.gas : %.c Makefile
	$(CC1) -std=c99 -o $*.gas $*.c

%.nas : %.gas Makefile
	$(GAS2NASK) $*.gas $*.nas

%.obj : %.nas Makefile
	$(NASK) $*.nas $*.obj $*.lst

# Command

clean :
	-$(DEL) *.lst
	-$(DEL) *.obj
	-$(DEL) *.map
	-$(DEL) *.bim
	-$(DEL) *.bex
