// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 2006-2015 by The Odamex Team.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	SDL input handler
//
//-----------------------------------------------------------------------------

// SoM 12-24-05: yeah... I'm programming on christmas eve.
// Removed all the DirectX crap.

#include <stdlib.h>
#include <list>
#include <sstream>

#include <SDL.h>
#include "win32inc.h"

#include "doomstat.h"
#include "m_argv.h"
#include "i_input.h"
#include "i_sdlinput.h"
#include "i_video.h"
#include "d_main.h"
#include "c_bind.h"
#include "c_console.h"
#include "c_cvars.h"
#include "i_system.h"
#include "c_dispatch.h"

#ifdef _XBOX
#include "i_xbox.h"
#endif

#ifdef _WIN32
#include <SDL_syswm.h>
bool tab_keydown = false;	// [ML] Actual status of tab key
#endif

#define JOY_DEADZONE 6000

EXTERN_CVAR (vid_fullscreen)
EXTERN_CVAR (vid_defwidth)
EXTERN_CVAR (vid_defheight)

static int mouse_driver_id = -1;
static MouseInput* mouse_input = NULL;

static bool window_focused = false;
static bool input_grabbed = false;
static bool nomouse = false;
extern bool configuring_controls;

EXTERN_CVAR (use_joystick)
EXTERN_CVAR (joy_active)

typedef struct
{
	SDL_Event	Event;
	unsigned int RegTick;
	unsigned int LastTick;
} JoystickEvent_t;

static SDL_Joystick *openedjoy = NULL;
static std::list<JoystickEvent_t*> JoyEventList;

// denis - from chocolate doom
//
// Mouse acceleration
//
// This emulates some of the behavior of DOS mouse drivers by increasing
// the speed when the mouse is moved fast.
//
// The mouse input values are input directly to the game, but when
// the values exceed the value of mouse_threshold, they are multiplied
// by mouse_acceleration to increase the speed.
EXTERN_CVAR (mouse_acceleration)
EXTERN_CVAR (mouse_threshold)

extern constate_e ConsoleState;



static ISDL12KeyboardInputDevice* sdl_keyboard_input_device = NULL;

//
// I_FlushInput
//
// Eat all pending input from outside the game
//
void I_FlushInput()
{
	SDL_Event ev;

	I_DisableKeyRepeat();

	while (SDL_PollEvent(&ev)) {}

	C_ReleaseKeys();

	I_EnableKeyRepeat();

	if (mouse_input)
		mouse_input->flushEvents();
}

void I_EnableKeyRepeat()
{
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY / 2, SDL_DEFAULT_REPEAT_INTERVAL);
}

void I_DisableKeyRepeat()
{
	SDL_EnableKeyRepeat(0, 0);
}

void I_ResetKeyRepeat()
{
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
}

//
// I_CheckFocusState
//
// Determines if the Odamex window currently has the window manager focus.
// We should have input (keyboard) focus and be visible (not minimized).
//
static bool I_CheckFocusState()
{
	SDL_PumpEvents();
	Uint8 state = SDL_GetAppState();
	return (state & SDL_APPACTIVE) && (state & SDL_APPINPUTFOCUS);
}

//
// I_UpdateFocus
//
// Update the value of window_focused each tic and in response to SDL
// window manager events.
//
// We try to make ourselves be well-behaved: the grab on the mouse
// is removed if we lose focus (such as a popup window appearing),
// and we dont move the mouse around if we aren't focused either.
// [ML] 4-2-14: Make more in line with EE and choco, handle alt+tab focus better
//
static void I_UpdateFocus()
{
	SDL_Event  ev;
	bool new_window_focused = I_CheckFocusState();

	// [CG][EE] Handle focus changes, this is all necessary to avoid repeat events.
	if (window_focused != new_window_focused)
	{
		if(new_window_focused)
		{
			while(SDL_PollEvent(&ev)) {}
			I_EnableKeyRepeat();
		}
		else
		{
			I_DisableKeyRepeat();
		}

#ifdef _WIN32
		tab_keydown = false;
#endif
		C_ReleaseKeys();

		window_focused = new_window_focused;

		if (mouse_input)
			mouse_input->flushEvents();
	}
}


//
// I_CanGrab
//
// Returns true if the input (mouse & keyboard) can be grabbed in
// the current game state.
//
static bool I_CanGrab()
{
	#ifdef GCONSOLE
	return true;
	#endif

	if (!window_focused)
		return false;
	else if (I_GetWindow()->isFullScreen())
		return true;
	else if (nomouse)
		return false;
	else if (configuring_controls)
		return true;
	else if (menuactive || ConsoleState == c_down || paused)
		return false;
	else if ((gamestate == GS_LEVEL || gamestate == GS_INTERMISSION) && !demoplayback)
		return true;
	else
		return false;
}


//
// I_GrabInput
//
static void I_GrabInput()
{
	SDL_WM_GrabInput(SDL_GRAB_ON);
	input_grabbed = true;
	I_ResumeMouse();
}

//
// I_UngrabInput
//
static void I_UngrabInput()
{
	SDL_WM_GrabInput(SDL_GRAB_OFF);
	input_grabbed = false;
	I_PauseMouse();
}


//
// I_ForceUpdateGrab
//
// Determines if SDL should grab the mouse based on the game window having
// focus and the status of the menu and console.
//
// Should be called whenever the video mode changes.
//
void I_ForceUpdateGrab()
{
	window_focused = I_CheckFocusState();

	if (I_CanGrab())
		I_GrabInput();
	else
		I_UngrabInput();
}


//
// I_UpdateGrab
//
// Determines if SDL should grab the mouse based on the game window having
// focus and the status of the menu and console.
//
static void I_UpdateGrab()
{
#ifndef GCONSOLE
	// force I_ResumeMouse or I_PauseMouse if toggling between fullscreen/windowed
	bool fullscreen = I_GetWindow()->isFullScreen();
	static bool prev_fullscreen = fullscreen;
	if (fullscreen != prev_fullscreen) 
		I_ForceUpdateGrab();
	prev_fullscreen = fullscreen;

	// check if the window focus changed (or menu/console status changed)
	if (!input_grabbed && I_CanGrab())
		I_GrabInput();
	else if (input_grabbed && !I_CanGrab())
		I_UngrabInput();
#endif
}


//
// I_InitFocus
//
// Sets the initial value of window_focused.
//
static void I_InitFocus()
{
	I_ForceUpdateGrab();
}


// Add any joystick event to a list if it will require manual polling
// to detect release. This includes hat events (mostly due to d-pads not
// triggering the centered event when released) and analog axis bound
// as a key/button -- HyperEye
//
// RegisterJoystickEvent
//
static int RegisterJoystickEvent(SDL_Event *ev, int value)
{
	JoystickEvent_t *evc = NULL;
	event_t		  event;

	if(!ev)
		return -1;

	if(ev->type == SDL_JOYHATMOTION)
	{
		if(!JoyEventList.empty())
		{
			std::list<JoystickEvent_t*>::iterator i;

			for(i = JoyEventList.begin(); i != JoyEventList.end(); ++i)
			{
				if(((*i)->Event.type == ev->type) && ((*i)->Event.jhat.which == ev->jhat.which)
							&& ((*i)->Event.jhat.hat == ev->jhat.hat) && ((*i)->Event.jhat.value == value))
					return 0;
			}
		}

		evc = new JoystickEvent_t;

		memcpy(&evc->Event, ev, sizeof(SDL_Event));
		evc->Event.jhat.value = value;
		evc->LastTick = evc->RegTick = SDL_GetTicks();

		event.data1 = event.data2 = event.data3 = 0;

		event.type = ev_keydown;
		if(value == SDL_HAT_UP)
			event.data1 = (ev->jhat.hat * 4) + KEY_HAT1;
		else if(value == SDL_HAT_RIGHT)
			event.data1 = (ev->jhat.hat * 4) + KEY_HAT2;
		else if(value == SDL_HAT_DOWN)
			event.data1 = (ev->jhat.hat * 4) + KEY_HAT3;
		else if(value == SDL_HAT_LEFT)
			event.data1 = (ev->jhat.hat * 4) + KEY_HAT4;

		event.data2 = event.data1;
	}

	if(evc)
	{
		JoyEventList.push_back(evc);
		D_PostEvent(&event);
		return 1;
	}

	return 0;
}

void UpdateJoystickEvents()
{
	std::list<JoystickEvent_t*>::iterator i;
	event_t	event;

	if(JoyEventList.empty())
		return;

	i = JoyEventList.begin();
	while(i != JoyEventList.end())
	{
		if((*i)->Event.type == SDL_JOYHATMOTION)
		{
			// Hat position released
			if(!(SDL_JoystickGetHat(openedjoy, (*i)->Event.jhat.hat) & (*i)->Event.jhat.value))
				event.type = ev_keyup;
			// Hat button still held - Repeat at key repeat interval
			else if((SDL_GetTicks() - (*i)->RegTick >= SDL_DEFAULT_REPEAT_DELAY) &&
					(SDL_GetTicks() - (*i)->LastTick >= SDL_DEFAULT_REPEAT_INTERVAL*2))
			{
				(*i)->LastTick = SDL_GetTicks();
				event.type = ev_keydown;
			}
			else
			{
				++i;
				continue;
			}

			event.data1 = event.data2 = event.data3 = 0;

			if((*i)->Event.jhat.value == SDL_HAT_UP)
				event.data1 = ((*i)->Event.jhat.hat * 4) + KEY_HAT1;
			else if((*i)->Event.jhat.value == SDL_HAT_RIGHT)
				event.data1 = ((*i)->Event.jhat.hat * 4) + KEY_HAT2;
			else if((*i)->Event.jhat.value == SDL_HAT_DOWN)
				event.data1 = ((*i)->Event.jhat.hat * 4) + KEY_HAT3;
			else if((*i)->Event.jhat.value == SDL_HAT_LEFT)
				event.data1 = ((*i)->Event.jhat.hat * 4) + KEY_HAT4;

			D_PostEvent(&event);

			if(event.type == ev_keyup)
			{
				// Delete the released event
				delete *i;
				i = JoyEventList.erase(i);
				continue;
			}
		}
		++i;
	}
}

// This turns on automatic event polling for joysticks so that the state
// of each button and axis doesn't need to be manually queried each tick. -- Hyper_Eye
//
// EnableJoystickPolling
//
static int EnableJoystickPolling()
{
	return SDL_JoystickEventState(SDL_ENABLE);
}

static int DisableJoystickPolling()
{
	return SDL_JoystickEventState(SDL_IGNORE);
}

CVAR_FUNC_IMPL (use_joystick)
{
	if(var <= 0.0)
	{
		// Don't let console users disable joystick support because
		// they won't have any way to reenable through the menu.
#ifdef GCONSOLE
		use_joystick = 1.0;
#else
		I_CloseJoystick();
		DisableJoystickPolling();
#endif
	}
	else
	{
		I_OpenJoystick();
		EnableJoystickPolling();
	}
}


CVAR_FUNC_IMPL (joy_active)
{
	if( (var < 0.0) || ((int)var > I_GetJoystickCount()) )
		var = 0.0;

	I_CloseJoystick();

	I_OpenJoystick();
}

//
// I_GetJoystickCount
//
int I_GetJoystickCount()
{
	return SDL_NumJoysticks();
}

//
// I_GetJoystickNameFromIndex
//
std::string I_GetJoystickNameFromIndex (int index)
{
	const char  *joyname = NULL;
	std::string  ret;

	joyname = SDL_JoystickName(index);

	if(!joyname)
		return "";

	ret = joyname;

	return ret;
}

//
// I_OpenJoystick
//
bool I_OpenJoystick()
{
	int numjoy;

	numjoy = I_GetJoystickCount();

	if(!numjoy || !use_joystick)
		return false;

	if((int)joy_active > numjoy)
		joy_active.Set(0.0);

	if(!SDL_JoystickOpened(joy_active))
		openedjoy = SDL_JoystickOpen(joy_active);

	if(!SDL_JoystickOpened(joy_active))
		return false;

	return true;
}

//
// I_CloseJoystick
//
void I_CloseJoystick()
{
	extern int joyforward, joystrafe, joyturn, joylook;
	int		ndx;

#ifndef _XBOX // This is to avoid a bug in SDLx
	if(!I_GetJoystickCount() || !openedjoy)
		return;

	ndx = SDL_JoystickIndex(openedjoy);

	if(SDL_JoystickOpened(ndx))
		SDL_JoystickClose(openedjoy);

	openedjoy = NULL;
#endif

	// Reset joy position values. Wouldn't want to get stuck in a turn or something. -- Hyper_Eye
	joyforward = joystrafe = joyturn = joylook = 0;
}

//
// I_InitInput
//
bool I_InitInput (void)
{
	if (Args.CheckParm("-nomouse"))
		nomouse = true;

	if (sdl_keyboard_input_device == NULL)
		sdl_keyboard_input_device = new ISDL12KeyboardInputDevice();

	atterm(I_ShutdownInput);

	I_DisableKeyRepeat();

	// Initialize the joystick subsystem and open a joystick if use_joystick is enabled. -- Hyper_Eye
	Printf(PRINT_HIGH, "I_InitInput: Initializing SDL's joystick subsystem.\n");
	SDL_InitSubSystem(SDL_INIT_JOYSTICK);

	if((int)use_joystick && I_GetJoystickCount())
	{
		I_OpenJoystick();
		EnableJoystickPolling();
	}

#ifdef _WIN32
	// denis - in fullscreen, prevent exit on accidental windows key press
	// [Russell] - Disabled because it screws with the mouse
	//g_hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL,  LowLevelKeyboardProc, GetModuleHandle(NULL), 0);
#endif

	I_InitFocus();

	// [SL] do not intialize mouse driver here since it will be called from
	// the mouse_driver CVAR callback

	return true;
}

//
// I_ShutdownInput
//
void STACK_ARGS I_ShutdownInput (void)
{
	I_PauseMouse();

	I_ShutdownMouseDriver();

	I_UngrabInput();
	I_ResetKeyRepeat();

	delete sdl_keyboard_input_device;
	sdl_keyboard_input_device = NULL;
}

//
// I_PauseMouse
//
// Enables the mouse cursor and prevents the game from processing mouse movement
// or button events
//
void I_PauseMouse()
{
	if (mouse_input)
		mouse_input->pause();
}

//
// I_ResumeMouse
//
// Disables the mouse cursor and allows the game to process mouse movement
// or button events
//
void I_ResumeMouse()
{
	if (mouse_input)
		mouse_input->resume();
}

//
// I_GetEvent
//
// Pumps SDL for new input events and posts them to the Doom event queue.
// Keyboard and joystick events are retreived directly from SDL while mouse
// movement and buttons are handled by the MouseInput class.
//
void I_GetEvent()
{
	const int MAX_EVENTS = 256;
	static SDL_Event sdl_events[MAX_EVENTS];
	event_t event;

	I_UpdateFocus();
	I_UpdateGrab();

	// Process mouse movement and button events
	if (mouse_input)
		mouse_input->processEvents();


	sdl_keyboard_input_device->gatherEvents();
	while (sdl_keyboard_input_device->hasEvent())
	{
		event_t event;
		sdl_keyboard_input_device->getEvent(&event);
		D_PostEvent(&event);
	}


	const int event_mask = SDL_ALLEVENTS & ~SDL_KEYEVENTMASK & ~SDL_MOUSEEVENTMASK;

	// Force SDL to gather events from input devices. This is called
	// implicitly from SDL_PollEvent but since we're using SDL_PeepEvents to
	// process only non-mouse events, SDL_PumpEvents is necessary.
	SDL_PumpEvents();
	int num_events = SDL_PeepEvents(sdl_events, MAX_EVENTS, SDL_GETEVENT, event_mask);

	for (int i = 0; i < num_events; i++)
	{
		event.data1 = event.data2 = event.data3 = 0;

		SDL_Event* sdl_ev = &sdl_events[i];
		switch (sdl_ev->type)
		{
		case SDL_QUIT:
			AddCommandString("quit");
			break;

		case SDL_VIDEORESIZE:
		{
			// Resizable window mode resolutions
			if (!vid_fullscreen)
			{
				std::stringstream Command;
				Command << "vid_setmode " << sdl_ev->resize.w << " " << sdl_ev->resize.h;
				AddCommandString(Command.str());

				vid_defwidth.Set((float)sdl_ev->resize.w);
				vid_defheight.Set((float)sdl_ev->resize.h);
			}
			break;
		}

		case SDL_ACTIVEEVENT:
			// need to update our focus state
			I_UpdateFocus();
			I_UpdateGrab();
			// pause the mouse when the focus goes away (eg, alt-tab)
			if (!window_focused)
				I_PauseMouse();
			break;

		case SDL_JOYBUTTONDOWN:
			if (sdl_ev->jbutton.which == joy_active)
			{
				event.type = ev_keydown;
				event.data1 = sdl_ev->jbutton.button + KEY_JOY1;
				event.data2 = event.data1;

				D_PostEvent(&event);
				break;
			}

		case SDL_JOYBUTTONUP:
			if (sdl_ev->jbutton.which == joy_active)
			{
				event.type = ev_keyup;
				event.data1 = sdl_ev->jbutton.button + KEY_JOY1;
				event.data2 = event.data1;

				D_PostEvent(&event);
				break;
			}

		case SDL_JOYAXISMOTION:
			if (sdl_ev->jaxis.which == joy_active)
			{
				event.type = ev_joystick;
				event.data1 = 0;
				event.data2 = sdl_ev->jaxis.axis;
				if ((sdl_ev->jaxis.value < JOY_DEADZONE) && (sdl_ev->jaxis.value > -JOY_DEADZONE))
					event.data3 = 0;
				else
					event.data3 = sdl_ev->jaxis.value;

				D_PostEvent(&event);
				break;
			}

		case SDL_JOYHATMOTION:
			if (sdl_ev->jhat.which == joy_active)
			{
				// Each of these need to be tested because more than one can be pressed and a
				// unique event is needed for each
				if (sdl_ev->jhat.value & SDL_HAT_UP)
					RegisterJoystickEvent(sdl_ev, SDL_HAT_UP);
				if (sdl_ev->jhat.value & SDL_HAT_RIGHT)
					RegisterJoystickEvent(sdl_ev, SDL_HAT_RIGHT);
				if (sdl_ev->jhat.value & SDL_HAT_DOWN)
					RegisterJoystickEvent(sdl_ev, SDL_HAT_DOWN);
				if (sdl_ev->jhat.value & SDL_HAT_LEFT)
					RegisterJoystickEvent(sdl_ev, SDL_HAT_LEFT);

				break;
			}
		};
	}

	if (use_joystick)
		UpdateJoystickEvents();
}

//
// I_StartTic
//
void I_StartTic (void)
{
	I_GetEvent();
}

//
// I_StartFrame
//
void I_StartFrame (void)
{
}

// ============================================================================
//
// Mouse Drivers
//
// ============================================================================

static bool I_SDLMouseAvailible();
static bool I_MouseUnavailible();
#ifdef USE_RAW_WIN32_MOUSE
static bool I_RawWin32MouseAvailible();
#endif	// USE_RAW_WIN32_MOUSE

MouseDriverInfo_t MouseDriverInfo[] = {
	{ SDL_MOUSE_DRIVER,			"SDL Mouse",	&I_SDLMouseAvailible,		&SDLMouse::create },
#ifdef USE_RAW_WIN32_MOUSE
	{ RAW_WIN32_MOUSE_DRIVER,	"Raw Input",	&I_RawWin32MouseAvailible,	&RawWin32Mouse::create }
#else
	{ RAW_WIN32_MOUSE_DRIVER,	"Raw Input",	&I_MouseUnavailible,	NULL }
#endif	// USE_WIN32_MOUSE
};


//
// I_FindMouseDriverInfo
//
MouseDriverInfo_t* I_FindMouseDriverInfo(int id)
{
	for (int i = 0; i < NUM_MOUSE_DRIVERS; i++)
	{
		if (MouseDriverInfo[i].id == id)
			return &MouseDriverInfo[i];
	}

	return NULL;
}

//
// I_IsMouseDriverValid
//
// Returns whether a mouse driver with the given ID is availible to use.
//
static bool I_IsMouseDriverValid(int id)
{
	MouseDriverInfo_t* info = I_FindMouseDriverInfo(id);
	return (info && info->avail_test() == true);
}

CVAR_FUNC_IMPL(mouse_driver)
{
	if (!I_IsMouseDriverValid(var))
	{
		if (var.asInt() == SDL_MOUSE_DRIVER)
		{
			// can't initialize SDL_MOUSE_DRIVER so don't use a mouse
			I_ShutdownMouseDriver();
			nomouse = true;
		}
		else
		{
			var.Set(SDL_MOUSE_DRIVER);
		}
	}
	else
	{
		if (var.asInt() != mouse_driver_id)
		{
			mouse_driver_id = var.asInt();
			I_InitMouseDriver();
		}
	}
}

//
// I_ShutdownMouseDriver
//
// Frees the memory used by mouse_input
//
void I_ShutdownMouseDriver()
{
	delete mouse_input;
	mouse_input = NULL;
}

static void I_SetSDLIgnoreMouseEvents()
{
	SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);
	SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_IGNORE);
	SDL_EventState(SDL_MOUSEBUTTONUP, SDL_IGNORE);
}

static void I_UnsetSDLIgnoreMouseEvents()
{
	SDL_EventState(SDL_MOUSEMOTION, SDL_ENABLE);
	SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_ENABLE);
	SDL_EventState(SDL_MOUSEBUTTONUP, SDL_ENABLE);
}

//
// I_InitMouseDriver
//
// Instantiates the proper concrete MouseInput object based on the
// mouse_driver cvar and stores a pointer to the object in mouse_input.
//
void I_InitMouseDriver()
{
	I_ShutdownMouseDriver();

	// ignore SDL mouse input for now... The mouse driver will change this if needed
	I_SetSDLIgnoreMouseEvents();

	if (nomouse)
		return;

	// try to initialize the user's preferred mouse driver
	MouseDriverInfo_t* info = I_FindMouseDriverInfo(mouse_driver_id);
	if (info)
	{
		if (info->create != NULL)
			mouse_input = info->create();
		if (mouse_input != NULL)
			Printf(PRINT_HIGH, "I_InitMouseDriver: Initializing %s input.\n", info->name);
		else
			Printf(PRINT_HIGH, "I_InitMouseDriver: Unable to initalize %s input.\n", info->name);
	}

	// fall back on SDLMouse if the preferred driver failed to initialize
	if (mouse_input == NULL)
	{
		mouse_input = SDLMouse::create();
		if (mouse_input != NULL)
			Printf(PRINT_HIGH, "I_InitMouseDriver: Initializing SDL Mouse input as a fallback.\n");
		else
			Printf(PRINT_HIGH, "I_InitMouseDriver: Unable to initialize SDL Mouse input as a fallback.\n");
	}

	I_FlushInput();
	I_ResumeMouse();
}

//
// I_CheckForProc
//
// Checks if a function with the given name is in the given DLL file.
// This is used to determine if the user's version of Windows has the necessary
// functions availible.
//
#if defined(_WIN32) && !defined(_XBOX)
static bool I_CheckForProc(const char* dllname, const char* procname)
{
	bool avail = false;
	HMODULE dll = LoadLibrary(TEXT(dllname));
	if (dll)
	{
		avail = (GetProcAddress(dll, procname) != NULL);
		FreeLibrary(dll);
	}
	return avail;
}
#endif  // WIN32

//
// I_RawWin32MouseAvailible
//
// Checks if the raw input mouse functions that the RawWin32Mouse
// class calls are availible on the current system. They require
// Windows XP or higher.
//
#ifdef USE_RAW_WIN32_MOUSE
static bool I_RawWin32MouseAvailible()
{
	return	I_CheckForProc("user32.dll", "RegisterRawInputDevices") &&
			I_CheckForProc("user32.dll", "GetRegisteredRawInputDevices") &&
			I_CheckForProc("user32.dll", "GetRawInputData");
}
#endif  // USE_RAW_WIN32_MOUSE

//
// I_SDLMouseAvailible
//
// Checks if SDLMouse can be used. Always true since SDL is used as the
// primary backend for everything.
//
static bool I_SDLMouseAvailible()
{
	return true;
}

//
// I_MouseUnavailible
//
// Generic function to indicate that a particular mouse driver is not availible
// on this platform.
//
static bool I_MouseUnavailible()
{
	return false;
}

BEGIN_COMMAND(debugmouse)
{
	if (mouse_input)
		mouse_input->debug();
}
END_COMMAND(debugmouse)


// ============================================================================
//
// RawWin32Mouse
//
// ============================================================================

#ifdef USE_RAW_WIN32_MOUSE

#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC  ((USHORT) 0x01)
#endif

#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE  ((USHORT) 0x02)
#endif


//
// processRawMouseMovement
//
// Helper function to aggregate mouse movement events from a RAWMOUSE struct
// to a Doom event_t struct. Note that event->data2 and event->data3 need to
// be zeroed before calling.
//
bool processRawMouseMovement(const RAWMOUSE* mouse, event_t* event)
{
	static int prevx, prevy;
	static bool prev_valid = false;

	event->type = ev_mouse;
	event->data1 = 0;

	if (mouse->usFlags & MOUSE_MOVE_ABSOLUTE)
	{
		// we're given absolute mouse coordinates and need to convert
		// them to relative coordinates based on the previous x & y
		if (prev_valid)
		{
			event->data2 += mouse->lLastX - prevx;
			event->data3 -= mouse->lLastY - prevy;
		}

		prevx = mouse->lLastX;
		prevy = mouse->lLastY;
		prev_valid = true;
	}
	else
	{
		// we're given relative mouse coordinates
		event->data2 += mouse->lLastX;
		event->data3 -= mouse->lLastY;
		prev_valid = false;
	}

	return true;
}


//
// processRawMouseButtons
//
// Helper function to check if there is an event for the specified mouse
// button number in the RAWMOUSE struct. Returns true and fills the fields
// of event if there is a button event.
//
bool processRawMouseButtons(const RAWMOUSE* mouse, event_t* event, int button_num)
{
	static const UINT ri_down_lookup[5] = {
		RI_MOUSE_BUTTON_1_DOWN, RI_MOUSE_BUTTON_2_DOWN, RI_MOUSE_BUTTON_3_DOWN,
		RI_MOUSE_BUTTON_4_DOWN,	RI_MOUSE_BUTTON_5_DOWN };

	static const UINT ri_up_lookup[5] = {
		RI_MOUSE_BUTTON_1_UP, RI_MOUSE_BUTTON_2_UP, RI_MOUSE_BUTTON_3_UP,
		RI_MOUSE_BUTTON_4_UP, RI_MOUSE_BUTTON_5_UP };

	static const int oda_button_lookup[5] = {
		KEY_MOUSE1, KEY_MOUSE2, KEY_MOUSE3, KEY_MOUSE4, KEY_MOUSE5 };

	event->data1 = event->data2 = event->data3 = 0;

	if (mouse->usButtonFlags & ri_down_lookup[button_num])
	{
		event->type = ev_keydown;
		event->data1 = oda_button_lookup[button_num];
		return true;
	}
	else if (mouse->usButtonFlags & ri_up_lookup[button_num])
	{
		event->type = ev_keyup;
		event->data1 = oda_button_lookup[button_num];
		return true;
	}
	return false;
}


//
// processRawMouseScrollWheel
//
// Helper function to check for scroll wheel events in the RAWMOUSE struct.
// Returns true and fills in the fields in event if there is a scroll
// wheel event.
bool processRawMouseScrollWheel(const RAWMOUSE* mouse, event_t* event)
{
	event->type = ev_keydown;
	event->data1 = event->data2 = event->data3 = 0;

	if (mouse->usButtonFlags & RI_MOUSE_WHEEL)
	{
		if ((SHORT)mouse->usButtonData < 0)
		{
			event->data1 = KEY_MWHEELDOWN;
			return true;
		}
		else if ((SHORT)mouse->usButtonData > 0)
		{
			event->data1 = KEY_MWHEELUP;
			return true;
		}
	}

	return false;
}


//
// getMouseRawInputDevice
//
// Helper function that searches for a registered mouse raw input device. If
// found, the device parameter is filled with the information for the device
// and the function returns true.
//
bool getMouseRawInputDevice(RAWINPUTDEVICE& device)
{
	device.usUsagePage	= 0;
	device.usUsage		= 0;
	device.dwFlags		= 0;
	device.hwndTarget	= 0;

	// get the number of raw input devices
	UINT num_devices;
	GetRegisteredRawInputDevices(NULL, &num_devices, sizeof(RAWINPUTDEVICE));

	// create a buffer to hold the raw input device info
	RAWINPUTDEVICE* devices = new RAWINPUTDEVICE[num_devices];

	// look at existing registered raw input devices
	GetRegisteredRawInputDevices(devices, &num_devices, sizeof(RAWINPUTDEVICE));
	for (UINT i = 0; i < num_devices; i++)
	{
		// is there already a mouse device registered?
		if (devices[i].usUsagePage == HID_USAGE_PAGE_GENERIC &&
			devices[i].usUsage == HID_USAGE_GENERIC_MOUSE)
		{
			device.usUsagePage	= devices[i].usUsagePage;
			device.usUsage		= devices[i].usUsage;
			device.dwFlags		= devices[i].dwFlags;
			device.hwndTarget	= devices[i].hwndTarget;
			break;
		}
	}

    delete [] devices;

	return device.usUsagePage == HID_USAGE_PAGE_GENERIC &&
			device.usUsage == HID_USAGE_GENERIC_MOUSE;
}


// define the static member variables declared in the header
RawWin32Mouse* RawWin32Mouse::mInstance = NULL;

//
// RawWin32Mouse::RawWin32Mouse
//
RawWin32Mouse::RawWin32Mouse() :
	mActive(false), mInitialized(false),
	mHasBackupDevice(false), mRegisteredMouseDevice(false),
	mWindow(NULL), mBaseWindowProc(NULL), mRegisteredWindowProc(false)
{
	if (!I_RawWin32MouseAvailible())
		return;

	// get a handle to the window
	SDL_SysWMinfo wminfo;
	SDL_VERSION(&wminfo.version)
	SDL_GetWMInfo(&wminfo);
	mWindow = wminfo.window;

	mInstance = this;

	mInitialized = true;
	registerMouseDevice();
	registerWindowProc();
}


//
// RawWin32Mouse::~RawWin32Mouse
//
// Remove the callback for retreiving input and unregister the RAWINPUTDEVICE
//
RawWin32Mouse::~RawWin32Mouse()
{
	pause();
	mInstance = NULL;
}


//
// RawWin32Mouse::create
//
// Instantiates a new RawWin32Mouse and returns a pointer to it if successful
// or returns NULL if unable to instantiate it.
//
MouseInput* RawWin32Mouse::create()
{
	if (mInstance)
		return mInstance;

	RawWin32Mouse* obj = new RawWin32Mouse();

	if (obj && obj->mInitialized)
		return obj;

	// could not properly initialize
	delete obj;
	return NULL;
}


//
// RawWin32Mouse::flushEvents
//
// Clears the queued events
//
void RawWin32Mouse::flushEvents()
{
	clear();
}


//
// RawWin32Mouse::processEvents
//
// Iterates our queue of RAWINPUT events and converts them to Doom event_t
// and posts them for processing by the game internals.
//
void RawWin32Mouse::processEvents()
{
	if (!mActive)
		return;

	event_t movement_event;
	movement_event.type = ev_mouse;
	movement_event.data1 = movement_event.data2 = movement_event.data3 = 0;

	const RAWMOUSE* mouse;
	while (mouse = front())
	{
		popFront();

		// process mouse movement and save it
		processRawMouseMovement(mouse, &movement_event);

		// process mouse button clicks and post the events
		event_t button_event;
		for (int i = 0; i < 5; i++)
		{
			if (processRawMouseButtons(mouse, &button_event, i))
				D_PostEvent(&button_event);
		}

		// process mouse scroll wheel action
		if (processRawMouseScrollWheel(mouse, &button_event))
			D_PostEvent(&button_event);
	}

	// post any mouse movement events
	if (movement_event.data2 || movement_event.data3)
		D_PostEvent(&movement_event);
}


//
// RawWin32Mouse::center
//
void RawWin32Mouse::center()
{
	RECT rect;
	GetWindowRect(mWindow, &rect);
	SetCursorPos((rect.left + rect.right) / 2, (rect.top + rect.bottom) / 2);
}


//
// RawWin32Mouse::paused
//
bool RawWin32Mouse::paused() const
{
	return mActive == false;
}


//
// RawWin32Mouse::pause
//
void RawWin32Mouse::pause()
{
	mActive = false;

	unregisterMouseDevice();
	unregisterWindowProc();
}


//
// RawWin32Mouse::resume
//
void RawWin32Mouse::resume()
{
	mActive = true;
	flushEvents();

	registerMouseDevice();
	registerWindowProc();
}


//
// RawWin32Mouse::registerWindowProc
//
// Saves the existing WNDPROC for the app window and installs our own.
//
void RawWin32Mouse::registerWindowProc()
{
	if (!mRegisteredWindowProc)
	{
		// install our own window message callback and save the previous
		// callback as mBaseWindowProc
		mBaseWindowProc = (WNDPROC)SetWindowLongPtr(mWindow, GWLP_WNDPROC, (LONG_PTR)RawWin32Mouse::windowProcWrapper);
		mRegisteredWindowProc = true;
	}
}


//
// RawWin32Mouse::unregisterWindowProc
//
// Restore the saved WNDPROC for the app window.
//
void RawWin32Mouse::unregisterWindowProc()
{
	if (mRegisteredWindowProc)
	{
		SetWindowLongPtr(mWindow, GWLP_WNDPROC, (LONG_PTR)mBaseWindowProc);
		mBaseWindowProc = NULL;
		mRegisteredWindowProc = false;
	}
}


//
// RawWin32Mouse::windowProc
//
// A callback function that reads WM_Input messages and queues them for polling
// in processEvents
//
LRESULT CALLBACK RawWin32Mouse::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_INPUT)
	{
		RAWINPUT raw;
		UINT size = sizeof(raw);

		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &raw, &size, sizeof(RAWINPUTHEADER));

		if (raw.header.dwType == RIM_TYPEMOUSE)
		{
			pushBack(&raw.data.mouse);
			return 0;
		}
	}

	// hand the message off to mDefaultWindowProc since it's not a WM_INPUT mouse message
	return CallWindowProc(mBaseWindowProc, hwnd, message, wParam, lParam);
}


//
// RawWin32Mouse::windowProcWrapper
//
// A static member function that wraps calls to windowProc to allow member
// functions to use Windows callbacks.
//
LRESULT CALLBACK RawWin32Mouse::windowProcWrapper(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return mInstance->windowProc(hwnd, message, wParam, lParam);
}


//
// RawWin32Mouse::backupMouseDevice
//
void RawWin32Mouse::backupMouseDevice(const RAWINPUTDEVICE& device)
{
	mBackupDevice.usUsagePage	= device.usUsagePage;
	mBackupDevice.usUsage		= device.usUsage;
	mBackupDevice.dwFlags		= device.dwFlags;
	mBackupDevice.hwndTarget	= device.hwndTarget;
}


//
// RawWin32Mouse::restoreMouseDevice
//
void RawWin32Mouse::restoreMouseDevice(RAWINPUTDEVICE& device) const
{
	device.usUsagePage	= mBackupDevice.usUsagePage;
	device.usUsage		= mBackupDevice.usUsage;
	device.dwFlags		= mBackupDevice.dwFlags;
	device.hwndTarget	= mBackupDevice.hwndTarget;
}


//
// RawWin32Mouse::registerRawInputDevice
//
// Registers the mouse as a raw input device, backing up the previous raw input
// device for later restoration.
//
bool RawWin32Mouse::registerMouseDevice()
{
	if (mRegisteredMouseDevice)
		return false;

	RAWINPUTDEVICE device;

	if (getMouseRawInputDevice(device))
	{
		// save a backup copy of this device
		if (!mHasBackupDevice)
		{
			backupMouseDevice(device);
			mHasBackupDevice = true;
		}

		// remove the existing device
		device.dwFlags = RIDEV_REMOVE;
		device.hwndTarget = NULL;
		RegisterRawInputDevices(&device, 1, sizeof(device));
	}

	// register our raw input mouse device
	device.usUsagePage	= HID_USAGE_PAGE_GENERIC;
	device.usUsage		= HID_USAGE_GENERIC_MOUSE;
	device.dwFlags		= RIDEV_NOLEGACY;
	device.hwndTarget	= mWindow;

	mRegisteredMouseDevice = RegisterRawInputDevices(&device, 1, sizeof(device));
	return mRegisteredMouseDevice;
}


//
// RawWin32Mouse::unregisterRawInputDevice
//
// Removes the mouse as a raw input device, restoring a previously backedup
// mouse device if applicable.
//
bool RawWin32Mouse::unregisterMouseDevice()
{
	if (!mRegisteredMouseDevice)
		return false;

	RAWINPUTDEVICE device;

	if (getMouseRawInputDevice(device))
	{
		// remove the device
		device.dwFlags = RIDEV_REMOVE;
		device.hwndTarget = NULL;
		RegisterRawInputDevices(&device, 1, sizeof(device));
		mRegisteredMouseDevice = false;
	}

	if (mHasBackupDevice)
	{
		restoreMouseDevice(device);
		mHasBackupDevice = false;
		RegisterRawInputDevices(&device, 1, sizeof(device));
	}

	return mRegisteredMouseDevice == false;
}


//
// RawWin32Mouse::debug
//
void RawWin32Mouse::debug() const
{
	// get a handle to the window
	SDL_SysWMinfo wminfo;
	SDL_VERSION(&wminfo.version)
	SDL_GetWMInfo(&wminfo);
	HWND cur_window = wminfo.window;

	// determine the hwndTarget parameter of the registered rawinput device
	HWND hwndTarget = NULL;

	RAWINPUTDEVICE device;
	if (getMouseRawInputDevice(device))
	{
		hwndTarget = device.hwndTarget;
	}

	Printf(PRINT_HIGH, "RawWin32Mouse: Current Window Address: 0x%x, mWindow: 0x%x, RAWINPUTDEVICE Window: 0x%x\n",
			cur_window, mWindow, hwndTarget);

	WNDPROC wndproc = (WNDPROC)GetWindowLongPtr(cur_window, GWLP_WNDPROC);
	Printf(PRINT_HIGH, "RawWin32Mouse: windowProcWrapper Address: 0x%x, Current Window WNDPROC Address: 0x%x\n",
			RawWin32Mouse::windowProcWrapper, wndproc);
}

#endif	// USE_RAW_WIN32_MOUSE


// ============================================================================
//
// SDLMouse
//
// ============================================================================

static ISDL12MouseInputDevice* sdl_mouse_input_device = NULL;

//
// SDLMouse::SDLMouse
//
SDLMouse::SDLMouse() :
	mActive(false)
{
	if (sdl_mouse_input_device == NULL)
		sdl_mouse_input_device = new ISDL12MouseInputDevice();
	sdl_mouse_input_device->resume();
}

//
// SDLMouse::~SDLMouse
//
SDLMouse::~SDLMouse()
{
	sdl_mouse_input_device->pause();
	delete sdl_mouse_input_device;
	sdl_mouse_input_device = NULL;
}

//
// SDLMouse::create
//
// Instantiates a new SDLMouse and returns a pointer to it if successful
// or returns NULL if unable to instantiate it. However, since SDL is
// always availible, this never returns NULL
//
//
MouseInput* SDLMouse::create()
{
	return new SDLMouse();
}

void SDLMouse::flushEvents()
{
	sdl_mouse_input_device->flushEvents();
}

//
// SDLMouse::processEvents
//
// Pumps SDL's event queue and processes only its mouse events. The events
// are converted to Doom event_t and posted for processing by the game
// internals.
//
void SDLMouse::processEvents()
{
	sdl_mouse_input_device->gatherEvents();

	event_t event;
	while (sdl_mouse_input_device->hasEvent())
	{
		sdl_mouse_input_device->getEvent(&event);

		if (event.data1 || event.data2 || event.data3)
			D_PostEvent(&event);
	}

	sdl_mouse_input_device->reset();
}

//
// SDLMouse::center
//
// Moves the mouse to the center of the screen to prevent absolute position
// methods from causing problems when the mouse is near the screen edges.
//
void SDLMouse::center()
{
	sdl_mouse_input_device->reset();
}


bool SDLMouse::paused() const
{
	return sdl_mouse_input_device->paused();
}


void SDLMouse::pause()
{
	sdl_mouse_input_device->pause();
}


void SDLMouse::resume()
{
	sdl_mouse_input_device->resume();
}


//
// SDLMouse::debug
//
void SDLMouse::debug() const
{
#if defined(_WIN32) && !defined(_XBOX)
	// get a handle to the window
	SDL_SysWMinfo wminfo;
	SDL_VERSION(&wminfo.version)
	SDL_GetWMInfo(&wminfo);
	HWND cur_window = wminfo.window;

	// determine the hwndTarget parameter of the registered rawinput device
	HWND hwndTarget = NULL;

	RAWINPUTDEVICE device;
	if (getMouseRawInputDevice(device))
	{
		hwndTarget = device.hwndTarget;
	}

	Printf(PRINT_HIGH, "SDLMouse: Current Window Address: 0x%x, RAWINPUTDEVICE Window: 0x%x\n",
			cur_window, hwndTarget);

	WNDPROC wndproc = (WNDPROC)GetWindowLongPtr(cur_window, GWLP_WNDPROC);
	Printf(PRINT_HIGH, "SDLMouse: Current Window WNDPROC Address: 0x%x\n",
			wndproc);
#endif
}

VERSION_CONTROL (i_input_cpp, "$Id$")


