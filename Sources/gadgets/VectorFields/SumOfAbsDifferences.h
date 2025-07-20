#ifndef SUMOFABSDIFFERENCES_INC
#define SUMOFABSDIFFERENCES_INC

__forceinline int _abs(int i) 
{
    return (i<0)?-i:i;
}

__forceinline double _sad8x8(LPBYTE s, LPBYTE d, const DWORD width)
{
	double sum = 0;
	int nxtL = width - 8;
	for ( int x = 0; x < 8; x++ )
	{
        LPBYTE eod=s+8;
		while(s < eod)
		{
			sum += _abs((*s) - (*d));
			s++; d++;
		}
		s += nxtL;
		d += nxtL;
	}
	return sum/64;
}

__forceinline double _sad4x4(LPBYTE s, LPBYTE d, const DWORD width)
{
	double sum = 0;
	int nxtL = width - 4;
	for ( int x = 0; x < 4; x++ )
	{
        LPBYTE eod=s+4;
		while(s < eod)
		{
			sum += _abs((*s) - (*d));
			s++; d++;
		}
		s += nxtL;
		d += nxtL;
	}
	return sum/16;
}

#endif