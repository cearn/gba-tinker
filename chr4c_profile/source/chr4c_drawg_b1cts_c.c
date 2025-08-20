//
// Testing iwram versions of chr4c_drawg_b1cts. Let's see what the compiler can make of it.
// This is the ARM/ROM version.

#define GEN_CODE_SEC // empty

#define GEN_PROCNAME_1P chr4c_drawg_b1cts_c_1p_thumb_rom
#define GEN_PROCNAME_8P chr4c_drawg_b1cts_c_8p_thumb_rom

#include "chr4c_drawg_b1cts.c.inc"
