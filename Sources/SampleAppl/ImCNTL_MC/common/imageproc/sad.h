#ifndef SAD_INC
#define SAD_INC

__forceinline double _calcSad(LPBYTE src, LPBYTE mask, int mskWidth)
{
    double sum=0;
    LPBYTE eod=src+mskWidth;
    while(src<eod)
    {
        sum+=abs(*src-*mask);
        src++;
        mask++;
    }
    return sum;
}

__forceinline pTVFrame SAD(pTVFrame Frame, pTVFrame Mask)
{
    pTVFrame res=(pTVFrame)malloc(sizeof(TVFrame));
    memset(res,0,sizeof(TVFrame));
    res->lpBMIH=(LPBITMAPINFOHEADER)malloc(getsize4BMIH(Frame));
    memcpy(res->lpBMIH,Frame->lpBMIH,Frame->lpBMIH->biSize);
    LPBYTE data=GetData(res);
    LPBYTE src=GetData(Frame);
    memset(data,128,GetImageSize(Frame->lpBMIH));
    int width=Frame->lpBMIH->biWidth;
    int height=Frame->lpBMIH->biHeight;
    for (int y=0; y<height; y++)
    {
        for (int x=0; x<width; x++)
        {
            int offset=x+y*width;
            int mskWidth=((width-x)<Mask->lpBMIH->biWidth)?width-x:Mask->lpBMIH->biWidth;
            int mskHeight=((height-y)<Mask->lpBMIH->biHeight)?height-y:Mask->lpBMIH->biHeight;
            double val=0;
            int cnt=0;
            for (int i=0; i<mskHeight; i++)
            {
                val+=(_calcSad(src+offset+i*width,GetData(Mask)+i*Mask->lpBMIH->biWidth,mskWidth));
                cnt+=mskWidth;
            }
            *(data+offset)=(BYTE)(val/cnt);
        }
    }
    _negative(res);
    return res;
}

#endif

