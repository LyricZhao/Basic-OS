; Basic-OS - Initial Program Load
; TAB=4

CYLS	EQU		10				; CYLS means "Cylinders", EQU is a command like "# define" in C language

		ORG		0x7c00			; ORG decides where this program will be loaded in RAM

; FAT12 File System Setting

		JMP		entry
		DB		0x90
		DB		"BASICIPL"		; The name of boot part, it must be 8 bytes
		DW		512				    ; The size of each sector, it must be 512 bytes
		DB		1				      ; The size of each cluster, it must be 1 sector
		DW		1				      ; The starting position of the FAT, it usually starts at sector 1
		DB		2				      ; The number of FAT, it must be 2
		DW		224				    ; The size of root directory, it is usually set to 224 entries
		DW		2880			    ; The size of the disk, it must be 2880 sectors
		DB		0xf0			    ; The kind of the disk, it must be 0xf0
		DW		9				      ; The length of FAT, it must be 9 sectors
		DW		18				    ; The number of the sectors of a track, it must be 18
		DW		2				      ; The number of magnetic heads
		DD		0				      ; No partition
		DD		2880			    ; The size of the disk, it must be 2880 sectors(line 18)
		DB		0, 0, 0x29		; Fixed
		DD		0xffffffff		; Volume label
		DB		"BASIC-OS   "	; The name of the disk
		DB		"FAT12   "		; The name of the disk format
		RESB	18				    ; Preset 18 bytes

; IPL Entry

entry:
		MOV		AX,0			; Initialize registers
		MOV		SS,AX
		MOV		SP,0x7c00
		MOV		DS,AX

; Disk Reader

		MOV		AX,0x0820
		MOV		ES,AX
		MOV		CH,0			  ; Cylinder 0
		MOV		DH,0			  ; Magnetic header 0
		MOV		CL,2			  ; Sector 2
readloop:
		MOV		SI,0			  ; The number of failed tries
retry:
		MOV		AH,0x02			; AH=0x02 : Read Disk Code
		MOV		AL,1			  ; The number of the sectors of the element to be processed
		MOV		BX,0
		MOV		DL,0x00			; Disk Drive ID
		INT		0x13			  ; BIOS
		JNC		next			  ; No error
		ADD		SI,1			  ; Failed tries ++
		CMP		SI,5			  ; Failed times >= 5, then jump to error
		JAE		error			  ;
		MOV		AH,0x00
		MOV		DL,0x00			; Disk Drive ID
		INT		0x13			  ; Reset the disk
		JMP		retry
next:
		MOV		AX,ES			  ; Memory Address += 0x20 (ES)
		ADD		AX,0x0020
		MOV		ES,AX
		ADD		CL,1			  ; CL += 1
		CMP		CL,18			  ; Compare CL with 18
		JBE		readloop		; CL <= 18, then jump to readloop
		MOV		CL,1
		ADD		DH,1
		CMP		DH,2
		JB		readloop		; DH < 2, then jump to readloop
		MOV		DH,0
		ADD		CH,1
		CMP		CH,CYLS
		JB		readloop		; CH < CYLS, the jump to readloop

; System Loader

		MOV		[0x0ff0],CH	; Record where IPL has read
		JMP		0xc200

error:
		MOV		SI, msg
putloop:
		MOV		AL,[SI]
		ADD		SI,1			   ; SI += 1
		CMP		AL,0
		JE		fin
		MOV		AH,0x0e			 ; Display
		MOV		BX,15			   ; Color
		INT		0x10			   ; Graphic Card BIOS
		JMP		putloop
fin:
		HLT						     ; CPU Halt
		JMP		fin			     ; Infinite loop
msg:
		DB		0x0a, 0x0a	 ; Wrap twice
		DB		"load error"
		DB		0x0a			   ; Wrap
		DB		0

		RESB	0x7dfe-$		 ;

		DB		0x55, 0xaa
