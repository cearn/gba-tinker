//
// 1p and 8p implementation of chr4c_drawg_b2ts. 
// Include this in the to-compile files to 'activate' the implementaitons. 
// Be sure to define
// - GEN_CODE_SEC			// Code section to put code in
// - GEN_PROCNAME_1P		// name of 'base' function
// - GEN_PROCNAME_8P		// name of 'fast' function
// Yes, normally you'd make these inline functions and then call them in the specific functions. 
// However, I want something for -O0 as well, and that does not inline functions.
// 
// Basically: don't emulate this :P

#include <tonc_memdef.h>
#include <tonc_tte.h>

#ifndef GEN_CODE_SEC
	#error "Please define `GEN_CODE_SEC` (empty or `IWRAM_CODE`)"
	#define GEN_CODE_SEC 
#endif

#ifndef GEN_PROCNAME_1P
	#error "Please define `GEN_PROCNAME_1P` for the 1p implementation"
#endif

#ifndef GEN_PROCNAME_8P
	#error "Please define `GEN_PROCNAME_8P` for the 8p implementation"
#endif

// Use a shiftmask instead of a right-test [1]
#define FEATURE_8P_MASK_TEST() 1

// Use an inline plotter for 1p [1]
#define FEATURE_1P_INLINE() 1

// --------------------------------------------------------------------
// FUNCTIONS
// --------------------------------------------------------------------

/// Base version, pixel-by-pixel.
GEN_CODE_SEC void GEN_PROCNAME_1P (uint gid)
{
	TTE_BASE_VARS(tc, font);
	TTE_CHAR_VARS(font, gid, u16, srcD, srcL, charW, charH);
	uint x0 = tc->cursorX, y0 = tc->cursorY;
	uint srcP = font->cellH;

	uint ix, iy, iw;
	for(iw=0; iw<charW; iw += 8)
	{
		for(iy=0; iy<charH; iy++)
		{
			u32 raw = srcL[iy];
			for(ix=0; raw>0; raw>>=2, ix++)
			{
				u32 px = raw&3;
				if(px)
#if FEATURE_1P_INLINE() == 1
					_schr4c_plot(&tc->dst, x0+ix, y0+iy, px);
#else
					schr4c_plot(&tc->dst, x0+ix, y0+iy, px);
#endif
			}
		}
		srcL += srcP;
		x0 += 8;
	}
}

/// Render 2bpp fonts to 4bpp tiles, fast(?) version: 8px at a time.
GEN_CODE_SEC void GEN_PROCNAME_8P (uint gid)
{
	TTE_BASE_VARS(tc, font);
	TTE_CHAR_VARS(font, gid, u16, srcD, srcL, charW, charH);
	uint x = tc->cursorX, y = tc->cursorY;
	uint srcP = font->cellH, dstP = tc->dst.pitch/4;

	u32 *dstD = (u32*)((u8*)tc->dst.data + y*4 + x/8*dstP*4), *dstL;
	x %= 8;
	const uint lsl = 4*x, lsr = 32-4*x;
	
	// Inner loop vars
	u32 px, pxmask, raw;
	const u32 UNPACKMASK = 0x30303030;
	const u32 PXMASK = 0x11111111;
#if FEATURE_8P_MASK_TEST() == 1
	const uint shiftMask = (1<<lsr)-1;
#else
	uint right = x+charW;
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
				raw |= raw<<8;
				raw &= ~0xFF00;
				raw |= raw<<4;
				px   = UNPACKMASK & (raw<<2);
				raw &= UNPACKMASK>>4;
				px |= raw;

				// generate pixel mask
				pxmask  =   px     & PXMASK;
				pxmask |= ((px>>1) & PXMASK);
				pxmask *= 15;

				// Write left tile:
				dstL[0] = (dstL[0] &~ (pxmask<<lsl) ) | (px<<lsl);

				// Write right tile (if any)
#if FEATURE_8P_MASK_TEST() == 1
				if (px &~ shiftMask)
					dstL[dstP] = (dstL[dstP] &~ (pxmask>>lsr) ) | (px>>lsr);
#else
				if(right > 8)
					dstL[dstP] = (dstL[dstP] &~ (pxmask>>lsr) ) | (px>>lsr);
#endif
			}
#if FEATURE_8P_MASK_TEST() == 0
			right -= 8;
#endif
			dstL++;
		}
	}
}
