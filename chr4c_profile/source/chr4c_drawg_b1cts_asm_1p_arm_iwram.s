///
/// Col-major tile character renderer. 1->4bpp recolored, any size, transparent
/// This is the optimized 'naive' version: still pixel-by-pixel.
/// I just want to see how fast I can get that.
///
/// @file chr4c_drawg_b1cts_asm_1p_arm_iwram.s
/// @author J Vijn
/// @date 20250719
///
/// === NOTES ===

#define FEATURE_MASK_TEST() 1

#include <tonc_asminc.h>
#include <../src/tte/tte_types.s>

// IWRAM_CODE void chr4c_drawg_b1cts_asm_1p_arm_iwram(int gid);
BEGIN_FUNC_ARM(chr4c_drawg_b1cts_asm_1p_arm_iwram, CSEC_IWRAM)
	push	{r4-r11}
	ldr		r3,=gp_tte_context
	ldr		r3, [r3]
	ldmia	r3, {r6, r11}			@ r6 = dstBase, r11 = dstP
	ldrb	r7, [r3, #TTC_ink]		@ r7 = ink
	add		r3, r3, #TTC_cursorX
	ldmia	r3, {r4, ip}			@ YX, font. r3 free

	ldmia	ip, {r1, r3}			@ glyphs, widths
	cmp		r3, #0
	ldrneb	r10, [r3, r0]			@ - r10 = charW
	ldreqb	r10, [ip, #TF_charW]	@ /
	ldrh	r3, [ip, #TF_cellS]
	mla		r1, r3, r0, r1			@ r1 = &glyphs[gid]
	ldrb	r2, [ip, #TF_charH]		@ r2 = charH

	@ Adjust for position & setup loop.
	@ r1, r2, (r4), (r6), r7, r10, r11, ip in use.
	mov		r3, r4, lsr #16			@ y
	bic		r4, r4, r3, lsl #16		@ x
	mov		r5, r4, lsr #3
	mla		r0, r11, r5, r6			@ -r0 = &dstD[y + x/8*dstP]
	add		r0, r0, r3, lsl #2		@ /

	and		r9, r4, #7				@ - r9 = xshift0
	mov		r9, r9, lsl #2			@ /
	mov		r5, #15					@ r5 = pxmask
	
	cmp		r10, #8
	suble	sp, sp, #8
	ble		.Lyloop
	ldrb	r3, [ip, #TF_cellH]
	sub		r3, r3, r2
	push	{r2, r3}
	b .Lyloop_prolog

	@ r0	dstL
	@ r1	srcL
	@ r2	iy, decrementing
	@ r3	raw
	@ r4	target
	@ r5	pxmask (=15)
	@ r6	xstride
	@ r7	ink
	@ r8	xshift
	@ r9	xhift0 = (x%8*4)
	@ r10	is, decrementing
	@ r11	dstP
	@ ip	dstD

	@ --- strip loop ---
.Lsloop:
		ldmia	sp, {r2, r3}		@ Reload charH and deltaS
		add		r1, r1, r3
		add		r0, ip, r11			@ (Re)set dstD/dstL
.Lyloop_prolog:
		mov		ip, r0
		@ --- row loop ---
.Lyloop:
			mov		r8, r9			@ xshift
			mov		r6, #0			@ xstride (rtile offset)
			ldrb	r3, [r1], #1
			cmp		r3, #0
			beq		.Lempty
				@ --- pixel loop ---
.Lxloop:
				tst		r3, #1		@ must draw?
				beq		.Lnopx
				cmp		r8, #32		@ moving to rtile?
				subge	r8, r8, #32
				addge	r6, r11
				ldr		r4, [r0, r6]
				bic		r4, r5, lsl r8
				orr		r4, r7, lsl r8
				str		r4, [r0, r6]
.Lnopx:
				add		r8, r8, #4
				movs	r3, r3, lsr #1
				bne		.Lxloop
.Lempty:
			add		r0, #4
			subs	r2, #1
			bne		.Lyloop
		@ strip loop
		subs	r10, #8
		bgt		.Lsloop
	
	add		sp, sp, #8
	pop		{r4-r11}
	bx		lr
END_FUNC(chr4c_drawg_b1cts_asm_1p_arm_iwram)

// EOF
