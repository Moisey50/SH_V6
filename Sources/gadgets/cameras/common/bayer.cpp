inline unsigned char _makeYBayer(DWORD R, DWORD G2, DWORD B)
{
    return((unsigned char)((9765*R+9585*G2+3801*B)>>15));
}

inline int BGGR(LPBYTE off, int w, DWORD& R, DWORD& G2, DWORD& B)
{
    B=*(off); R=*(off+w+1); G2=*(off+1)+*(off+w);
    return _makeYBayer(R,G2,B);
}

inline int GBRG(LPBYTE off, int w, DWORD& R, DWORD& G2, DWORD& B)
{
    B=*(off+1); R=*(off+w); G2=*(off)+*(off+w+1);
    return _makeYBayer(R,G2,B);
}

inline int GRBG(LPBYTE off, int w, DWORD& R, DWORD& G2, DWORD& B)
{
    B=*(off+w); R=*(off+1); G2=*(off)+*(off+w+1);
    return _makeYBayer(R,G2,B);
}

inline int RGGB(LPBYTE off, int w, DWORD& R, DWORD& G2, DWORD& B)
{
    B=*(off+w+1); R=*(off); G2=*(off+1)+*(off+w);
    return _makeYBayer(R,G2,B);
}

inline BYTE _makeUBayer(int rS, int gS, int bS)
{
    return (unsigned char)(((-5669*(int)rS-5571*gS+16777*bS)>>19)+128);
}

inline BYTE _makeVBayer(int rS, int gS, int bS)
{
    return (unsigned char)(((16383*rS-6832*gS-2720*bS)>>19)+128);
}

inline void CopyRaw8(LPBYTE srcO, LPBYTE dstO, int width, int height)
{
	LPBYTE org=srcO, dst=dstO;
    
    int w9=(width/4)*4,h9=(height/4)*4;
    int w=width, h=height;

	int eod=(w9*h9)/16;

    LPBYTE offU=dstO+w9*h9;
    LPBYTE offV=offU+eod;
    
    memset(dstO,128,(9*w9*h9/8));
    
    int i=0;
    DWORD R,G,B;
    DWORD rS,gS,bS;
#define SUM() rS+=R; gS+=G; bS+=B
	while (i<eod)
	{
        rS=gS=bS=0;
		*dst = BGGR(org,w,R,G,B); SUM(); *(dst+w9) =GRBG(org+w,w,R,G,B); SUM(); *(dst+2*w9) =BGGR(org+2*w,w,R,G,B); SUM(); *(dst+3*w9) =GRBG(org+3*w,w,R,G,B); SUM(); dst++; org++;
		*dst = GBRG(org,w,R,G,B); SUM(); *(dst+w9) =RGGB(org+w,w,R,G,B); SUM(); *(dst+2*w9) =GBRG(org+2*w,w,R,G,B); SUM(); *(dst+3*w9) =RGGB(org+3*w,w,R,G,B); SUM(); dst++; org++; 
		*dst = BGGR(org,w,R,G,B); SUM(); *(dst+w9) =GRBG(org+w,w,R,G,B); SUM(); *(dst+2*w9) =BGGR(org+2*w,w,R,G,B); SUM(); *(dst+3*w9) =GRBG(org+3*w,w,R,G,B); SUM(); dst++; org++;
		*dst = GBRG(org,w,R,G,B); SUM(); *(dst+w9) =RGGB(org+w,w,R,G,B); SUM(); *(dst+2*w9) =GBRG(org+2*w,w,R,G,B); SUM(); *(dst+3*w9) =RGGB(org+3*w,w,R,G,B); SUM(); dst++; org++;

        *offU= _makeUBayer(rS,gS,bS); offU++; 
        *offV= _makeVBayer(rS,gS,bS); offV++;

		i++;
        if ((i%(w9/4))==0)
        {
            org+=(w%4);
            dst+=3*w9;
            org+=(3*w);
        }
    }
}
