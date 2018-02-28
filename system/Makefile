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

OBJS_BOOTPACK = bootpack.obj naskfunc.obj hankaku.obj graphic.obj \
	dsctbl.obj int.obj fifo.obj debuger.obj keyboard.obj memory.obj \
	mouse.obj timer.obj utility.obj lyrctl.obj api.obj console.obj \
	filesystem.obj multitask.obj window.obj desktop.obj jpeg.obj

default :
	$(MAKE) img

# .bin

ipl10.bin : ipl10.nas Makefile
	$(NASK) ipl10.nas ipl10.bin ipl10.lst

asmhead.bin : asmhead.nas Makefile
	$(NASK) asmhead.nas asmhead.bin asmhead.lst

hankaku.bin : hankaku.txt Makefile
	$(MAKEFONT) hankaku.txt hankaku.bin

# .obj

hankaku.obj : hankaku.bin Makefile
	$(BIN2OBJ) hankaku.bin hankaku.obj _hankaku

# .bim

bootpack.bim : $(OBJS_BOOTPACK) Makefile
	$(OBJ2BIM) @$(RULEFILE) out:bootpack.bim stack:3136k map:bootpack.map \
		$(OBJS_BOOTPACK)

# .hrb

bootpack.hrb : bootpack.bim Makefile
	$(BIM2HRB) bootpack.bim bootpack.hrb 0

# .sys

haribote.sys : asmhead.bin bootpack.hrb Makefile
	cat asmhead.bin bootpack.hrb > haribote.sys

# .img

haribote.img : ipl10.bin haribote.sys fileT.txt app.bex Makefile
	$(EDIMG)   imgin:../z_tools/fdimg0at.tek \
		wbinimg src:ipl10.bin len:512 from:0 to:0 \
		copy from:haribote.sys to:@: \
		copy from:desktop.jpg to:@: \
		copy from:app.bex to:@:\
		imgout:haribote.img

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

img :
	$(MAKE) haribote.img

run :
	$(MAKE) img
	$(COPY) haribote.img ../z_tools/qemu/fdimage0.bin
	$(QEMU) -L . -m 32 -localtime -vga std -fda $(TOOLPATH)qemu/fdimage0.bin

clean :
	-$(DEL) *.bin
	-$(DEL) *.lst
	-$(DEL) *.obj
	-$(DEL) app.bex
	-$(DEL) bootpack.map
	-$(DEL) bootpack.bim
	-$(DEL) bootpack.hrb
	-$(DEL) haribote.sys
	-$(DEL) haribote.img
	-$(DEL) $(TOOLPATH)qemu/fdimage0.bin
