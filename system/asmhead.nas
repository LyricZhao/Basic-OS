; haribote-os boot asm
; TAB=4

[INSTRSET "i486p"]

VBEMODE	EQU		0x105
;	0x100 :  640 x  400 x 8
;	0x101 :  640 x  480 x 8
;	0x103 :  800 x  600 x 8
;	0x105 : 1024 x  768 x 8
;	0x107 : 1280 x 1024 x 8

BOTPAK	EQU		0x00280000
DSKCAC	EQU		0x00100000
DSKCAC0	EQU		0x00008000

; BOOT_INFO
CYLS	EQU		0x0ff0
LEDS	EQU		0x0ff1
VMODE	EQU		0x0ff2
SCRNX	EQU		0x0ff4
SCRNY	EQU		0x0ff6
VRAM	EQU		0x0ff8

		ORG		0xc200

; VBE Existence Check

		MOV		AX, 0x9000
		MOV		ES, AX
		MOV		DI, 0
		MOV		AX, 0x4f00
		INT		0x10
		CMP		AX, 0x004f
		JNE		scrn320

; VBE Version Check

		MOV		AX, [ES:DI+4]
		CMP		AX, 0x0200
		JB		scrn320

; Info Get

		MOV		CX, VBEMODE
		MOV		AX, 0x4f01
		INT		0x10
		CMP		AX, 0x004f
		JNE		scrn320

; Info Check

		CMP		BYTE [ES:DI+0x19], 8
		JNE		scrn320
		CMP		BYTE [ES:DI+0x1b], 4
		JNE		scrn320
		MOV		AX, [ES:DI+0x00]
		AND		AX, 0x0080
		JZ		scrn320

; Handoff

		MOV		BX, VBEMODE + 0x4000
		MOV		AX, 0x4f02
		INT		0x10
		MOV		BYTE [VMODE], 8
		MOV		AX, [ES:DI+0x12]
		MOV		[SCRNX], AX
		MOV		AX, [ES:DI+0x14]
		MOV		[SCRNY], AX
		MOV		EAX, [ES:DI+0x28]
		MOV		[VRAM], EAX
		JMP		keystatus

scrn320:
		MOV		AL, 0x13
		MOV		AH, 0x00
		INT		0x10
		MOV		BYTE [VMODE], 8
		MOV		WORD [SCRNX], 320
		MOV		WORD [SCRNY], 200
		MOV		DWORD [VRAM], 0x000a0000

keystatus:

; BIOS tells you the status of LEDs

		MOV		AH, 0x02
		INT		0x16 			; keyboard BIOS
		MOV		[LEDS], AL

; PIC Close All Interrupts

		MOV		AL, 0xff
		OUT		0x21, AL
		NOP						; Some machine can't execute OUT continuously
		OUT		0xa1, AL

		CLI						; Ban interrupts on CPU level

; Let CPU access the address over 1MB (A20GATE)

		CALL	waitkbdout
		MOV		AL, 0xd1
		OUT		0x64, AL
		CALL	waitkbdout
		MOV		AL, 0xdf			; Enable A20
		OUT		0x60, AL
		CALL	waitkbdout

; Protected Virtual Address Mode

[INSTRSET "i486p"]				; Enable i486 Instructions

		LGDT	[GDTR0]			    ; Temporary GDTR
		MOV		EAX, CR0
		AND		EAX, 0x7fffffff	; BIT 31 = 0
		OR		EAX, 0x00000001	; BIT  0 = 1
		MOV		CR0, EAX
		JMP		pipelineflush
pipelineflush:
		MOV		AX, 1 * 8			;  Read/Write Segment
		MOV		DS, AX
		MOV		ES, AX
		MOV		FS, AX
		MOV		GS, AX
		MOV		SS, AX

; copy bootpack

		MOV		ESI, bootpack	; source
		MOV		EDI, BOTPAK		; destination
		MOV		ECX, 512 * 1024 / 4 ; 512 * 1024 Bytes
		CALL	memcpy

; Disk data transfer

; Bootloader

		MOV		ESI, 0x7c00		; source
		MOV		EDI, DSKCAC		; destination
		MOV		ECX, 512 / 4
		CALL	memcpy

; rest

		MOV		ESI, DSKCAC0 + 512	; source
		MOV		EDI, DSKCAC + 512	; destination
		MOV		ECX, 0
		MOV		CL, BYTE [CYLS]
		IMUL	ECX,512 * 18 * 2 / 4	; CYLS -> BYTES / 4
		SUB		ECX,512 / 4		; - IPL
		CALL	memcpy

; asmhead rest
;	later the work belongs to bootpack

; bootpack boot

		MOV		EBX, BOTPAK
		MOV		ECX, [EBX + 16]
		ADD		ECX, 3						; ECX += 3;
		SHR		ECX, 2						; ECX /= 4;
		JZ		skip							; nothing to transfer
		MOV		ESI, [EBX + 20]		; source
		ADD		ESI, EBX
		MOV		EDI, [EBX + 12]		; destination
		CALL	memcpy
skip:
		MOV		ESP,[EBX + 12]
		JMP		DWORD 2 * 8: 0x0000001b

waitkbdout:
		IN		 AL, 0x64
		AND		 AL, 0x02
		JNZ		 waitkbdout
		RET

memcpy:
		MOV		EAX, [ESI]
		ADD		ESI, 4
		MOV		[EDI], EAX
		ADD		EDI, 4
		SUB		ECX, 1
		JNZ		memcpy			; not equal, call memcpy
		RET

		ALIGNB	16
GDT0:
		RESB	8				; NULL selector
		DW		0xffff, 0x0000, 0x9200, 0x00cf	; read/write segment
		DW		0xffff, 0x0000, 0x9a28, 0x0047	; executable segment

		DW		0
GDTR0:
		DW		8*3-1
		DD		GDT0

		ALIGNB	16
bootpack:
