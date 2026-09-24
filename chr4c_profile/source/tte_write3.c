
#include <tonc_tte.h>

uint utf8_decode_char(const char *ptr, char **endptr);


///! Render a string.
///	@param text	String to parse and write.
///	@return		Number of parsed characters.
int tte_write3(const char *text)
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

		if(__builtin_expect(ch=='#' && str[0]=='{', 0))
		{
			str= tte_cmd_default(str+1);
			continue;
		}
		else if(__builtin_expect(ch=='\\' && str[0]=='#', 0))
		{
			// Escaped command: skip '\\' and print '#'
			ch= *str++;
		}

		if (__builtin_expect(ch < ' ', 0))
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
