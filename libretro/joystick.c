// JOYSTICK.C - joystick support for WinX68k

#include "common.h"
#include "prop.h"
#include "joystick.h"
#include "winui.h"
#include "keyboard.h"

#include "libretro.h"
extern retro_input_state_t input_state_cb;
extern DWORD libretro_supports_input_bitmasks;

#ifndef MAX_BUTTON
#define MAX_BUTTON 32
#endif

char joyname[2][MAX_PATH];
char joybtnname[2][MAX_BUTTON][MAX_PATH];
BYTE joybtnnum[2] = {0, 0};

BYTE joy[2];
BYTE JoyKeyState;
BYTE JoyKeyState0;
BYTE JoyKeyState1;
BYTE JoyState0[2];
BYTE JoyState1[2];

// This stores whether the buttons were down. This avoids key repeats.
BYTE JoyDownState0;
BYTE MouseDownState0;

// This stores whether the buttons were up. This avoids key repeats.
BYTE JoyUpState0;
BYTE MouseUpState0;

BYTE JoyPortData[2];

int *r_joy;

void Joystick_Init(void)
{
	joy[0] = 1; // activate JOY1
	joy[1] = 1; // activate JOY2
	JoyKeyState = 0;
	JoyKeyState0 = 0;
	JoyKeyState1 = 0;
	JoyState0[0] = 0xff;
	JoyState0[1] = 0xff;
	JoyState1[0] = 0xff;
	JoyState1[1] = 0xff;
	JoyPortData[0] = 0;
	JoyPortData[1] = 0;
}

void Joystick_Cleanup(void)
{

}

BYTE FASTCALL Joystick_Read(BYTE num)
{
	BYTE joynum = num;
	BYTE ret0 = 0xff, ret1 = 0xff, ret;

	if (Config.JoySwap) joynum ^= 1;
	if (joy[num]) {
		ret0 = JoyState0[num];
		ret1 = JoyState1[num];
	}

	if (Config.JoyKey)
	{
		if ((Config.JoyKeyJoy2)&&(num==1))
			ret0 ^= JoyKeyState;
		else if ((!Config.JoyKeyJoy2)&&(num==0))
			ret0 ^= JoyKeyState;
	}

	ret = ((~JoyPortData[num])&ret0)|(JoyPortData[num]&ret1);

	return ret;
}


void FASTCALL Joystick_Write(BYTE num, BYTE data)
{
	if ( (num==0)||(num==1) ) JoyPortData[num] = data;
}

// Menu navigation related vars
#define RATE   3      // repeat rate
#define DELAY 30      // delay before 1st repeat
BYTE keyb_in, joy_in;

static DWORD get_px68k_input_bitmasks(int port)
{
   return input_state_cb(port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_MASK);
}

static WORD get_px68k_input(int port)
{
   DWORD i = 0;
   DWORD res = 0;
   for (i = 0; i < (RETRO_DEVICE_ID_JOYPAD_R + 1); i++)
      res |= input_state_cb(port, RETRO_DEVICE_JOYPAD, 0, i) ? (1 << i) : 0;
   return res;
}

#define PAD_2BUTTON  0
#define PAD_CPSF_MD  1
#define PAD_CPSF_SFC 2

void FASTCALL Joystick_Update(int is_menu, int key, int port)
{
	BYTE ret0 = 0xff, ret1 = 0xff;
	BYTE mret0 = 0xff, mret1 = 0xff;
	BYTE temp = 0;
	static BYTE pre_ret0 = 0xff, pre_mret0 = 0xff;
	DWORD res = 0;

	if (libretro_supports_input_bitmasks)
		res = get_px68k_input_bitmasks(port);
	else
		res = get_px68k_input(port);

	/* D-Pad */
	if (res & (1 << RETRO_DEVICE_ID_JOYPAD_RIGHT))	temp |= JOY_RIGHT;
	if (res & (1 << RETRO_DEVICE_ID_JOYPAD_LEFT))	temp |=  JOY_LEFT;
	if (res & (1 << RETRO_DEVICE_ID_JOYPAD_UP))		temp |=  JOY_UP;
	if (res & (1 << RETRO_DEVICE_ID_JOYPAD_DOWN))	temp |=  JOY_DOWN;

	if ((temp & (JOY_LEFT | JOY_RIGHT)) == (JOY_LEFT | JOY_RIGHT))
		temp &= ~(JOY_LEFT | JOY_RIGHT);
	if ((temp & (JOY_UP | JOY_DOWN)) == (JOY_UP | JOY_DOWN))
		temp &= ~(JOY_UP | JOY_DOWN);

	ret0 ^= temp;

	/* Buttons */
	switch (Config.JOY_TYPE[port]) {
	case PAD_2BUTTON:
		if (res & (1 << RETRO_DEVICE_ID_JOYPAD_A))	ret0 ^= (Config.VbtnSwap ? JOY_TRG1 : JOY_TRG2);
		if (res & (1 << RETRO_DEVICE_ID_JOYPAD_B))	ret0 ^= (Config.VbtnSwap ? JOY_TRG2 : JOY_TRG1);
		if (res & (1 << RETRO_DEVICE_ID_JOYPAD_X))	ret0 ^= (Config.VbtnSwap ? JOY_TRG2 : JOY_TRG1);
		if (res & (1 << RETRO_DEVICE_ID_JOYPAD_Y))	ret0 ^= (Config.VbtnSwap ? JOY_TRG1 : JOY_TRG2);

		if (res & (1 << RETRO_DEVICE_ID_JOYPAD_START)) ret0 ^= (JOY_UP | JOY_DOWN);
		if (!Config.joy1_select_mapping)
			if (res & (1 << RETRO_DEVICE_ID_JOYPAD_SELECT)) ret0 ^= (JOY_LEFT | JOY_RIGHT);
		break;

	case PAD_CPSF_MD:
		if (res & (1 << RETRO_DEVICE_ID_JOYPAD_A))	ret0 ^= JOY_TRG1;	// Low-Kick
		if (res & (1 << RETRO_DEVICE_ID_JOYPAD_B))	ret0 ^= JOY_TRG2;	// Mid-Kick
		if (res & (1 << RETRO_DEVICE_ID_JOYPAD_X))	ret1 ^= JOY_TRG4; 	// Low-Punch
		if (res & (1 << RETRO_DEVICE_ID_JOYPAD_Y))	ret1 ^= JOY_TRG3;	// Mid-Punch
		if (res & (1 << RETRO_DEVICE_ID_JOYPAD_L))	ret1 ^= JOY_TRG5;	// High-Punch
		if (res & (1 << RETRO_DEVICE_ID_JOYPAD_R))	ret1 ^= JOY_TRG8;	// High-Kick

		if (res & (1 << RETRO_DEVICE_ID_JOYPAD_START))	ret1 ^= JOY_TRG6; // Start
		if (!Config.joy1_select_mapping)
			if (res & (1 << RETRO_DEVICE_ID_JOYPAD_SELECT))	ret1 ^= JOY_TRG7;	// Mode
		break;

	case PAD_CPSF_SFC:
		if (res & (1 << RETRO_DEVICE_ID_JOYPAD_A))	ret0 ^= JOY_TRG2;
		if (res & (1 << RETRO_DEVICE_ID_JOYPAD_B))	ret0 ^= JOY_TRG1;
		if (res & (1 << RETRO_DEVICE_ID_JOYPAD_X))	ret1 ^= JOY_TRG3;
		if (res & (1 << RETRO_DEVICE_ID_JOYPAD_Y))	ret1 ^= JOY_TRG4;
		if (res & (1 << RETRO_DEVICE_ID_JOYPAD_L))	ret1 ^= JOY_TRG8;
		if (res & (1 << RETRO_DEVICE_ID_JOYPAD_R))	ret1 ^= JOY_TRG5;

		if (res & (1 << RETRO_DEVICE_ID_JOYPAD_START))	ret1 ^= JOY_TRG6;
		if (!Config.joy1_select_mapping)
			if (res & (1 << RETRO_DEVICE_ID_JOYPAD_SELECT))	ret1 ^= JOY_TRG7;
		break;
	}

	JoyDownState0 = ~(ret0 ^ pre_ret0) | ret0;
	JoyUpState0 = (ret0 ^ pre_ret0) & ret0;
	pre_ret0 = ret0;

	MouseDownState0 = ~(mret0 ^ pre_mret0) | mret0;
	MouseUpState0 = (mret0 ^ pre_mret0) & mret0;
	pre_mret0 = mret0;

	// disable Joystick when software keyboard is active
	if (!is_menu && !Keyboard_IsSwKeyboard()) {
		JoyState0[port] = ret0;
		JoyState1[port] = ret1;
	}

	/* input overrides section during Menu mode for faster menu browsing
	 * by pressing and holding key or button aka turbo mode */
	if (is_menu) {
		int i;
		static int repeat_rate, repeat_delay;
		static BYTE last_in;
		BYTE inbuf;

		for (i = 0; i < 4; i++)
			speedup_joy[1 << i] = 0;

		joy_in = (ret0 ^ 0xff);
		inbuf = (joy_in | keyb_in);

		if ((inbuf & (JOY_LEFT | JOY_RIGHT)) == (JOY_LEFT | JOY_RIGHT))
			inbuf &= ~(JOY_LEFT | JOY_RIGHT);
		if ((inbuf & (JOY_UP | JOY_DOWN)) == (JOY_UP | JOY_DOWN))
			inbuf &= ~(JOY_UP | JOY_DOWN);

		if (last_in != inbuf) {
			last_in = inbuf;
			repeat_delay = DELAY;
			repeat_rate = 0;
			JoyDownState0 = (inbuf ^ 0xff);
		} else {
			if (repeat_delay)
				repeat_delay--;
			if (repeat_delay == 0) {
				if (repeat_rate)
					repeat_rate--;
				if (repeat_rate == 0) {
					repeat_rate = RATE;
					for (i = 0; i < 4; i++) {
						BYTE tmp = (1 << i); // which direction? UP/DOWN/LEFT/RIGHT
						if ((inbuf & tmp) == tmp)
							speedup_joy[tmp] = 1;
					}
				}
			}
		}
	}
}

BYTE get_joy_downstate(void)
{
	return JoyDownState0;
}
void reset_joy_downstate(void)
{
	JoyDownState0 = 0xff;
}
BYTE get_joy_upstate(void)
{
	return JoyUpState0;
}
void reset_joy_upstate(void)
{
	JoyUpState0 = 0x00;
}
