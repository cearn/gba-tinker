
#include <stdio.h>
#include "common.h"

#include <thread>

typedef int32_t FIXED;

#define FEATURE_DPOS() 1

struct TTE_WRITER
{
	enum Flags {
		DONE = 0x01,
		PAUSED = 0x02,
	};

	const char* str;
#if FEATURE_DPOS() > 0
	u16		speed;	///< 8.8 speed.
	u16		ipos;	///< Position in string, int-part
	u8		dpos;	///< Subposition.
	u8		flags;	///< flags (not necessary?)
#else
	s32		pos;	///< 24.8 position.A new glyph will be written when an int-bound is crossed.
	u16		speed;	///< 8.8 speed.
	u8		flags;	///< flags (not necessary?)
#endif
};

void tte_writer_init(TTE_WRITER* tw, const char* str, u16 speed)
{
#if FEATURE_DPOS() > 0
	*tw = { str, speed };
	if (speed > 0 && speed < 0x100)
		tw->dpos = (u8)-speed;
#else
	*tw = { str, 0, speed };
	if (speed > 0 && speed < 0x100)
		tw->pos = 0x100 - speed;
#endif
}

uint tte_writer_update(TTE_WRITER* tw)
{
	if (tw->flags & (TTE_WRITER::DONE | TTE_WRITER::PAUSED) )
		return 0;

#if FEATURE_DPOS() > 0
	uint dpos = tw->dpos + tw->speed;
	uint count = dpos>>8;

	printf("[%04X + %04X] ", tw->ipos, (u16)dpos);
	const char* pstr = (const char*)(tw->str + tw->ipos);
#else
	s32 start = tw->pos, end = tw->pos + tw->speed;
	s32 istart = start>>8, iend = end>>8;
	uint count = iend-istart;

	printf("[%04X-%04X : %02X] ", (s16)start, (s16)end, count);
	const char* str = (const char*)(tw->str + istart);
#endif
	uint i;
	for (i=0; i<count; i++)
	{
		uint ch = *pstr;
		if (ch == '\0')
			break;
		pstr++;

		// Do normal writer stuff.
		printf("%c ", ch);
	}

#if FEATURE_DPOS() > 0
	if (*pstr == '\0')
	{
		// We're done.
		tw->ipos = pstr - tw->str;
		tw->dpos = 0;
		tw->flags |= TTE_WRITER::DONE;
	}
	else
	{
		// continue next time.
		tw->ipos += i;
		tw->dpos = (u8)dpos;
	}
#else
	if (*str == '\0')
	{
		// We're done.
		tw->pos = (str - tw->str);
		tw->flags |= TTE_WRITER::DONE;
	}
	else
	{
		// continue next time.
		tw->pos = end;
	}
#endif
	return i;
}

int main(int argc, char** argv)
{
	struct Test { const char* str; u16 speed; } tests[] = {
		{ "", 0x0000, },
		{ "", 0x0100, },
		{ "abcdefghij", 0x0100 },
		{ "abcdefghij", 0x0300 },
		{ "abcdefghij", 0x0180 },
		{ "abcdefghij", 0x0040 },
		{ "abcd", 0x1000 },
	};

	for (auto& test : tests)
	{
		TTE_WRITER tw;
		tte_writer_init(&tw, test.str, test.speed);

#if FEATURE_DPOS() > 0
		printf("TW: p=%04X+%02X, v=%04X, \"%s\"\n", (u16)tw.ipos, tw.dpos, tw.speed, tw.str);
#else
		printf("TW: p=%04X, v=%04X, \"%s\"\n", (u16)tw.pos, tw.speed, tw.str);
#endif
		while (~tw.flags & TTE_WRITER::DONE)
		{
			tte_writer_update(&tw);
			printf("\n");
		}
		printf("DONE\n\n");
	}

}
