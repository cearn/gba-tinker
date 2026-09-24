//
// Testing iwram versions of chr4_drawg_b1cts. Let's see what the compiler can make of it.
// This is the base version (ARM/iwram)

#include <tonc.h>

#define GEN_CODE_SEC IWRAM_CODE

#define GEN_PROCNAME_1P chr4c_drawg_b2ts_c_1p_arm_iwram
#define GEN_PROCNAME_8P chr4c_drawg_b2ts_c_8p_arm_iwram

#include "chr4c_drawg_b2ts.c.inc"