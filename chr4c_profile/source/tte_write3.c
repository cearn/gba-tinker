
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

		if(ch=='#' && str[0]=='{')
		{
			str= tte_cmd_default(str+1);
			continue;
		}
		// Escaped command: skip '\\' and print '#'
		else if(ch=='\\' && str[0]=='#')
			ch= *str++;
		
		switch(ch)
		{
		// --- Newline/carriage return ---
		case '\r':
			if(str[0] == '\n')	// deal with CRLF pair
				str++;
			// FALLTHRU
		case '\n':
			tc->cursorY += tc->font->charH;
			tc->cursorX  = tc->marginLeft;
			break;
		// --- Tab ---
		case '\t':
			tc->cursorX= (tc->cursorX/TTE_TAB_WIDTH+1)*TTE_TAB_WIDTH;
			break;

		// --- Normal char ---
		default:
			// Check for UTF8 code
			if(ch>=0x80)
				ch= utf8_decode_char(str-1, &str);

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
	}

	// Return characters used (PONDER: is this really the right thing?)
	return str - text;
}
