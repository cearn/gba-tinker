//
// Testing iwram versions of chr4c_drawg_b1cts. Let's see what the compiler can make of it.
// This is the base version (Thumb/ROM), included in the other files. Be sure to define
// - GEN_CODE_SEC			// Code section to put code in
// - GEN_PROCNAME_BASE		// name of 'base' function
// - GEN_PROCNAME_FAST		// name of 'fast' function


#include <tonc_memdef.h>
#include <tonc_tte.h>

#ifndef GEN_CODE_SEC
	#define GEN_CODE_SEC 
#endif

#ifndef GEN_PROCNAME_BASE
	#define GEN_PROCNAME_BASE chr4c_drawg_b1cts_c_1p_thumb_rom
	#endif
#ifndef GEN_PROCNAME_FAST
	#define GEN_PROCNAME_FAST chr4c_drawg_b1cts_c_8p_thumb_rom
#endif

// --------------------------------------------------------------------
// FUNCTIONS
// --------------------------------------------------------------------

/// Base version, pixel-by-pixel.
GEN_CODE_SEC void GEN_PROCNAME_BASE (uint gid)
{
	TTE_BASE_VARS(tc, font);
	TTE_CHAR_VARS(font, gid, u8, srcD, srcL, charW, charH);
	uint x0 = tc->cursorX, y0 = tc->cursorY;
	uint srcP = font->cellH;

	u32 ink = tc->cattr[TTE_INK], raw;

	uint ix, iy, iw;
	for(iw=0; iw<charW; iw += 8)
	{	
		for(iy=0; iy<charH; iy++)
		{
			raw = srcL[iy];
			for(ix=0; raw>0; raw>>=1, ix++)
				if(raw&1)
					_schr4c_plot(&tc->dst, x0+ix, y0+iy, ink);
		}
		srcL += srcP;
		x0 += 8;
	}
}

#define FEATURE_MASK_TEST() 1

/// Render 1bpp fonts to 4bpp tiles, fast(?) version: 8px at a time.
GEN_CODE_SEC void GEN_PROCNAME_FAST (uint gid)
{
	TTE_BASE_VARS(tc, font);
	TTE_CHAR_VARS(font, gid, u8, srcD, srcL, charW, charH);
	uint x = tc->cursorX, y = tc->cursorY;
	uint srcP = font->cellH, dstP = tc->dst.pitch/4;

	u32 *dstD = (u32*)((u8*)tc->dst.data + y*4 + x/8*dstP*4), *dstL;
	x %= 8;
	const u32 lsl = 4*x, lsr = 32-4*x;
	const u32 shiftMask = (1<<lsr)-1;

	// Inner loop vars
	u32 px, pxmask, raw;
	u32 ink = tc->cattr[TTE_INK];
#if FEATURE_MASK_TEST() == 1
	const u32 mask = 0x01010101;
#else
	u32 right = x+charW;
#endif

	uint iy;
	for(uint iw=0; iw<charW; iw += 8)	// Loop over strips
	{
		dstL = dstD;		dstD += dstP;
		srcL = srcD;		srcD += srcP;

		iy = charH;
		while(iy--)					// Loop over scanlines
		{
			raw = *srcL++;
			if(raw)
			{
				raw |= raw<<12;
				raw |= raw<< 6;
				px   = raw & mask<<1;
				raw &= mask;
				px   = raw | px<<3;

				pxmask = px*15;
				px   *= ink;

				// Write left tile:
				dstL[0] = (dstL[0] &~ (pxmask<<lsl) ) | (px<<lsl);

				// Write right tile (if any)
#if FEATURE_MASK_TEST() == 1
				if (px &~ shiftMask)
					dstL[dstP] = (dstL[dstP] &~ (pxmask>>lsr) ) | (px>>lsr);
#else
				if(right > 8)
					dstL[dstP] = (dstL[dstP] &~ (pxmask>>lsr) ) | (px>>lsr);
#endif
			}
			dstL++;
		}
	}
}
