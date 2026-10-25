
#include <tonc_tte.h>

#define LIKELY(exp) __builtin_expect(exp, 1)
#define UNLIKELY(exp) __builtin_expect(exp, 0)

uint utf8_decode_char(const char *ptr, char **endptr);

INLINE void _tte_newline(TTC* tc)
{
	tc->cursorY += tc->font->charH;
	tc->cursorX  = tc->marginLeft;
}

#if 0
	ldr	r3, [r4, #24]
	ldrh	r2, [r4, #22]
	ldrb	r3, [r3, #17]
	adds	r3, r3, r2
	strh	r3, [r4, #22]
	ldrh	r3, [r4, #44]
	movs	r0, r5
	strh	r3, [r4, #20]
	b	.L8
#endif

/// Original.
int	tte_write_a(const char *text)
{
	if(text == NULL)
		return 0;

	uint ch, gid;
	char *str= (char*)text;
	TTC *tc= tte_get_context();
	TFont *font;

	while( (ch=*str) != '\0' )
	{
		str++;
		switch(ch)
		{
		// --- Newline/carriage return ---
		case '\r':
			if(str[0] == '\n')	// deal with CRLF pair
				str++;
			// FALLTHRU
		case '\n':
			_tte_newline(tc);
			break;
		// --- Tab ---
		case '\t':
			tc->cursorX= (tc->cursorX/TTE_TAB_WIDTH+1)*TTE_TAB_WIDTH;
			break;

		// --- Normal char ---
		default:
			// Command sequence
			if(ch=='#' && str[0]=='{')
			{
				str= tte_cmd_default(str+1);
				break;
			}
			// Escaped command: skip '\\' and print '#'
			else if(ch=='\\' && str[0]=='#')
				ch= *str++;
			// Check for UTF8 code
			else if(ch>=0x80)
				ch= utf8_decode_char(str-1, &str);

			// Get glyph index and call renderer
			font= tc->font;
			gid= ch - font->charOffset;
			if(tc->charLut)
				gid= tc->charLut[gid];

			// Character wrap
			int charW= font->widths ? font->widths[gid] : font->charW;
			if(tc->cursorX+charW > tc->marginRight)
				_tte_newline(tc);

			// Draw and update position
			tc->drawgProc(gid);
			tc->cursorX += charW;
		}
	}

	// Return characters used (PONDER: is this really the right thing?)
	return str - text;
}

/// Original, with likelies (where possible)
int	tte_write_a2(const char *text)
{
	if(text == NULL)
		return 0;

	uint ch, gid;
	char *str= (char*)text;
	TTC *tc= tte_get_context();
	TFont *font;

	while( (ch=*str) != '\0' )
	{
		str++;
		if (ch == '\r')
		{
			// --- Newline/carriage return ---
			if(str[0] == '\n')	// deal with CRLF pair
				str++;
			_tte_newline(tc);
			continue;
		}
		else if (ch == '\n')
		{
			// Just newline.
			_tte_newline(tc);
			continue;
		}
		else if (ch == '\t')
		{
			// --- Tab ---
			tc->cursorX= (tc->cursorX/TTE_TAB_WIDTH+1)*TTE_TAB_WIDTH;
			continue;
		}
		else if(ch=='#' && str[0]=='{')
		{
			// Command sequence
			str= tte_cmd_default(str+1);
			continue;
		}
		else if(ch=='\\' && str[0]=='#')
		{
			// Escaped command: skip '\\' and print '#'
			ch= *str++;
		}
		else if(ch>=0x80)
		{
			// Check for UTF8 code
			ch= utf8_decode_char(str-1, &str);
		}

		// --- Normal char ---
		// Get glyph index and call renderer
		font= tc->font;
		gid= ch - font->charOffset;
		if(tc->charLut)
			gid= tc->charLut[gid];

		// Character wrap
		int charW= font->widths ? font->widths[gid] : font->charW;
		if(tc->cursorX+charW > tc->marginRight)
			_tte_newline(tc);

		// Draw and update position
		tc->drawgProc(gid);
		tc->cursorX += charW;
	}

	// Return characters used (PONDER: is this really the right thing?)
	return str - text;
}

/// Original, with likelies (where possible)
int	tte_write_a2x(const char *text)
{
	if(text == NULL)
		return 0;

	uint ch, gid;
	char *str= (char*)text;
	TTC *tc= tte_get_context();
	TFont *font;

	while( (ch=*str) != '\0' )
	{
		str++;
		if (UNLIKELY(ch == '\r'))
		{
			// --- Newline/carriage return ---
			if(str[0] == '\n')	// deal with CRLF pair
				str++;
			_tte_newline(tc);
			continue;
		}
		else if (UNLIKELY(ch == '\n'))
		{
			// Just newline.
			_tte_newline(tc);
			continue;
		}
		else if (UNLIKELY(ch == '\t'))
		{
			// --- Tab ---
			tc->cursorX= (tc->cursorX/TTE_TAB_WIDTH+1)*TTE_TAB_WIDTH;
			continue;
		}
		else if(UNLIKELY(ch=='#' && str[0]=='{'))
		{
			// Command sequence
			str= tte_cmd_default(str+1);
			continue;
		}
		else if(UNLIKELY(ch=='\\' && str[0]=='#'))
		{
			// Escaped command: skip '\\' and print '#'
			ch= *str++;
		}
		else if(ch>=0x80)
		{
			// Check for UTF8 code
			ch= utf8_decode_char(str-1, &str);
		}

		// --- Normal char ---
		// Get glyph index and call renderer
		font= tc->font;
		gid= ch - font->charOffset;
		if(tc->charLut)
			gid= tc->charLut[gid];

		// Character wrap
		int charW= font->widths ? font->widths[gid] : font->charW;
		if(tc->cursorX+charW > tc->marginRight)
			_tte_newline(tc);

		// Draw and update position
		tc->drawgProc(gid);
		tc->cursorX += charW;
	}

	// Return characters used (PONDER: is this really the right thing?)
	return str - text;
}

/// switchless, special-white, with likelies (where possible)
int	tte_write_a3x(const char *text)
{
	if(text == NULL)
		return 0;

	uint ch, gid;
	char *str= (char*)text;
	TTC *tc= tte_get_context();
	TFont *font;

	while( (ch=*str) != '\0' )
	{
		str++;
		if (UNLIKELY(ch <= '\r'))
		{
			if (ch == '\r')
			{
				// --- Newline/carriage return ---
				if(str[0] == '\n')	// deal with CRLF pair
					str++;
				_tte_newline(tc);
				continue;
			}
			else if (ch == '\n')
			{
				// Just newline.
				_tte_newline(tc);
				continue;
			}
			else if (ch == '\t')
			{
				// --- Tab ---
				tc->cursorX= (tc->cursorX/TTE_TAB_WIDTH+1)*TTE_TAB_WIDTH;
				continue;
			}
		}
		else if(UNLIKELY(ch=='#' && str[0]=='{'))
		{
			// Command sequence
			str= tte_cmd_default(str+1);
			continue;
		}
		else if(UNLIKELY(ch=='\\' && str[0]=='#'))
		{
			// Escaped command: skip '\\' and print '#'
			ch= *str++;
		}
		else if(ch>=0x80)
		{
			// Check for UTF8 code
			ch= utf8_decode_char(str-1, &str);
		}

		// --- Normal char ---
		// Get glyph index and call renderer
		font= tc->font;
		gid= ch - font->charOffset;
		if(tc->charLut)
			gid= tc->charLut[gid];

		// Character wrap
		int charW= font->widths ? font->widths[gid] : font->charW;
		if(tc->cursorX+charW > tc->marginRight)
			_tte_newline(tc);


		// Draw and update position
		tc->drawgProc(gid);
		tc->cursorX += charW;
	}

	// Return characters used (PONDER: is this really the right thing?)
	return str - text;
}

/// Original, with likelies (where possible)
int	tte_write_ax(const char *text)
{
	if(text == NULL)
		return 0;

	uint ch, gid;
	char *str= (char*)text;
	TTC *tc= tte_get_context();
	TFont *font;

	while( (ch=*str) != '\0' )
	{
		str++;
		switch(ch)
		{
		// --- Newline/carriage return ---
		case '\r':
			if(str[0] == '\n')	// deal with CRLF pair
				str++;
			// FALLTHRU
		case '\n':
			_tte_newline(tc);
			break;
		// --- Tab ---
		case '\t':
			tc->cursorX= (tc->cursorX/TTE_TAB_WIDTH+1)*TTE_TAB_WIDTH;
			break;

		// --- Normal char ---
		default:
			// Command sequence
			if(UNLIKELY(ch=='#' && str[0]=='{'))
			{
				str= tte_cmd_default(str+1);
				break;
			}
			// Escaped command: skip '\\' and print '#'
			else if(UNLIKELY(ch=='\\' && str[0]=='#'))
				ch= *str++;
			// Check for UTF8 code
			else if(ch>=0x80)
				ch= utf8_decode_char(str-1, &str);

			// Get glyph index and call renderer
			font= tc->font;
			gid= ch - font->charOffset;
			if(tc->charLut)
				gid= tc->charLut[gid];

			// Character wrap
			int charW= font->widths ? font->widths[gid] : font->charW;
			if(tc->cursorX+charW > tc->marginRight)
				_tte_newline(tc);

			// Draw and update position
			tc->drawgProc(gid);
			tc->cursorX += charW;
		}
	}

	// Return characters used (PONDER: is this really the right thing?)
	return str - text;
}


// A: command first, full if/else
int tte_write_b(const char *text)
{
	if(text == NULL)
		return 0;

	uint ch, gid;
	char *str= (char*)text;
	TTC *tc= tte_get_context();
	TFont *font;

	while( (ch=*str) != '\0' )
	{
		str++;

		if(ch=='#' && str[0]=='{')
		{
			str= tte_cmd_default(str+1);
			continue;
		}
		else if(ch=='\\' && str[0]=='#')
		{
			// Escaped command: skip '\\' and print '#'
			ch= *str++;
		}
		else if (ch == '\r')
		{
			if(str[0] == '\n')	// deal with CRLF pair
				str++;
			_tte_newline(tc);
			continue;
		}
		else if (ch =='\n')
		{
			_tte_newline(tc);
			continue;
		}
		else if (ch == '\t')
		{
			tc->cursorX= (tc->cursorX/TTE_TAB_WIDTH+1)*TTE_TAB_WIDTH;
			continue;
		}
		else if (ch >= 0x80)
		{
			ch= utf8_decode_char(str-1, &str);
		}

		// Get glyph index and call renderer
		font= tc->font;
		gid= ch - font->charOffset;
		if(tc->charLut)
			gid= tc->charLut[gid];

		// Character wrap
		int charW= font->widths ? font->widths[gid] : font->charW;
		if(tc->cursorX+charW > tc->marginRight)
			_tte_newline(tc);

		// Draw and update position
		tc->drawgProc(gid);
		tc->cursorX += charW;
	}

	// Return characters used (PONDER: is this really the right thing?)
	return str - text;
}

// A: command first, full if/else
int tte_write_bx(const char *text)
{
	if(text == NULL)
		return 0;

	uint ch, gid;
	char *str= (char*)text;
	TTC *tc= tte_get_context();
	TFont *font;

	while( (ch=*str) != '\0' )
	{
		str++;

		if(ch=='#' && str[0]=='{')
		{
			str= tte_cmd_default(str+1);
			continue;
		}
		else if(ch=='\\' && str[0]=='#')
		{
			// Escaped command: skip '\\' and print '#'
			ch= *str++;
		}
		else if (ch == '\r')
		{
			if(str[0] == '\n')	// deal with CRLF pair
				str++;
			tc->cursorY += tc->font->charH;
			tc->cursorX  = tc->marginLeft;
			continue;
		}
		else if (ch =='\n')
		{
			tc->cursorY += tc->font->charH;
			tc->cursorX  = tc->marginLeft;
			continue;
		}
		else if (ch == '\t')
		{
			tc->cursorX= (tc->cursorX/TTE_TAB_WIDTH+1)*TTE_TAB_WIDTH;
			continue;
		}
		else if (ch >= 0x80)
		{
			ch= utf8_decode_char(str-1, &str);
		}

		// Get glyph index and call renderer
		font= tc->font;
		gid= ch - font->charOffset;
		if(tc->charLut)
			gid= tc->charLut[gid];

		// Character wrap
		int charW= font->widths ? font->widths[gid] : font->charW;
		if(tc->cursorX+charW > tc->marginRight)
		{
			tc->cursorY += font->charH;
			tc->cursorX  = tc->marginLeft;
		}

		// Draw and update position
		tc->drawgProc(gid);
		tc->cursorX += charW;
	}

	// Return characters used (PONDER: is this really the right thing?)
	return str - text;
}


// C, with likelies
int tte_write_cx(const char *text)
{
	if(text == NULL)
		return 0;

	uint ch, gid;
	char *str= (char*)text;
	TTC *tc= tte_get_context();
	TFont *font;

	while( (ch=*str) != '\0' )
	{
		str++;

		if(UNLIKELY(ch=='#' && str[0]=='{'))
		{
			str= tte_cmd_default(str+1);
			continue;
		}
		else if(UNLIKELY(ch=='\\' && str[0]=='#'))
		{
			// Escaped command: skip '\\' and print '#'
			ch= *str++;
		}
		else if (UNLIKELY(ch < ' '))
		{
			if (ch == '\r')
			{
				if(str[0] == '\n')	// deal with CRLF pair
					str++;
				tc->cursorY += tc->font->charH;
				tc->cursorX  = tc->marginLeft;
				continue;
			}
			else if (ch =='\n')
			{
				tc->cursorY += tc->font->charH;
				tc->cursorX  = tc->marginLeft;
				continue;
			}
			else if (ch == '\t')
			{
				tc->cursorX= (tc->cursorX/TTE_TAB_WIDTH+1)*TTE_TAB_WIDTH;
				continue;
			}
		}
		else if (ch >= 0x80)
		{
			ch= utf8_decode_char(str-1, &str);
		}

		// Get glyph index and call renderer
		font= tc->font;
		gid= ch - font->charOffset;
		if(tc->charLut)
			gid= tc->charLut[gid];

		// Character wrap
		int charW= font->widths ? font->widths[gid] : font->charW;
		if(tc->cursorX+charW > tc->marginRight)
		{
			tc->cursorY += font->charH;
			tc->cursorX  = tc->marginLeft;
		}

		// Draw and update position
		tc->drawgProc(gid);
		tc->cursorX += charW;
	}

	// Return characters used (PONDER: is this really the right thing?)
	return str - text;
}

int tte_write_d(const char *text)
{
	if(text == NULL)
		return 0;

	uint ch, gid;
	char *str= (char*)text;
	TTC *tc= tte_get_context();
	TFont *font;

	while( (ch=*str) != '\0' )
	{
		str++;

		if(ch=='#' && str[0]=='{')
		{
			str= tte_cmd_default(str+1);
			continue;
		}
		else if(ch=='\\' && str[0]=='#')
		{
			// Escaped command: skip '\\' and print '#'
			ch= *str++;
		}

		if (ch < ' ')
		{
			if (ch == '\r')
			{
				if(str[0] == '\n')	// deal with CRLF pair
					str++;
				_tte_newline(tc);
				continue;
			}
			else if (ch =='\n')
			{
				_tte_newline(tc);
				continue;
			}
			else if (ch == '\t')
			{
				tc->cursorX= (tc->cursorX/TTE_TAB_WIDTH+1)*TTE_TAB_WIDTH;
				continue;
			}
		}
		else if (ch >= 0x80)
		{
			ch= utf8_decode_char(str-1, &str);
		}

		// Get glyph index and call renderer
		font= tc->font;
		gid= ch - font->charOffset;
		if(tc->charLut)
			gid= tc->charLut[gid];

		// Character wrap
		int charW= font->widths ? font->widths[gid] : font->charW;
		if(tc->cursorX+charW > tc->marginRight)
			_tte_newline(tc);

		// Draw and update position
		tc->drawgProc(gid);
		tc->cursorX += charW;
	}

	// Return characters used (PONDER: is this really the right thing?)
	return str - text;
}

/// Render a string.
///	@param text	String to parse and write.
///	@return		Number of parsed characters.
int tte_write_dx(const char *text)
{
	if(text == NULL)
		return 0;

	uint ch, gid;
	char *str= (char*)text;
	TTC *tc= tte_get_context();
	TFont *font;

	while( (ch=*str) != '\0' )
	{
		str++;

		if(UNLIKELY(ch=='#' && str[0]=='{'))
		{
			str= tte_cmd_default(str+1);
			continue;
		}
		else if(UNLIKELY(ch=='\\' && str[0]=='#'))
		{
			// Escaped command: skip '\\' and print '#'
			ch= *str++;
		}

		if (UNLIKELY(ch < ' '))
		{
			if (ch == '\r')
			{
				if(str[0] == '\n')	// deal with CRLF pair
					str++;
				_tte_newline(tc);
				continue;
			}
			else if (ch =='\n')
			{
				_tte_newline(tc);
				continue;
			}
			else if (ch == '\t')
			{
				tc->cursorX= (tc->cursorX/TTE_TAB_WIDTH+1)*TTE_TAB_WIDTH;
				continue;
			}
		}
		else if (ch >= 0x80)
		{
			ch= utf8_decode_char(str-1, &str);
		}

		// Get glyph index and call renderer
		font= tc->font;
		gid= ch - font->charOffset;
		if(tc->charLut)
			gid= tc->charLut[gid];

		// Character wrap
		int charW= font->widths ? font->widths[gid] : font->charW;
		if(tc->cursorX+charW > tc->marginRight)
			_tte_newline(tc);

		// Draw and update position
		tc->drawgProc(gid);
		tc->cursorX += charW;
	}

	// Return characters used (PONDER: is this really the right thing?)
	return str - text;
}
