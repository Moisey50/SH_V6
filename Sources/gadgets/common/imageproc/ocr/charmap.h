#ifndef _OCR_CHARACTER_MAP_INCLUDED
#define _OCR_CHARACTER_MAP_INCLUDED

#include "imageproc\settings.h"
#include "imageproc\ocr\charproperties.h"

inline const char _char_printable_to_char_id(const char ch)
{
	switch (ch)
	{
	case 'c':
		return 'C';
	case 'i':
	case 'I':
	case '1':
		return 'l';
	case 'j':
		return 'J';
	case 'o':
	case '0':
		return 'O';
	case 'p':
		return 'P';
	case 's':
		return 'S';
	case 'v':
		return 'V';
	case 'w':
		return 'W';
	case 'x':
		return 'X';
	case 'z':
		return 'Z';
	default:
		return ch;
	}
}

inline const char _char_id_to_bestviewcap(const char ch)
{
    switch (ch)
	{
    case 'l':
        return '1';
    case 'O':
        return '0';
    default:
        return ch;
    }
}

inline const char* _char_id_to_chars_printable(const char ch, DWORD charProps = CHAR_NOPROPERTIES)
{
	switch (ch)
	{
	case 'a':
		return _T("a");
	case 'A':
		return _T("A");
	case 'b':
		return _T("b");
	case 'B':
		return _T("B");
	case 'c':
	case 'C':
		if ((charProps & CHAR_NOTCAPITAL) == CHAR_NOTCAPITAL)
			return _T("c");
		else if ((charProps & CHAR_TALL) == CHAR_TALL)
			return _T("C");
		else
			return _T("cC");
	case 'd':
		return _T("d");
	case 'D':
		return _T("D");
	case 'e':
		return _T("e");
	case 'E':
		return _T("E");
	case 'f':
		return _T("f");
	case 'F':
		return _T("F");
	case 'g':
		return _T("g");
	case 'G':
		return _T("G");
	case 'h':
		return _T("h");
	case 'H':
		return _T("H");
	case 'i':
	case 'I':
	case 'l':
	case '1':
		if ((charProps & CHAR_NOTCAPITAL) == CHAR_NOTCAPITAL)
			return _T("i");
		else if ((charProps & (CHAR_CAPITAL | CHAR_TALL)) == CHAR_TALL)
			return _T("l");
		else if ((charProps & CHAR_CAPITAL) == CHAR_CAPITAL)
			return _T("I1");
		else
			return _T("iIl1");
	case 'j':
	case 'J':
		if ((charProps & CHAR_NOTCAPITAL) == CHAR_NOTCAPITAL)
			return _T("j");
		else if ((charProps & CHAR_TALL) == CHAR_TALL)
			return _T("J");
		else
			return _T("jJ");
	case 'k':
		return _T("k");
	case 'K':
		return _T("K");
	case 'L':
		return _T("L");
	case 'm':
		return _T("m");
	case 'M':
		return _T("M");
	case 'n':
		return _T("n");
	case 'N':
		return _T("N");
	case 'o':
	case 'O':
	case '0':
		if ((charProps & CHAR_NOTCAPITAL) == CHAR_NOTCAPITAL)
			return _T("o");
		else if ((charProps & CHAR_TALL) == CHAR_TALL)
			return _T("O0");
		else
			return _T("oO0");
	case 'p':
	case 'P':
		if ((charProps & CHAR_NOTCAPITAL) == CHAR_NOTCAPITAL)
			return _T("p");
		else if ((charProps & CHAR_TALL) == CHAR_TALL)
			return _T("P");
		else
			return _T("pP");
	case 'q':
		return _T("q");
	case 'Q':
		return _T("Q");
	case 'r':
		return _T("r");
	case 'R':
		return _T("R");
	case 's':
	case 'S':
		if ((charProps & CHAR_NOTCAPITAL) == CHAR_NOTCAPITAL)
			return _T("s");
		else if ((charProps & CHAR_TALL) == CHAR_TALL)
			return _T("S");
		else
			return _T("sS");
	case 't':
		return _T("t");
	case 'T':
		return _T("T");
	case 'u':
		return _T("u");
	case 'U':
		return _T("U");
	case 'v':
	case 'V':
		if ((charProps & CHAR_NOTCAPITAL) == CHAR_NOTCAPITAL)
			return _T("v");
		else if ((charProps & CHAR_TALL) == CHAR_TALL)
			return _T("V");
		else
			return _T("vV");
	case 'w':
	case 'W':
		if ((charProps & CHAR_NOTCAPITAL) == CHAR_NOTCAPITAL)
			return _T("w");
		else if ((charProps & CHAR_TALL) == CHAR_TALL)
			return _T("W");
		else
			return _T("wW");
	case 'x':
	case 'X':
		if ((charProps & CHAR_NOTCAPITAL) == CHAR_NOTCAPITAL)
			return _T("x");
		else if ((charProps & CHAR_TALL) == CHAR_TALL)
			return _T("X");
		else
			return _T("xX");
	case 'y':
		return _T("y");
	case 'Y':
		return _T("Y");
	case 'z':
	case 'Z':
		if ((charProps & CHAR_NOTCAPITAL) == CHAR_NOTCAPITAL)
			return _T("z");
		else if ((charProps & CHAR_TALL) == CHAR_TALL)
			return _T("Z");
		else
			return _T("zZ");
	case '2':
		return _T("2");
	case '3':
		return _T("3");
	case '4':
		return _T("4");
	case '5':
		return _T("5");
	case '6':
		return _T("6");
	case '7':
		return _T("7");
	case '8':
		return _T("8");
	case '9':
		return _T("9");
	default:
		return _T("?");
	}
}

inline bool _char_is_always_capital(const char c)
{
	const char alwaysCapital[] = "ABDEFGHKLMNQRTUY23456789";
	if (!strchr(alwaysCapital, c))
		return false;
	return true;
}

inline bool _char_is_always_small(const char c)
{
	const char alwaysSmall[] = "abdefghkmnqrtuy";
	if (!strchr(alwaysSmall, c))
		return false;
	return true;
}

#endif