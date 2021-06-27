#ifndef _winx68k_keyboard
#define _winx68k_keyboard

#include "common.h"

#define KeyBufSize 128

extern	BYTE	KeyBuf[KeyBufSize];
extern	BYTE	KeyBufWP;
extern	BYTE	KeyBufRP;
extern	BYTE	KeyTable[512];
extern	BYTE	KeyTableMaster[512];
extern	BYTE	KeyEnable;
extern	BYTE	KeyIntFlag;

struct keyboard_key {
	int x;
	int y;
	int w;
	int h;
	char *s;
	unsigned char c;
};

extern struct keyboard_key kbd_key[];
extern int  kbd_kx, kbd_ky;
extern int kbd_x, kbd_y, kbd_w, kbd_h;

void Keyboard_Init(void);
void Keyboard_KeyDown(DWORD vkcode);
void Keyboard_KeyUp(DWORD vkcode);
void Keyboard_Int(void);
void send_keycode(BYTE code, int flag);
int Keyboard_get_key_ptr(int x, int y);
void Keyboard_skbd(void);
int Keyboard_IsSwKeyboard(void);
void Keyboard_ToggleSkbd(void);

#define	RETROK_XFX	333
/* https://gamesx.com/wiki/doku.php?id=x68000:keycodes */
#define	KBD_XF1		0x55
#define	KBD_XF2		0x56
#define	KBD_XF3		0x57
#define	KBD_XF4		0x58
#define	KBD_XF5		0x59
#define	KBD_F1		0x63
#define	KBD_F2		0x64
#define	KBD_OPT1	0x72
#define	KBD_OPT2	0x73

#endif //_winx68k_keyboard
