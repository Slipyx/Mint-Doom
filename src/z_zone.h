// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// Copyright (C) 1999 by
// id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
// Copyright (C) 1999-2000 by
// Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
// Copyright 2005, 2006 by
// Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
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
//     Zone Memory Allocation, perhaps NeXT ObjectiveC inspired.
//     Remark: this was the only stuff that, according
//      to John Carmack, might have been useful for
//      Quake.
//
// JoshK: Ported from PrBoom, sans INSTRUMENTED. Rewritten by Lee Killough.
//
//---------------------------------------------------------------------

#ifndef __Z_ZONE__
#define __Z_ZONE__

// Include system definitions so that prototypes become
// active before macro replacements below are in effect.

#include "doomtype.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

//
// ZONE MEMORY
// PU - purge tags.
//
enum
{
    PU_FREE,       // a free block
    PU_STATIC,     // static entire execution time
    PU_SOUND,      // static while playing
    PU_MUSIC,      // static while playing
    PU_LEVEL,      // static until level exited
    PU_LEVSPEC,    // a special thinker in a level

    PU_CACHE,
    PU_MAX,    // Must always be last -- killough
};

#define PU_PURGELEVEL    PU_CACHE    // First purgable tag's level

void *Z_Malloc(size_t size, int32_t tag, void **ptr);
void Z_Free(void *ptr);
void Z_FreeTags(int32_t lowtag, int32_t hightag);
void Z_ChangeTag(void *ptr, int32_t tag);
void Z_Init(void);
void Z_Close(void);
void *Z_Calloc(size_t n1, size_t n2, int32_t tag, void **user);
void *Z_Realloc(void *ptr, size_t n, int32_t tag, void **user);
char *Z_Strdup(const char *s, int32_t tag, void **user);
void Z_CheckHeap(void);

// Remove all definitions before including system definitions
/*
#undef malloc
#undef free
#undef realloc
#undef calloc
#undef strdup

#define malloc(n)         Z_Malloc(n, PU_STATIC, NULL)
#define free(p)           Z_Free(p)
#define realloc(p, n)     Z_Realloc(p, n, PU_STATIC, NULL)
#define calloc(n1, n2)    Z_Calloc(n1, n2, PU_STATIC, NULL)
#define strdup(s)         Z_Strdup(s, PU_STATIC, NULL)
*/

#endif
