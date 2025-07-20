#ifndef _OCR_CHARPROPERTIES_INCLUDED
#define _OCR_CHARPROPERTIES_INCLUDED


enum
{
	CHAR_NOPROPERTIES	= 0,		// just new character

	CHAR_NOTSTARTCHAR	= 0x0000,	// not first character in group/word
	CHAR_STARTCHAR		= 0x0001,	// first character in group/word

	CHAR_CAPUNKNOWN		= 0x0000,	// not defined, if the character is capital or not
	CHAR_TALL			= 0x0010,	// character is taller than average one in line
	CHAR_NOTCAPITAL		= 0x0020,	// character recognized as not-capital by size
	CHAR_CAPITAL		= 0x0040,	// character is recognized as capital by size
	CHAR_UNIHEIGHT		= 0x0080,	// all characters in line has almost the same height

	CHAR_NOTALIGNED		= 0x1000,	// character is not aligned in string
	CHAR_SKIPCHAR		= 0x2000,	// character should not be recognized
};



#endif