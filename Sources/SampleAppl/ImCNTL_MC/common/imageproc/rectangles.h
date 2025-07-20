#ifndef RECTANGLES_INC
#define RECTANGLES_INC

__forceinline void _correct_rect4(RECT& rc)
{
    rc.left     = (rc.left >>2)<<2;
	rc.top      = (rc.top>>2)<<2;
	rc.right    = ((rc.right + 3)>>2)<<2;
	rc.bottom   = ((rc.bottom + 3)>>2)<<2;
}

__forceinline void _rect_applyscale(pTVFrame src,pTVFrame dst, RECT& rc)
{
    ASSERT(src->lpBMIH->biXPelsPerMeter!=0);
    ASSERT(src->lpBMIH->biYPelsPerMeter!=0);
    ASSERT(dst->lpBMIH->biXPelsPerMeter!=0);
    ASSERT(dst->lpBMIH->biYPelsPerMeter!=0);
    double scaleX=((double)(src->lpBMIH->biXPelsPerMeter))/(dst->lpBMIH->biXPelsPerMeter);
    double scaleY=((double)(src->lpBMIH->biYPelsPerMeter))/(dst->lpBMIH->biYPelsPerMeter);
    rc.bottom=(int)(rc.bottom*scaleY);
    rc.top=(int)(rc.top*scaleY);
    rc.left=(int)(rc.left*scaleX);
    rc.right=(int)(rc.right*scaleX);
}

#endif