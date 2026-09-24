
#include <tonc.h>
#include <stdio.h>
#include <string.h>

#include "../build/all_gfx.h"
#include "../fonts/verdana9_2.h"

extern CSTR c_glados[];

IWRAM_CODE void chr4c_drawg_b2ts_c_1p_arm_iwram(uint gid);
IWRAM_CODE void chr4c_drawg_b2ts_c_8p_arm_iwram(uint gid);

void test_unpack()
{
	const char* str = c_glados[0];

	REG_DISPCNT = DCNT_MODE0 | DCNT_BG2 | DCNT_OBJ | DCNT_OBJ_1D;
	
	tte_init_chr4c(2, BG_CBB(0)|BG_SBB(24), 0xF000, 0x000F0000, CLR_YELLOW, &verdana9_2bFont, chr4c_drawg_b2ts_c_8p_arm_iwram);
	tte_erase_screen();

	pal_bg_mem[0] = CLR_GRAY;
	pal_bg_bank[15][1] = CLR_YELLOW;
	pal_bg_bank[15][2] = CLR_ORANGE;
	pal_bg_bank[15][15] = CLR_CYAN;

	tte_write(str);
}
