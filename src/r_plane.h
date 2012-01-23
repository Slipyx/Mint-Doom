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
//    Refresh, visplane stuff (floor, ceilings).
//
//-----------------------------------------------------------------------------

#ifndef __R_PLANE__
#define __R_PLANE__

#include "r_data.h"

// Visplane related.
extern int16_t    *lastopening;

typedef void (*planefunction_t)(int32_t top, int32_t bottom);

extern planefunction_t    floorfunc;
extern planefunction_t    ceilingfunc_t;

extern int16_t    floorclip[SCREENWIDTH];
extern int16_t    ceilingclip[SCREENWIDTH];

extern fixed_t    yslope[SCREENHEIGHT];
extern fixed_t    distscale[SCREENWIDTH];

void R_InitPlanes(void);
void R_ClearPlanes(void);

void R_MapPlane(int32_t y, int32_t x1, int32_t x2);

void R_MakeSpans(int32_t x, int32_t t1, int32_t b1, int32_t t2, int32_t b2);

void R_DrawPlanes(void);

visplane_t *R_FindPlane(fixed_t height, int32_t picnum, int32_t lightlevel);

visplane_t *R_CheckPlane(visplane_t *pl, int32_t start, int32_t stop);

#endif
