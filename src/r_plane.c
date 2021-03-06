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
//    Here is a core component: drawing the floors and ceilings,
//     while maintaining a per column clipping list only.
//    Moreover, the sky areas have to be determined.
//
//-----------------------------------------------------------------------------

#include <stdlib.h>

#include "i_system.h"
#include "z_zone.h"
#include "w_wad.h"

#include "doomdef.h"
#include "doomstat.h"

#include "r_local.h"
#include "r_sky.h"

planefunction_t    floorfunc;
planefunction_t    ceilingfunc;

//
// opening
//

// Here comes the obnoxious "visplane".
#define MAXVISPLANES    128    // Must be a power of two
// JoshK: Visplanes hash table. From PRBoom. By Lee Killough.
static visplane_t    *visplanes[MAXVISPLANES];
static visplane_t    *freetail;
static visplane_t    **freehead = &freetail;

visplane_t    *floorplane;
visplane_t    *ceilingplane;

// JoshK: Hash function for visplanes. From PRBoom. By Lee Killough.
#define visplane_hash(picnum, lightlevel, height) \
  ((uint32_t)((picnum) * 3 + (lightlevel) + (height) * 7) & (MAXVISPLANES - 1))

// ?
#define MAXOPENINGS    SCREENWIDTH * 64
int16_t    openings[MAXOPENINGS];
int16_t    *lastopening;

//
// Clip values are the solid pixel bounding the range.
//  floorclip starts out SCREENHEIGHT
//  ceilingclip starts out -1
//
int16_t    floorclip[SCREENWIDTH];
int16_t    ceilingclip[SCREENWIDTH];

//
// spanstart holds the start of a plane span
// initialized to 0 at start
//
int32_t    spanstart[SCREENHEIGHT];
//int32_t    spanstop[SCREENHEIGHT];

//
// texture mapping
//
lighttable_t    **planezlight;
fixed_t    planeheight;

fixed_t    yslope[SCREENHEIGHT];
fixed_t    distscale[SCREENWIDTH];
fixed_t    basexscale;
fixed_t    baseyscale;

fixed_t    cachedheight[SCREENHEIGHT];
fixed_t    cacheddistance[SCREENHEIGHT];
fixed_t    cachedxstep[SCREENHEIGHT];
fixed_t    cachedystep[SCREENHEIGHT];

//
// R_InitPlanes
// Only at game startup.
//
void R_InitPlanes(void)
{
    // Doh!
}

//
// R_MapPlane
//
// Uses global vars:
//  planeheight
//  ds_source
//  basexscale
//  baseyscale
//  viewx
//  viewy
//
// BASIC PRIMITIVE
//
static void R_MapPlane(int32_t y, int32_t x1, int32_t x2)
{
    angle_t     angle;
    fixed_t     distance;
    fixed_t     length;
    uint32_t    index;

#ifdef RANGECHECK
    if (x2 < x1 || x1 < 0
     || x2 >= viewwidth || y > viewheight)
    {
        I_Error("R_MapPlane: %i, %i at %i", x1, x2, y);
    }
#endif

    if (planeheight != cachedheight[y])
    {
        cachedheight[y] = planeheight;
        distance = cacheddistance[y] = FixedMul(planeheight, yslope[y]);
        ds_xstep = cachedxstep[y] = FixedMul(distance, basexscale);
        ds_ystep = cachedystep[y] = FixedMul(distance, baseyscale);
    }
    else
    {
        distance = cacheddistance[y];
        ds_xstep = cachedxstep[y];
        ds_ystep = cachedystep[y];
    }

    length = FixedMul(distance, distscale[x1]);
    angle = (viewangle + xtoviewangle[x1]) >> ANGLETOFINESHIFT;
    ds_xfrac = viewx + FixedMul(finecosine[angle], length);
    ds_yfrac = -viewy - FixedMul(finesine[angle], length);

    if (!fixedcolormap)
    {
        index = distance >> LIGHTZSHIFT;

        if (index >= MAXLIGHTZ)
        {
            index = MAXLIGHTZ - 1;
        }

        ds_colormap = planezlight[index];
    }
    else
    {
        ds_colormap = fixedcolormap;
    }

    ds_y = y;
    ds_x1 = x1;
    ds_x2 = x2;

    // high or low detail
    spanfunc();
}

//
// R_ClearPlanes
// At begining of frame.
//
void R_ClearPlanes(void)
{
    int32_t    i;
    //angle_t    angle;

    // opening / clipping determination
    for (i=0; i<viewwidth; i++)
    {
        floorclip[i] = viewheight;
        ceilingclip[i] = -1;
    }

    // JoshK: Clear visplanes hash table. From PRBoom. By Lee Killough
    for (i=0; i<MAXVISPLANES; ++i)
    {
        for (*freehead=visplanes[i], visplanes[i]=NULL; *freehead; )
        {
            freehead = &(*freehead)->next;
        }
    }

    lastopening = openings;

    // texture calculation
    memset(cachedheight, 0, sizeof(cachedheight));

    // left to right mapping
    //angle = (viewangle - ANG90) >> ANGLETOFINESHIFT;

    // JoshK: from prboom :D
    // scale will be unit scale at SCREENWIDTH/2 distance
    basexscale = FixedDiv(viewsin, projection);
    baseyscale = FixedDiv(viewcos, projection);
}

// JoshK: New function for making a new visplane. From PRBoom. By Lee Killough
static visplane_t *new_visplane(uint32_t hash)
{
    visplane_t   *check = freetail;

    if (check == NULL)
    {
        check = (visplane_t *)Z_Calloc(1, sizeof(*check), PU_STATIC, NULL);
    }
    else if ((freetail = freetail->next) == NULL)
    {
        freehead = &freetail;
    }
    check->next = visplanes[hash];
    visplanes[hash] = check;

    return check;
}

//
// R_FindPlane
//
visplane_t *R_FindPlane(fixed_t height, int32_t picnum, int32_t lightlevel)
{
    visplane_t    *check;
    uint32_t      hash;

    if (picnum == skyflatnum)
    {
        height = lightlevel = 0; // all skys map together
    }

    // JoshK: New visplane search algorithm. From PRBoom. By Lee Killough
    hash = visplane_hash(picnum, lightlevel, height);
    for (check=visplanes[hash]; check; check=check->next)
    {
        if (height == check->height
         && picnum == check->picnum
         && lightlevel == check->lightlevel)
        {
            return check;
        }
    }

    // JoshK: new visplane
    check = new_visplane(hash);

    check->height = height;
    check->picnum = picnum;
    check->lightlevel = lightlevel;
    check->minx = viewwidth; // JoshK: PRBoom uses viewwidth, not SCREENWIDTH
    check->maxx = -1;

    memset(check->top, 0xff, sizeof(check->top));

    return check;
}

//
// R_CheckPlane
//
visplane_t *R_CheckPlane(visplane_t *pl, int32_t start, int32_t stop)
{
    int32_t    intrl, intrh, unionl, unionh, x;

    if (start < pl->minx)
    {
        intrl = pl->minx;
        unionl = start;
    }
    else
    {
        unionl = pl->minx;
        intrl = start;
    }

    if (stop > pl->maxx)
    {
        intrh = pl->maxx;
        unionh = stop;
    }
    else
    {
        unionh = pl->maxx;
        intrh = stop;
    }

    // JoshK: Dropoff overflow from PRBoom
    for (x=intrl; x<=intrh && pl->top[x]==0xffu; x++)
    {
        ;
    }

    if (x > intrh)
    {
        // use the same one
        pl->minx = unionl; pl->maxx = unionh;
        return pl;
    }
    else
    {
        // JoshK:
        // Make a new visplane. From ZDoom.
        uint32_t      hash;
        visplane_t    *new_pl = NULL;

        hash = visplane_hash(pl->picnum, pl->lightlevel, pl->height);
        new_pl = new_visplane(hash);
        new_pl->height = pl->height;
        new_pl->picnum = pl->picnum;
        new_pl->lightlevel = pl->lightlevel;

        pl = new_pl;
        pl->minx = start;
        pl->maxx = stop;

        memset(pl->top, 0xff, sizeof(pl->top));
        return pl;
    }
}

//
// R_MakeSpans
//
// JoshK: Ported from PRBoom
//
static void R_MakeSpans(int32_t x, int32_t t1, int32_t b1, int32_t t2, int32_t b2)
{
    for (; t1<t2 && t1<=b1; ++t1)
    {
        R_MapPlane(t1, spanstart[t1], x - 1);
    }
    for (; b1>b2 && b1>=t1; --b1)
    {
        R_MapPlane(b1, spanstart[b1], x - 1);
    }
    while (t2 < t1 && t2 <= b2)
    {
        spanstart[t2++] = x;
    }
    while (b2 > b1 && b2 >= t2)
    {
        spanstart[b2--] = x;
    }
}

//
// R_DrawPlanes
// At the end of each frame.
//
void R_DrawPlanes(void)
{
    visplane_t    *pl;
    int32_t       light;
    int32_t       x;
    int32_t       stop;
    int32_t       angle;
    int32_t       lumpnum;
    uint32_t      i;

#ifdef RANGECHECK
    if (ds_p - drawsegs > MAXDRAWSEGS)
    {
        I_Error("R_DrawPlanes: drawsegs overflow (%i)", ds_p - drawsegs);
    }

    if (lastopening - openings > MAXOPENINGS)
    {
        I_Error("R_DrawPlanes: opening overflow (%i)",
                lastopening - openings);
    }
#endif

    // JoshK: New visplanes draw loop.
    for (i=0; i<MAXVISPLANES; ++i)
    {
    for (pl=visplanes[i]; pl; pl=pl->next)
    {
        if (pl->minx > pl->maxx)
        {
            continue;
        }

        // sky flat
        if (pl->picnum == skyflatnum)
        {
            dc_iscale = pspriteiscale >> detailshift;

            // Sky is allways drawn full bright,
            //  i.e. colormaps[0] is used.
            // Because of this hack, sky is not affected
            //  by INVUL inverse mapping.
            dc_colormap = colormaps;
            dc_texturemid = skytexturemid;
            for (x=pl->minx; x<=pl->maxx; x++)
            {
                dc_yl = pl->top[x];
                dc_yh = pl->bottom[x];

                if (dc_yl <= dc_yh)
                {
                    angle = (viewangle + xtoviewangle[x]) >> ANGLETOSKYSHIFT;
                    dc_x = x;
                    dc_source = R_GetColumn(skytexture, angle);
                    dc_texheight = textureheight[skytexture] >> FRACBITS;
                    colfunc();
                }
            }
            continue;
        }

        // regular flat
        lumpnum = firstflat + flattranslation[pl->picnum];
        ds_source = W_CacheLumpNum(lumpnum, PU_STATIC);

        planeheight = abs(pl->height - viewz);
        light = (pl->lightlevel >> LIGHTSEGSHIFT) + extralight;

        if (light >= LIGHTLEVELS)
        {
            light = LIGHTLEVELS - 1;
        }

        if (light < 0)
        {
            light = 0;
        }

        // JoshK: Dropoff overflow from PRBoom
        stop = pl->maxx + 1;
        planezlight = zlight[light];
        pl->top[pl->minx - 1] = pl->top[stop] = 0xffu;

        for (x=pl->minx; x<=stop; x++)
        {
            R_MakeSpans(x, pl->top[x - 1], pl->bottom[x - 1],
                        pl->top[x], pl->bottom[x]);
        }

        W_ReleaseLumpNum(lumpnum);
    }
    }
}
