///
/// Col-major tile character renderer. 1->4bpp recolored, any size, transparent
/// This is the optimized 'fast' 'version.
///
/// @file chr4c_drawg_b1cts_asm_8p_arm_iwram.s
/// @author J Vijn
/// @date 20250719
///
/// === NOTES ===

#define FEATURE_SMALL_SPECIAL() 0

#define FEATURE_MASK_TEST() 1
#define FEATURE_MASK_LEFT_CND() 0

#include <tonc_asminc.h>
#include <../src/tte/tte_types.s>

@ IWRAM_CODE void chr4c_drawg_b1cts_asm_8p_arm_iwram(int gid);
BEGIN_FUNC_ARM(chr4c_drawg_b1cts_asm_8p_arm_iwram, CSEC_IWRAM)
	stmfd	sp!, {r4-r11, lr}

	ldr		r5,=gp_tte_context
	ldr		r5, [r5]
	
	@ Preload dstBase (r4), dstPitch (ip), yx (r6), font (r7)
	ldmia	r5, {r4, ip}
	add		r3, r5, #TTC_cursorX
	ldmia	r3, {r6, r7}

	@ Get srcD (r1), width (r11), charH (r2)
	ldmia	r7, {r1, r3}			@ Load data, widths
	cmp		r3, #0
	ldrneb	r11, [r3, r0]			@ Var charW
	ldreqb	r11, [r7, #TF_charW]	@ Fixed charW
	ldrh	r3, [r7, #TF_cellS]
	mla		r1, r3, r0, r1			@ srcL
	ldrb	r2, [r7, #TF_charH]		@ charH
	ldrb	r10, [r7, #TF_cellH]	@ cellH PONDER: load later?

	@ Positional issues: dstD(r0), lsl(r8), lsr(r9), right(lr), cursorX 
	mov		r3, r6, lsr #16			@ r9 = y
	bic		r9, r6, r3, lsl #16		@ x

	add		r0, r4, r3, lsl #2		@ dstD= dstBase + y*4
	mov		r3, r9, lsr #3
	mla		r0, ip, r3, r0

	ldr		r6,=0x01010101
	ldrh	r7, [r5, #TTC_ink]

	and		r9, r9, #7				@ x%7
	mov		r8, r9, lsl #2			@ lsl = x%8*4
#if FEATURE_SMALL_SPECIAL() == 1
	add		lr, r11, r9
	cmp		lr, #8
	blt		.Lleft_only
#endif

#if FEATURE_MASK_TEST() == 1
	rsb		r9, r8, #32				@ lsr = 32-x%8*4
	mov		lr, #1
	rsb		lr, lr, lr, lsl r9		@ lr = (1<<rshift)-1 : bits that stay in ltile
#else
	add		lr, r11, r9				@ right= width + x%8
	rsb		r9, r8, #32				@ lsr = 32-x%8*4
#endif

	@ --- Reg-list for strip/render loop ---
	@ r0	dstL
	@ r1	srcL
	@ r2	scanline looper
	@ r3	raw
	@ r4	px / tmp
	@ r5	pxmask
	@ r6	bitmask
	@ r7	ink
	@ r8	left shift
	@ r9	right shift
	@ r10	dstD		@# TODO: unnecessary; remove later
	@ r11	charW
	@ ip	dstP
#if FEATURE_MASK_TEST() == 1
	@ lr	ltile mask
#else
	@ lr	right edge
#endif
	@ sp00	charH
	@ sp04	deltaS = cellH-charH	 (delta srcL)

	cmp		r11, #8
	@ Prep for single-strip render
	suble	sp, sp, #8
	ble		.Lyloop
	@ Prep for multi-strip render
	sub		r3, r10, r2
	mov		r10, r0
	stmfd	sp!, {r2, r3}			@ Store charH, deltaS
	b		.Lyloop

	@ --- Strip loop ---
.Lsloop:
		ldmia	sp, {r2, r3}		@ Reload charH and deltaS
		add		r10, r10, ip		@ (Re)set dstD/dstL
		mov		r0, r10
		add		r1, r1, r3
#if FEATURE_MASK_TEST() != 1
		sub		lr, lr, #8
#endif

		@ --- Render loop ---
.Lyloop:
			@ Prep px and pxmask
			ldrb	r3, [r1], #1
			orrs	r3, r3, r3, lsl #12
			beq		.Lnopx				@ Skip if no pixels
			orr		r3, r3, r3, lsl #6
			and		r4, r3, r6, lsl #1
			and		r3, r3, r6
			orr		r3, r3, r4, lsl #3

			rsb		r5, r3, r3, lsl #4
			mul		r4, r3, r7

			@ Render to left tile
#if FEATURE_MASK_TEST() == 1
			@ Don't do this. Glyphs are usually left-aligned, so you'll almost always have 
			@ left-tile pixels anyway.
	#if FEATURE_MASK_LEFT_CND() == 1
			ands	r3, r4, lr
			ldrne	r3, [r0]
			bicne	r3, r3, r5, lsl r8
			orrne	r3, r3, r4, lsl r8
			strne	r3, [r0]
	#else
			ldr		r3, [r0]
			bic		r3, r3, r5, lsl r8
			orr		r3, r3, r4, lsl r8
			str		r3, [r0]
	#endif
#else
			ldr		r3, [r0]
			bic		r3, r3, r5, lsl r8
			orr		r3, r3, r4, lsl r8
			str		r3, [r0]
#endif
			@ Render to right tile 
#if FEATURE_MASK_TEST() == 1
			bics	r3, r4, lr
			ldrne	r3, [r0, ip]
			bicne	r3, r3, r5, lsr r9
			orrne	r3, r3, r4, lsr r9
			strne	r3, [r0, ip]
#else
			cmp		lr, #8
			ldrgt	r3, [r0, ip]
			bicgt	r3, r3, r5, lsr r9
			orrgt	r3, r3, r4, lsr r9
			strgt	r3, [r0, ip]
#endif
.Lnopx:
			add		r0, r0, #4
			subs	r2, r2, #1
			bgt		.Lyloop

		@ Test for strip loop
		subs	r11, r11, #8
		bgt		.Lsloop
	
	add		sp, sp, #8
	ldmfd	sp!, {r4-r11, lr}
	bx		lr

#if FEATURE_SMALL_SPECIAL() == 1
	@ A special case where we know we're rending into a single tile column.
#if 1
.Lleft_only:
	sub		r2, r2, #1
.Lleft_only_yloop:
		@ Prep px and pxmask
		ldrb	r3, [r1, r2]
		orrs	r3, r3, r3, lsl #12
		beq		.Lleft_only_nopx				@ Skip if no pixels
		orr		r3, r3, r3, lsl #6
		and		r4, r3, r6, lsl #1
		and		r3, r3, r6
		orr		r3, r3, r4, lsl #3

		rsb		r5, r3, r3, lsl #4
		mul		r4, r3, r7

		ldr		r3, [r0, r2, lsl #2]
		bic		r3, r3, r5, lsl r8
		orr		r3, r3, r4, lsl r8
		str		r3, [r0, r2, lsl #2]
.Lleft_only_nopx:
		subs	r2, r2, #1
		bge		.Lleft_only_yloop
#else
.Lleft_only:
.Lleft_only_yloop:
		@ Prep px and pxmask
		ldrb	r3, [r1], #1
		orrs	r3, r3, r3, lsl #12
		beq		.Lleft_only_nopx				@ Skip if no pixels
		orr		r3, r3, r3, lsl #6
		and		r4, r3, r6, lsl #1
		and		r3, r3, r6
		orr		r3, r3, r4, lsl #3

		rsb		r5, r3, r3, lsl #4
		mul		r4, r3, r7

		ldr		r3, [r0]
		bic		r3, r3, r5, lsl r8
		orr		r3, r3, r4, lsl r8
		str		r3, [r0]
.Lleft_only_nopx:
		add		r0, r0, #4
		subs	r2, r2, #1
		bgt		.Lleft_only_yloop
#endif
	ldmfd	sp!, {r4-r11, lr}
	bx		lr
#endif

END_FUNC(chr4c_drawg_b1cts_asm_8p_arm_iwram)

@ EOF
