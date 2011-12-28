// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005 Simon Howard
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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
// DESCRIPTION:
//    System specific interface stuff.
//
//-----------------------------------------------------------------------------

#ifndef __I_VIDEO__
#define __I_VIDEO__

#include "doomtype.h"

#define MAX_MOUSE_BUTTONS    8

typedef struct
{
        // Screen width and height

        int32_t    width;
        int32_t    height;

        // Initialisation function to call when using this mode.
        // Called with a pointer to the Doom palette.
        //
        // If NULL, no init function is called.

        void (*InitMode)(byte *palette);

        // Function to call to draw the screen from the source buffer.
        // Return true if draw was successful.

        boolean (*DrawScreen)(int32_t x1, int32_t y1, int32_t x2, int32_t y2);

        // If true, this is a "poor quality" mode.  The autoadjust
        // code should always attempt to use a different mode to this
        // mode in fullscreen.

        boolean    poor_quality;
} screen_mode_t;

// Called by D_DoomMain,
// determines the hardware configuration
// and sets up the video mode
void I_InitGraphics(void);

void I_ShutdownGraphics(void);

// Takes full 8 bit values.
void I_SetPalette(byte *palette);

void I_UpdateNoBlit(void);
void I_FinishUpdate(void);

// Wait for vertical retrace or pause a bit.
void I_WaitVBL(int32_t count);

void I_ReadScreen(byte *scr);

void I_BeginRead(void);
void I_EndRead(void);

void I_SetWindowCaption(void);
void I_SetWindowIcon(void);

void I_CheckIsScreensaver(void);

extern char       *video_driver;
extern int32_t    autoadjust_video_settings;
extern boolean    screenvisible;
extern int32_t    screen_width, screen_height;
extern int32_t    screen_bpp;
extern int32_t    fullscreen;
extern int32_t    aspect_ratio_correct;
extern int32_t    grabmouse;
extern float      mouse_acceleration;
extern int32_t    mouse_threshold;
extern int32_t    startup_delay;
extern int32_t    vanilla_keyboard_mapping;

#endif
