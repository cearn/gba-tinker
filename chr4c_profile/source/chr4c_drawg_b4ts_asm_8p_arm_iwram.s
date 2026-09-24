///
/// Col-major tile character renderer. 4bpp, any size, transparent
/// This is the optimized 'fast' 'version.
///
/// @file chr4c_drawg_b4ts_asm_8p_arm_iwram.s
/// @author J Vijn
/// @date 20251202 - 20251202
///
/// === NOTES ===

#include <tonc_asminc.h>
#include "tte_types.s.inc"

@ IWRAM_CODE void chr4c_drawg_b4ts_asm_8p_arm_iwram(int gid);
BEGIN_FUNC_ARM(chr4c_drawg_b4ts_asm_8p_arm_iwram, CSEC_IWRAM)
	push	{r4-r11, lr}
	ldr		r3, =gp_tte_context
	ldr		r3, [r3]
	ldmia	r3, {r6, r11}			@ r6 = dstBase, r11 = dstP
	add		r3, r3, #TTC_cursorX
	ldmia	r3, {r4, r5}			@ YX, font. r3 free
	
	ldmia	r5, {r1, r3}			@ glyphs, widths
	cmp		r3, #0
	ldrneb	r10, [r3, r0]			@ - r10 = charW
	ldreqb	r10, [r5, #TF_charW]	@ /
	ldrh	r3, [r5, #TF_cellS]
	mla		r1, r3, r0, r1			@ r1 = &glyphs[gid]
	ldrb	r2, [r5, #TF_charH]		@ r2 = charH
	
	@ Adjust for position & setup loop.
	@ r1, r2, (r4), r5, (r6), r10, r11 in use.
	mov		r3, r4, lsr #16			@ y
	bic		r4, r4, r3, lsl #16		@ x
	mov		ip, r4, lsr #3
	mla		r0, r11, ip, r6			@ -r0 = &dstD[y + x/8*dstP]
	add		r0, r0, r3, lsl #2		@ /
	
	and		r3, r4, #7
	mov		r8, r3, lsl #2			@ r8 = lshift = x%8*4

	rsb		r9, r8, #32				@ r9 = rshift = 32-lshift
	mov		r6, #1
	rsb		ip, r6, r6, lsl r9		@ ip = lmask = (1<<rshift)-1
	orr		r6, r6, r6, lsl #16		@ -- r6 = 0x11111111 (PONDER: via ldr?)
	orr		r6, r6, r6, lsl #8		@ -/
	orr		r6, r6, r6, lsl #4		@ /

	@ Move base to bottom of char.
	add		r0, r0, r2, lsl #2		@ r0 += charH
	neg		r2, r2					@ r2 = -charH
	
	cmp		r10, #8
	suble	sp, sp, #8
	ble		.Lyloop_prolog
	ldrb	r5, [r5, #TF_cellH]		@ r5 = cellH
	add		r3, r5, r2
	push	{r2, r3}				@ -charH, cellH-charH
	b		.Lyloop_prolog
	
	@ Register assignments:
	@ r0	dstLeft
	@ r1	srcL
	@ r2	-iy, incrementing
	@ r3	scratch, source row
	@ r4	px
	@ r5	pxmask
	@ r6	bupmask (0x11111111)
	@ r7	(FREE?)
	@ r8	lshift
	@ r9	rshift
	@ r10	is, decrementing
	@ r11	dstP
	@ ip	lmask
	@ lr	dstRight (=dstLeft + dstP)
	@ sp00	-charH
	@ sp04	srcStride = cellH-charH
.Lsloop:
		ldmia	sp, {r2, r3}
		add		r1, r1, r3
		add		r0, r0, r11
.Lyloop_prolog:
		add		lr, r0, r11
.Lyloop:
			ldr		r4, [r1], #4
			orrs	r5, r4, r4, lsr #2
			beq		.Lempty				@ Skip if no pixels
			orr		r5, r5, r5, lsr #1
			and		r5, r5, r6
			rsb		r5, r5, r5, lsl #4

			@ Left tile
			ldr		r3, [r0, r2, lsl #2]
			bic		r3, r3, r5, lsl r8
			orr		r3, r3, r4, lsl r8
			str		r3, [r0, r2, lsl #2]
			
			@ Right tile
			bics	r3, r4, ip
			ldrne	r3, [lr, r2, lsl #2]
			bicne	r3, r3, r5, lsr r9
			orrne	r3, r3, r4, lsr r9
			strne	r3, [lr, r2, lsl #2]
.Lempty:
			adds	r2, r2, #1
			blt		.Lyloop
		@ strip test
		subs		r10, #8
		bgt		.Lsloop
	add		sp, sp, #8
	pop		{r4-r11, lr}
	bx		lr
END_FUNC(chr4c_drawg_b4ts_asm_8p_arm_iwram)

@ EOF
