///
///  TTE profiling
/// @author J Vijn
/// @date 2025-08-10
///
/// === NOTES ===

#include <tonc.h>
#include <stdio.h>
#include <string.h>

#include "../build/all_gfx.h"

CSTR glados[] = 
{
"Please note that we have added a consequence for failure. "
	"Any contact with the chamber floor will result in an "
	"'unsatisfactory' mark on your official testing record "
	"followed by death. Good luck!", 

"Assume the party escort submission "
	"position or you will miss the party.", 
"Okay. The test is over. You win. "
	"Go back to the recovery annex. "
	"For your cake.", 

"[pain noise] You think you're doing "
"some damage? Two plus two is ten... "
	"IN BASE FOUR! I'M FINE!", 

"I invited your best friend the companion "
"cube. Of course, he couldn't come because you murdered him."
};

typedef struct TProfiler TProfiler;
struct TProfiler
{
	const char	*str;
	void		(*proc)(TProfiler *prof, const char *str);
	int			time;
	bool		check;
 };

void prof_null_drawg(TProfiler *prof, const char *str);
void prof_chr4c_base(TProfiler *prof, const char *str, fnDrawg proc);

#define GEN_DEF_TESTER(codesec, affix)											\
	codesec void chr4c_drawg_b1cts_##affix (uint gid);							\
	void prof_chr4c_drawg_b1cts_##affix (TProfiler* prof, const char* str) {	\
		prof_chr4c_base(prof, str, chr4c_drawg_b1cts_##affix);					\
	}

GEN_DEF_TESTER( , c_1p_thumb_rom);
GEN_DEF_TESTER( , c_8p_thumb_rom);

GEN_DEF_TESTER( , c_1p_arm_rom);
GEN_DEF_TESTER( , c_8p_arm_rom);

GEN_DEF_TESTER(IWRAM_CODE, c_1p_thumb_iwram);
GEN_DEF_TESTER(IWRAM_CODE, c_8p_thumb_iwram);

GEN_DEF_TESTER(IWRAM_CODE, c_1p_arm_iwram);
GEN_DEF_TESTER(IWRAM_CODE, c_8p_arm_iwram);

GEN_DEF_TESTER(IWRAM_CODE, asm_1p_arm_iwram);
GEN_DEF_TESTER(IWRAM_CODE, asm_8p_arm_iwram);

TProfiler g_profs[]= 
{
	{	"null",				prof_null_drawg, 0 }, 
	// 1 px variants	
	{	"c_1p_th_rom",		prof_chr4c_drawg_b1cts_c_1p_thumb_rom, 0 }, 
	{	"c_1p_th_iw",		prof_chr4c_drawg_b1cts_c_1p_thumb_iwram, 0 }, 
	{	"c_1p_arm_rom",		prof_chr4c_drawg_b1cts_c_1p_arm_rom, 0 }, 
	{	"c_1p_arm_iw",		prof_chr4c_drawg_b1cts_c_1p_arm_iwram, 0 }, 
	{	"asm_1p_arm_iw",	prof_chr4c_drawg_b1cts_asm_1p_arm_iwram, 0 }, 

	// 8px variants
	{	"c_8p_th_rom",		prof_chr4c_drawg_b1cts_c_8p_thumb_rom, 0 }, 
	{	"c_8p_th_iw",		prof_chr4c_drawg_b1cts_c_8p_thumb_iwram, 0 }, 
	{	"c_8p_arm_rom",		prof_chr4c_drawg_b1cts_c_8p_arm_rom, 0 }, 
	{	"c_8p_arm_iw",		prof_chr4c_drawg_b1cts_c_8p_arm_iwram, 0 }, 
	{	"asm_8p_arm_iw",	prof_chr4c_drawg_b1cts_asm_8p_arm_iwram, 0 }, 
};

TFont g_font;

// --------------------------------------------------------------------
// Additional renderers
// --------------------------------------------------------------------
void null_drawg(uint gid)
{
}

// --------------------------------------------------------------------
// Profiler routines
// --------------------------------------------------------------------

void prof_draws(TProfiler *prof, const char *str)
{
	tte_set_pos(0, 0);
	tte_write(prof->str);
	tte_write(": \n");

	tte_set_pos(0, 24);
	VBlankIntrWait();

	profile_start();
	tte_write(str);

	prof->time = profile_stop();
}

void prof_null_drawg(TProfiler *prof, const char *str)
{
	REG_DISPCNT = DCNT_MODE3 | DCNT_OBJ | DCNT_OBJ_1D;
	
	tte_init_bmp(3, &g_font, null_drawg);

	prof_draws(prof, str);
}

void prof_chr4c_base(TProfiler *prof, const char *str, fnDrawg proc)
{
	REG_DISPCNT = DCNT_MODE0 | DCNT_BG2 | DCNT_OBJ | DCNT_OBJ_1D;
	
	tte_init_chr4c(2, BG_CBB(0)|BG_SBB(24), 0xF000, 0x0F0201, CLR_YELLOW, &g_font, proc);

	tte_erase_screen();
	prof_draws(prof, str);
}

void test_check_render_init()
{
	TSurface* srf = &tte_get_context()->dst;

	// Render base text
	REG_DISPCNT = 0;
	tte_init_chr4c(2, BG_CBB(0)|BG_SBB(24), 0xF000, 0x0F0201, CLR_MONEYGREEN, &g_font, chr4c_drawg_b1cts);
	
	TProfiler prof = {};
	tte_erase_screen();
	prof_draws(&prof, glados[0]);
	
	// Copy (0, 24) - (240, 160) to VRAM+6000h for later comparison.
	const uint row0 = 3;
	const uint srcRows = srf->height/8, dstRows = srcRows-row0-1;
	TILE* src = &((TILE*)srf->data)[row0];
	TILE* dst = &tile_mem[1][256];

	for (uint tx=0; tx<srf->width/8; tx++)
		memcpy32(&dst[tx*dstRows], &src[tx*srcRows], dstRows*sizeof(TILE)/4);
}

/// Checks whether the rendering went alright.
bool test_check_render()
{
	TSurface* srf = &tte_get_context()->dst;

	const uint row0 = 3;
	const uint srcRows = srf->height/8, checkRows = srcRows-row0-1;
	TILE* src = &((TILE*)srf->data)[row0];
	TILE* check = &tile_mem[1][256];

	uint tx = 0;
	for (; tx<srf->width/8; tx++)
	{
		if (memcmp(&src[tx*srcRows], &check[tx*checkRows], checkRows*sizeof(TILE)))
			break;
	}
	if (tx == srf->width/8)
	{
		oam_mem[0].attr0 &= ~ATTR0_HIDE;
		oam_mem[1].attr0 |=  ATTR0_HIDE;
		return true;
	}
	else
	{
		oam_mem[0].attr0 |=  ATTR0_HIDE;
		oam_mem[1].attr0 &= ~ATTR0_HIDE;
		return false;
	}
}

//! Testing rendering speeds.
void test_speeds()
{
	RegisterRamReset(RESET_GFX);

	// --- Setup render ---
	int i;
	const char *str = glados[0];
	const uint len = strlen(str);

	g_font = vwf_default;

	// --- Setup checkers ---
	pal_bg_bank[15][15] = CLR_GRAY;
	test_check_render_init();

	GRIT_CPY(tile_mem[5], checkmarksTiles);
	GRIT_CPY(pal_obj_bank[0], checkmarksPal);

	memset32(oam_mem, ATTR0_HIDE, OAM_SIZE/4);

	obj_set_attr(&oam_mem[0], 
		ATTR0_4BPP | ATTR0_SQUARE | (SCREEN_HEIGHT-16), 
		ATTR1_SIZE_16 | (SCREEN_WIDTH-16), 
		ATTR2_PALBANK(0) | 512
	);
	obj_set_attr(&oam_mem[1], 
		ATTR0_4BPP | ATTR0_SQUARE | ATTR0_HIDE | (SCREEN_HEIGHT-16),
		ATTR1_SIZE_16 | (SCREEN_WIDTH-16), 
		ATTR2_PALBANK(0) | (512+4)
	);

	// --- Running tests ---
	for(i=0; i<countof(g_profs); i++)
	{
		g_profs[i].proc(&g_profs[i], str);
		g_profs[i].check = test_check_render();
		// Next on 16 frames or key-down.
		for (int j=0; j<16; j++)
		{
			VBlankIntrWait();
			if (key_is_down(KEY_ANY))
				break;
		}
	}

	// --- Display results ---
	RegisterRamReset(RESET_GFX);
	REG_DISPCNT= DCNT_MODE0 | DCNT_BG2;

	tte_init_se_default(2, BG_CBB(0) | BG_SBB(24));
	tte_init_con();
	pal_bg_bank[14][1] = CLR_ORANGE;

	iprintf("%d\n", len);
	// I miss my c++ ranged-for loops :(
	for(i=0; i<countof(g_profs); i++)
	{
		TProfiler* prof = &g_profs[i];
		tte_set_special(SE_PALBANK(14+(i&1)));
		iprintf("%-13s %6d  %d\n", prof->str, prof->time, prof->time/len);
	}

	key_wait_till_hit(KEY_START);
}

int main()
{
	irq_init(NULL);
	irq_add(II_VBLANK, NULL);

	test_speeds();

	while(1)
		VBlankIntrWait();
}
