[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "app_nask.nas"]

	; System
	GLOBAL 	_api_memory_init
	GLOBAL  _api_malloc
	GLOBAL  _api_free
	GLOBAL	_api_end
	GLOBAL 	_api_getkey

	; Console
	GLOBAL  _api_putchar
	GLOBAL  _api_print

	; GUI
	GLOBAL	_api_win_open
	GLOBAL  _api_win_close
	GLOBAL  _api_win_print
	GLOBAL  _api_win_boxfill
	GLOBAL 	_api_win_dw_point
	GLOBAL  _api_win_dw_line
	GLOBAL  _api_win_refresh_sub

	; TIMERS
	GLOBAL  _api_timer_alloc
	GLOBAL  _api_timer_init
	GLOBAL  _api_timer_countdown
	GLOBAL	_api_timer_free

[SECTION .text]

_api_win_open: ; void api_win_open(int xsize, int ysize, int icol, char *title)
	PUSH	EDI
	PUSH  ESI
	PUSH	EBX
	MOV		EDX, 3
	MOV		EBX, [ESP + 12]
	MOV		ESI, [ESP + 16]
	MOV		EDI, [ESP + 20]
	MOV		EAX, [ESP + 24]
	MOV		ECX, [ESP + 28]
	INT		0x40
	POP		EBX
	POP		ESI
	POP		EDI
	RET

_api_win_close: ; void api_win_close(int win)
	PUSH	EBX
	MOV		EDX, 12
	MOV		EBX, [ESP + 8]
	INT		0x40
	POP		EBX
	RET

_api_win_print: ; void api_win_print(int win, int x, int y, int col, int len, char *str);
	PUSH 	EDI
	PUSH	ESI
	PUSH	EBP
	PUSH	EBX
	MOV		EDX, 4
	MOV		EBX, [ESP + 20]
	MOV		ESI, [ESP + 24]
	MOV		EDI, [ESP + 28]
	MOV		EAX, [ESP + 32]
	MOV		ECX, [ESP + 36]
	MOV		EBP, [ESP + 40]
	INT		0x40
	POP		EBX
	POP		EBP
	POP		ESI
	POP		EDI
	RET

_api_win_boxfill: ; void api_win_boxfill(int win, int x0, int y0, int x1, int y1, int col);
	PUSH 	EDI
	PUSH	ESI
	PUSH	EBP
	PUSH	EBX
	MOV		EDX, 5
	MOV		EBX, [ESP + 20]
	MOV		EAX, [ESP + 24]
	MOV		ECX, [ESP + 28]
	MOV		ESI, [ESP + 32]
	MOV		EDI, [ESP + 36]
	MOV		EBP, [ESP + 40]
	INT		0x40
	POP		EBX
	POP		EBP
	POP		ESI
	POP		EDI
	RET

_api_win_dw_line: ; void api_win_dw_line(int win, int x0, int y0, int x1, int y1, int col)
	PUSH	EDI
	PUSH	ESI
	PUSH	EBP
	PUSH	EBX
	MOV		EDX, 11
	MOV		EBX, [ESP + 20]
	MOV		EAX, [ESP + 24]
	MOV		ECX, [ESP + 28]
	MOV		ESI, [ESP + 32]
	MOV		EDI, [ESP + 36]
	MOV		EBP, [ESP + 40]
	INT		0x40
	POP		EBX
	POP		EBP
	POP		ESI
	POP		EDI
	RET

_api_win_dw_point: ;void api_win_dw_point(int win, int x, int y, int col)
	PUSH	EDI
	PUSH	ESI
	PUSH	EBX
	MOV		EDX, 9
	MOV		EBX, [ESP + 16]
	MOV		ESI, [ESP + 20]
	MOV		EDI, [ESP + 24]
	MOV		EAX, [ESP + 28]
	INT		0x40
	POP		EBX
	POP		ESI
	POP		EDI
	RET

_api_print:  ; void api_print(char *str)
	PUSH	EBX
	MOV		EDX, 2
	MOV		EBX, [ESP + 8]
	INT		0x40
	POP		EBX
	RET

_api_putchar:	; void api_putchar(char c)
	MOV		EDX, 1
	MOV		AL, [ESP + 4]
	INT    	0x40
	RET

_api_memory_init: ;void api_memory_init(void)
	PUSH	EBX
	MOV		EDX, 6
	MOV		EBX, [CS:0x0020]
	MOV		EAX, EBX
	ADD		EAX, 32 * 1024
	MOV		ECX, [CS:0x0000]
	SUB		ECX, EAX
	INT		0x40
	POP		EBX
	RET

_api_malloc: ;char* api_malloc(int size)
	PUSH	EBX
	MOV		EDX, 7
	MOV		EBX, [CS:0x0020]
	MOV		ECX, [ESP + 8]
	INT		0x40
	POP		EBX
	RET

_api_win_refresh_sub: ; void api_win_refresh_sub(int win, int x0, int y0, int x1, int y1)
	PUSH	EDI
	PUSH	ESI
	PUSH	EBX
	MOV		EDX, 10
	MOV		EBX, [ESP + 16]
	MOV		EAX, [ESP + 20]
	MOV		ECX, [ESP + 24]
	MOV		ESI, [ESP + 28]
	MOV		EDI, [ESP + 32]
	INT		0x40
	POP		EBX
	POP		ESI
	POP		EDI
	RET

_api_free: ; void api_free(char *addr, int size)
	PUSH	EBX
	MOV		EDX, 8
	MOV		EBX, [CS:0x0020]
	MOV		EAX, [ESP + 8]
	MOV		ECX, [ESP + 12]
	INT 	0x40
	POP		EBX
	RET

_api_getkey:	; int api_getkey(int mode)
	MOV		EDX, 13
	MOV		EAX, [ESP + 4]
	INT		0x40
	RET

_api_end:	; void api_end(void)
	MOV		EDX, 0
	INT		0x40

_api_timer_alloc:      ; int api_timer_alloc(void);
	MOV		EDX, 14
	INT		0x40
	RET

_api_timer_init:   		 ; void api_timer_init(int timer, int data);
	PUSH	EBX
	MOV		EDX, 15
	MOV		EBX, [ESP + 8]
	MOV		EAX, [ESP + 12]
	INT 	0x40
	POP		EBX
	RET

_api_timer_countdown:  ; void api_timer_countdown(int timer, int time);
	PUSH	EBX
	MOV		EDX, 16
	MOV 	EBX, [ESP + 8]
	MOV		EAX, [ESP + 12]
	INT 	0x40
	POP		EBX
	RET

_api_timer_free:			 ; void api_timer_free(int timer);
	PUSH	EBX
	MOV		EDX, 17
	MOV		EBX, [ESP + 8]
	INT		0x40
	POP		EBX
	RET
