///
/// tte_write experiments
///
/// @author J Vijn
/// @date 20250810 - 20250824
//
// === NOTES ===

#include <tonc.h>
#include <stdio.h>
#include <string.h>

#include "../build/all_gfx.h"
//#include "../fonts/fonts.h"

#define FEATURE_KEY_WAIT() 0

#define FEATURE_TTE_WRITE_ORG 0
#define FEATURE_TTE_WRITE_IWRAM 2
#define FEATURE_TTE_WRITE_ALT 3

#define FEATURE_TTE_WRITE_MODE() FEATURE_TTE_WRITE_ALT

#define FEATURE_FONT_BPP 1

CSTR c_glados[] = 
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

typedef struct Profiler Profiler;
struct Profiler
{
	const char	*name;		///< Name of routine.
	int			(*write_proc)(const char *text);		///< Test tte_write variant
	int			time_null;	///< (out) Time of tte_write call with null renderer.
	int			time_drawg;	///< (out) Time of tte_write call with chr4c renderer.
	bool		check;		///< Whether @a proc rendered the text correctly.
};


IWRAM_CODE void chr4c_drawg_b1cts_asm_8p_arm_iwram(uint gid);

#if REMOVE
void prof_null_drawg(Profiler *prof, const char *str);
void prof_chr4c_base(Profiler *prof, const char *str, fnDrawg proc);

#define GEN_DEF_TESTER_BASE(codesec, type, affix)									\
	codesec void chr4c_drawg_##type##_##affix (uint gid);							\
	int prof_chr4c_drawg_##type##_##affix (Profiler* prof, const char* str) {		\
		prof_chr4c_base(prof, str, chr4c_drawg_##type##_##affix);					\
	}

#define GEN_DEF_TESTER_B1(codesec, affix) GEN_DEF_TESTER_BASE(codesec, b1cts, affix)

GEN_DEF_TESTER_B1(IWRAM_CODE, asm_8p_arm_iwram);
#endif

int tte_write_a(const char* text);

Profiler g_testCases[] = 
{
	{	"lib",		tte_write, 0 }, 
	{	"a",		tte_write_a, 0 }, 
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

int prof_draws(Profiler *prof, const char *str)
{
	tte_set_pos(0, 0);
	tte_write(prof->name);
	tte_write(": \n");

	tte_set_pos(0, 24);
	VBlankIntrWait();

	profile_start();
	prof->write_proc(str);
	return profile_stop();
}

/// Gets the overhead of drawg calls.
void prof_null(Profiler *prof, const char *str)
{
	REG_DISPCNT = DCNT_MODE3 | DCNT_OBJ | DCNT_OBJ_1D;
	tte_init_bmp(3, &g_font, null_drawg);
	prof->time_null = prof_draws(prof, str);
}

/// Core profiler for chr4c routines.
/// Colors: 
/// - 01 for text. (yellow)
/// - 0F for background (grey)
void prof_chr4c(Profiler* prof, const char* str, fnDrawg proc)
{
	REG_DISPCNT = DCNT_MODE0 | DCNT_BG2 | DCNT_OBJ | DCNT_OBJ_1D;
	
	tte_init_chr4c(2, BG_CBB(0)|BG_SBB(24), 0xF000, 0x0F0001, CLR_YELLOW, &g_font, proc);

	tte_erase_screen();
	prof->time_drawg = prof_draws(prof, str);
}

void prof_write(Profiler* prof, const char* str)
{
	prof_null(prof, str);
	//prof_chr4c(prof, str, chr4c_drawg_b1cts);
	prof_chr4c(prof, str, chr4c_drawg_b1cts_asm_8p_arm_iwram);
}


/// Setup for rendering checking. chr4c-renders the test string to the screen and copies 
/// it to 0600:6000 for safe keeping and later used as reference data by @a test_check_render().
void test_check_render_init()
{
	TSurface* srf = &tte_get_context()->dst;
	
	// Render base text
	REG_DISPCNT = 0;
	tte_init_chr4c(2, BG_CBB(0)|BG_SBB(24), 0xF000, 0x0F0001, CLR_MONEYGREEN, &g_font, chr4c_drawg_b1cts);
	pal_bg_bank[2][3] = CLR_RED;
	
	Profiler prof = { "", tte_write };
	tte_erase_screen();
	prof_draws(&prof, c_glados[0]);
		
	// Copy (0, 24) - (240, 160) to VRAM+6000h for later comparison.
	const uint row0 = 3;
	const uint srcRows = srf->height/8, dstRows = srcRows-row0-1;
	TILE* src = &((TILE*)srf->data)[row0];
	TILE* dst = &tile_mem[1][256];

	for (uint tx=0; tx<srf->width/8; tx++)
	memcpy32(&dst[tx*dstRows], &src[tx*srcRows], dstRows*sizeof(TILE)/4);
}

/// Checks whether the rendering went alright. Checkmark or cross should appear 
/// in the bottom-right as an indicator.
bool test_check_render()
{
	TSurface* srf = &tte_get_context()->dst;

	const uint row0 = 3;
	const uint srcRows = srf->height/8;
	const uint checkRows = srcRows-row0-1;
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

/// Testing rendering speeds.
void test_speeds()
{
	RegisterRamReset(RESET_GFX);

	// --- Setup render ---
	const char *str = c_glados[0];
	const uint len = strlen(str);

	g_font = vwf_default;

	// --- Setup checkers ---
	pal_bg_bank[15][1] = CLR_YELLOW;
	pal_bg_bank[15][2] = RGB15(24, 24, 2);
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
	for(int i=0; i<countof(g_testCases); i++)
	{
		prof_write(&g_testCases[i], str);
		g_testCases[i].check = test_check_render();

#if FEATURE_KEY_WAIT() == 1
		key_wait_till_hit(KEY_ANY);
#else
		// Next on 16 frames or key-down.
		for (int j=0; j<16; j++)
		{
			VBlankIntrWait();
			key_poll();
			if (key_is_down(KEY_ANY))
				break;
			}
#endif
	}

	// --- Display results ---
	RegisterRamReset(RESET_GFX);
	REG_DISPCNT= DCNT_MODE0 | DCNT_BG2;

	tte_init_se_default(2, BG_CBB(0) | BG_SBB(24));
	tte_init_con();
	pal_bg_bank[14][1] = CLR_ORANGE;

	pal_bg_bank[2][3] = CLR_RED;

	iprintf("%d\n", len);
	// I miss my c++ ranged-for loops :(
	for(int i=0; i<countof(g_testCases); i++)
	{
		Profiler* prof = &g_testCases[i];
		tte_set_special(SE_PALBANK(14+(i&1)));
		iprintf("%c%-13s %5d %5d\n", 
			(prof->check ? ' ':'-'), prof->name, prof->time_null/len, prof->time_drawg/len);
	}

	key_wait_till_hit(KEY_START);
}

int main()
{
	irq_init(NULL);
	irq_add(II_VBLANK, NULL);

	g_font = vwf_default;

#if 0
	Profiler* prof = &g_testCases[0];

	REG_DISPCNT = DCNT_MODE0 | DCNT_BG2 | DCNT_OBJ | DCNT_OBJ_1D;
	
	tte_init_chr4c(2, BG_CBB(0)|BG_SBB(24), 0xF000, 0x0F0001, CLR_YELLOW, &vwf_default, NULL);

	tte_erase_screen();
	//prof_write(prof, "hello");

	//g_testCases[0].write_proc("hellxo!");
	prof->time_drawg = prof_draws(prof, "hi?");
#else
	test_speeds();
#endif
	//test_unpack();

	while(1)
		VBlankIntrWait();
}
