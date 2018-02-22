/* asmhead.nas */
# define ADR_BOOTINFO 0x00000ff0
# define ADR_DISKIMG  0x00100000

/* graphic.c */
# define COL8_000000		0
# define COL8_BLACK     0
# define COL8_FF0000		1
# define COL8_RED 			1
# define COL8_00FF00		2
# define COL8_L_GREEN		2
# define COL8_FFFF00		3
# define COL8_YELLOW		3
# define COL8_0000FF		4
# define COL8_BLUE			4
# define COL8_FF00FF		5
# define COL8_PINK		  5
# define COL8_00FFFF		6
# define COL8_Z_GREEN   6
# define COL8_FFFFFF		7
# define COL8_WHITE			7
# define COL8_2D2D2D		8
# define COL8_M_GRAY  	8
# define COL8_840000		9
# define COL8_D_RED  		9
# define COL8_008400		10
# define COL8_D_GREAN		10
# define COL8_848400		11
# define COL8_D_YELLOW	11
# define COL8_000084		12
# define COL8_D_BLUE		12
# define COL8_840084		13
# define COL8_D_PURPLE  13
# define COL8_008484		14
# define COL8_D2_BLUE		14
# define COL8_848484		15
# define COL8_D_GRAY		15

/* dstcbl.c */
#define ADR_IDT			   0x0026f800
#define LIMIT_IDT		   0x000007ff
#define ADR_GDT			   0x00270000
#define LIMIT_GDT		   0x0000ffff
#define ADR_BOTPAK		 0x00280000
#define LIMIT_BOTPAK	 0x0007ffff
#define AR_DATA32_RW	 0x4092
#define AR_CODE32_ER	 0x409a
#define AR_TSS32		   0x0089
#define AR_INTGATE32	 0x008e

/* int.c */
# define PIC0_ICW1		0x0020
# define PIC0_OCW2		0x0020
# define PIC0_IMR		  0x0021
# define PIC0_ICW2		0x0021
# define PIC0_ICW3		0x0021
# define PIC0_ICW4		0x0021
# define PIC1_ICW1		0x00a0
# define PIC1_OCW2		0x00a0
# define PIC1_IMR		  0x00a1
# define PIC1_ICW2		0x00a1
# define PIC1_ICW3		0x00a1
# define PIC1_ICW4		0x00a1

/* keyboard.c */
# define PORT_KEYDAT 						0x0060
# define PORT_KEYSTA				    0x0064
# define PORT_KEYCMD				    0x0064
# define KEYSTA_SEND_NOTREADY	  0x02
# define KEYCMD_WRITE_MODE		  0x60
# define KBC_MODE				        0x47
# define KEYCMD_LED             0xed

/* mouse.c */
# define KEYCMD_SENDTO_MOUSE		0xd4
# define MOUSECMD_ENABLE		   	0xf4

/* memory.c */
# define EFLAGS_AC_BIT			0x00040000
# define CR0_CACHE_DISABLE 	0x60000000
# define MAX_FREE_BLOCK_CNT 4090
# define MEM_MANAGER_ADDR		0x003c0000

/* lyrctl.c */
# define MAX_LAYERS   256
# define LAYER_INUSE 0x01

/* timer.c */
# define MAX_TIMERS		     256
# define PIT_CTRL       0x0043
# define PIT_CNT0       0x0040
# define TIMER_FLAGS_ALLOC   1
# define TIMER_FLAGS_USING   2

/* multitask.c */
# define MAX_LEVELS         10
# define MAX_TASKS_LV      100
# define MAX_TASKS        1000
# define TASK_GDT0           3
# define TASK_FLAGS_ALLOC    1
# define TASK_FLAGS_USING    2 // Running, not current

/* window.h */
# define MAX_WINDOWS       100
