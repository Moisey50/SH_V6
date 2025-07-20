#include "StdAfx.h"
#include <gadgets/gadbase.h>
#include <video\TVFrame.h>
#include <video\shvideo.h>
#include <gadgets/videoframe.h>
#include "ColorMapGadget.h"

IMPLEMENT_RUNTIME_GADGET_EX(ColorMap, CFilterGadget, "Video.math", TVDB400_PLUGIN_NAME);

#define OFFSET_DEF1 30
#define OFFSET_DEF2 10

#define COLOR_R 50
#define COLOR_RG 80
#define COLOR_G 110
#define COLOR_GB 140
#define COLOR_B 170
#define COLOR_BR 200

__forceinline void _makeRGB15(LPBYTE pntr, DWORD& R, DWORD& G, DWORD& B)
{
    unsigned short dw=*((unsigned short*)pntr);
    B=((dw&0x1F)<<3); dw=(dw>>5);
    G=((dw&0x1F)<<3); dw=(dw>>5);
    R=((dw&0x1F)<<3);
}

__forceinline void _makeRGB8(LPBYTE pntr, RGBQUAD * Palette, DWORD& R, DWORD& G, DWORD& B)
{
    R=Palette[*(pntr)].rgbRed;
    G=Palette[*(pntr)].rgbGreen;
    B=Palette[*(pntr)].rgbBlue; 
}

__forceinline void _makeRGB(LPBYTE pntr, DWORD& R, DWORD& G, DWORD& B)
{
    B=*(pntr);
    G=*(pntr+1);
    R=*(pntr+2);
}



__forceinline BYTE getMainColor(DWORD R, DWORD G, DWORD B, DWORD OFFSET_1, DWORD OFFSET_2)
{
    if( R >= G && R >= B)
        //red first
    {
        if(R - max(G, B) < OFFSET_2)
            //mixed color (or black)
            if( labs(G - B) > (long) OFFSET_1 )
                return G >= B ? COLOR_RG : COLOR_BR;
            else
                return 0;
        else
            if(R - max(G, B) > OFFSET_1)
                //pure color (or black)
                return COLOR_R;
            else
                return 0;
    }
    else 
        if (G >= B)
            //green first
        {
            if(G - max(R, B) < OFFSET_2)
                //mixed color (or black)
                if(labs(R - B) > (long) OFFSET_1)
                    return R >= B ? COLOR_RG : COLOR_GB;
                else
                    return 0;
            if(G - max(R, B) > OFFSET_1)
                //pure color (or black)
                return COLOR_G;
            else
                return 0;
        }
        //blue first
        else
        {
            if(B - max(R, G) < OFFSET_2)
                //mixed color (or black)
                if(labs(R - G) > (long) OFFSET_1)
                    return R >= G ? COLOR_BR : COLOR_GB;
                else
                    return 0;
            if(B - max(R, G) > OFFSET_1)
                //pure color (or black)
                return COLOR_B;
            else
                return 0;
        }
}


#define COLOR_PU_B 50
#define COLOR_PU_MV_CYAN 75
#define COLOR_MV_GC 100
#define COLOR_MU_MV_GREEN 125
#define COLOR_MU_YELLOW 150
#define COLOR_MU_PV_RED 175
#define COLOR_PV_MAG 200
#define COLOR_PV_PU_MAG 225

__forceinline BYTE getMainColor(char U, char V, char OFFSET_1, char OFFSET_2)
{
    if( U > OFFSET_1 )
    {
        if ( V > OFFSET_2)
            return COLOR_PV_PU_MAG ;
        if ( V < -OFFSET_2 )
            return COLOR_PU_MV_CYAN ;
        return COLOR_PU_B ;
    }
    if ( U < -OFFSET_1 )
    {
        if ( V > OFFSET_2)
            return COLOR_MU_PV_RED ;
        if ( V < -OFFSET_2 )
            return COLOR_MU_MV_GREEN ;
        return COLOR_MU_YELLOW ;
    }
    if ( V > OFFSET_2 )
        return COLOR_PV_MAG ;
    if ( V < -OFFSET_2 )
        return COLOR_MV_GC ;
    return 0 ;
}

__forceinline void mapcolors15(pTVFrame src, pTVFrame dst, DWORD OFFSET_1, DWORD OFFSET_2)
{
    DWORD R, G, B;
    int iSize = src->lpBMIH->biWidth * src->lpBMIH->biHeight * 2;
    LPBYTE lpDataSrc=(src->lpData)?src->lpData:((LPBYTE)src->lpBMIH)+src->lpBMIH->biSize;
    LPBYTE lpDatadst=(dst->lpData)?dst->lpData:((LPBYTE)dst->lpBMIH)+dst->lpBMIH->biSize;
    for(int i = 0, j = 0; i < iSize; i += 2,j += 3)
    {
        _makeRGB15(lpDataSrc+i, R, G, B);
        lpDatadst[j] = lpDatadst[j+1] = lpDatadst[j+2] = getMainColor(R, G, B, OFFSET_1, OFFSET_2);
    }
}
__forceinline void mapcolors24(pTVFrame src, pTVFrame dst, DWORD OFFSET_1, DWORD OFFSET_2)
{
    DWORD R, G, B;
    LPBYTE lpDataSrc=(src->lpData)?src->lpData:((LPBYTE)src->lpBMIH)+src->lpBMIH->biSize;
    LPBYTE lpDatadst=(dst->lpData)?dst->lpData:((LPBYTE)dst->lpBMIH)+dst->lpBMIH->biSize;

    int iSize = src->lpBMIH->biWidth * src->lpBMIH->biHeight * 3;
    for(int i = 0, j = 0; i < iSize; i += 3,j += 3)
    {
        _makeRGB(lpDataSrc+i, R, G, B);
        lpDatadst[j] = lpDatadst[j+1] = lpDatadst[j+2] = getMainColor(R, G, B, OFFSET_1, OFFSET_2);
    }
}

__forceinline void mapcolors8(pTVFrame src, pTVFrame dst, DWORD OFFSET_1, DWORD OFFSET_2)
{
    DWORD R, G, B;
    int iSize = src->lpBMIH->biWidth * src->lpBMIH->biHeight;
    LPBYTE lpDataSrc=(src->lpData)?src->lpData:((LPBYTE)src->lpBMIH)+src->lpBMIH->biSize;
    LPBYTE lpDatadst=(dst->lpData)?dst->lpData:((LPBYTE)dst->lpBMIH)+dst->lpBMIH->biSize;
    RGBQUAD* Palette=(RGBQUAD*)((LPBYTE)(src->lpBMIH)+src->lpBMIH->biSize);
    for(int i = 0, j = 0; i < iSize; i += 1,j += 3)
    {
        _makeRGB8(lpDataSrc+i, Palette, R, G, B);
        lpDatadst[j] = lpDatadst[j+1] = lpDatadst[j+2] = getMainColor(R, G, B, OFFSET_1, OFFSET_2);
    }
}

__forceinline void mapcolors32(pTVFrame src, pTVFrame dst, DWORD OFFSET_1, DWORD OFFSET_2)
{
    DWORD R, G, B;
    int iSize = src->lpBMIH->biWidth * src->lpBMIH->biHeight * 4;
    LPBYTE lpDataSrc=(src->lpData)?src->lpData:((LPBYTE)src->lpBMIH)+src->lpBMIH->biSize;
    LPBYTE lpDatadst=(dst->lpData)?dst->lpData:((LPBYTE)dst->lpBMIH)+dst->lpBMIH->biSize;
    for(int i = 0, j = 0; i < iSize; i += 4,j += 3)
    {
        _makeRGB(lpDataSrc+i, R, G, B);
        lpDatadst[j] = lpDatadst[j+1] = lpDatadst[j+2] = getMainColor(R, G, B, OFFSET_1, OFFSET_2);
    }
}

ColorMap::ColorMap(void)
{
    m_pInput = new CInputConnector(vframe);
    m_pOutput = new COutputConnector(vframe);
    m_iOffsetPure = OFFSET_DEF1;
    m_iOffsetMixed = OFFSET_DEF2;
    Resume();
}

void ColorMap::ShutDown(void)
{
    CFilterGadget::ShutDown();
    delete m_pInput;
    m_pInput = NULL;
    delete m_pOutput;
    m_pOutput = NULL;
}


CDataFrame* ColorMap::DoProcessing(const CDataFrame* pDataFrame)
{
    const CVideoFrame* vf1 = pDataFrame->GetVideoFrame(DEFAULT_LABEL);
    if (!vf1)
        return NULL;
    LPBYTE bm1 = GetData(vf1);
    int iYSize = GetWidth(vf1) * GetHeight(vf1) ;
    if ( !bm1  ||  !iYSize )
        return NULL ;

    char * pV = (char*)(bm1 + iYSize) ;

    CVideoFrame* vf = CVideoFrame::Create();
    CVideoFrame* vfTemp = NULL ;

    if ( vf1->lpBMIH->biCompression == BI_YUV9 
        || vf1->lpBMIH->biCompression == BI_YUV12
        || vf1->lpBMIH->biCompression == BI_YUV411 )
    {
        vf->lpBMIH=(LPBITMAPINFOHEADER)malloc(sizeof(BITMAPINFOHEADER)+ iYSize);
        memcpy(vf->lpBMIH, vf1->lpBMIH, sizeof(BITMAPINFOHEADER));
        vf->lpData = NULL ;
        vf->lpBMIH->biBitCount = 8;
        vf->lpBMIH->biCompression = BI_Y8;
        vf->lpBMIH->biSizeImage = iYSize ;
    }
    else
    {
        vfTemp = CVideoFrame::Create();
        vfTemp->lpBMIH=(LPBITMAPINFOHEADER)malloc(sizeof(BITMAPINFOHEADER) + vf1->lpBMIH->biSizeImage);
        if(vf1->lpData)
        {
            memcpy( vfTemp->lpBMIH, vf1->lpBMIH, sizeof(BITMAPINFOHEADER));
            memcpy( vfTemp->lpBMIH + 1 , vf1->lpData, vf1->lpBMIH->biSizeImage);
        }
        else
        {
            memcpy(vfTemp->lpBMIH, vf1->lpBMIH, sizeof(BITMAPINFOHEADER) + vf1->lpBMIH->biSizeImage);
        }
        vf->lpBMIH=(LPBITMAPINFOHEADER)malloc(sizeof(BITMAPINFOHEADER)+ ((vfTemp->lpBMIH->biSizeImage / 2) * 3));
        memcpy(vf->lpBMIH, vf1->lpBMIH, sizeof(BITMAPINFOHEADER));
        vf->lpBMIH->biBitCount = 24;
        vf->lpBMIH->biCompression = BI_RGB;
        vf->lpBMIH->biSizeImage = ((vfTemp->lpBMIH->biSizeImage / 2) * 3);
    }

    switch( vf1->lpBMIH->biCompression )
    {

    case BI_YUV9:
        {
            int iVSize = iYSize/16 ;
            char * pU = pV + iVSize ;
            char *pDest = (char*) GetData( vf ) ;
            char *pDestStep = pDest ;
            for ( DWORD iY = 0 ; iY < GetHeight(vf1 ) ; iY += 4 )
            {
                for ( DWORD iX = 0 ; iX < GetWidth(vf1) ; iX += 4 )
                {
                    BYTE iBr = getMainColor( *(pU++) , *(pV++) , (char) m_iOffsetMixed , (char)m_iOffsetPure ) ;
                    *(pDest++) = (char)iBr ;
                    *(pDest++) = (char)iBr ;
                    *(pDest++) = (char)iBr ;
                    *(pDest++) = (char)iBr ;
                }
                char * pSrc = pDestStep ;
                memcpy( pDestStep += GetWidth(vf1) , pSrc , GetWidth(vf1) ) ;
                memcpy( pDestStep += GetWidth(vf1) , pSrc , GetWidth(vf1) ) ;
                memcpy( pDestStep += GetWidth(vf1) , pSrc , GetWidth(vf1) ) ;
                pDest = (pDestStep += GetWidth(vf1)) ;
            }
        }
        break ;
    case BI_YUV12:
        {
            int iVSize = iYSize/4 ;
            char * pU = pV + iVSize ;
            char *pDest = (char*)GetData( vf ) ;
            char *pDestStep = pDest ;
            for ( DWORD iY = 0 ; iY < GetHeight(vf1) ; iY += 2 )
            {
                for ( DWORD iX = 0 ; iX < GetWidth(vf1) ; iX += 2 )
                {
                    BYTE iBr = getMainColor( *(pU++) , *(pV++) , (char) m_iOffsetMixed , (char)m_iOffsetPure ) ;
                    *(pDest++) = (char)iBr ;
                    *(pDest++) = (char)iBr ;
                }
                char * pSrc = pDestStep ;
                memcpy( pDestStep += GetWidth(vf1) , pSrc , GetWidth(vf1) ) ;
                pDest = (pDestStep += GetWidth(vf1)) ;
            }
        }
        break ;
    case BI_YUV411:
        {
            char *pDest = (char*)GetData( vf ) ;
            char * pYUV411 = (char*)bm1 ;
            for ( DWORD iY = 0 ; iY < GetHeight(vf1) ; iY++ )
            {
                for ( DWORD iX = 0 ; iX < GetWidth(vf1) ; iX += 4 )
                {
                    char cU = *pYUV411 ;
                    pYUV411 += 3 ;
                    char cV = *pYUV411 ;
                    pYUV411 += 3 ;

                    BYTE iBr = getMainColor( cU , cV , (char) m_iOffsetMixed , (char)m_iOffsetPure ) ;
                    *(pDest++) = (char)iBr ;
                    *(pDest++) = (char)iBr ;
                    *(pDest++) = (char)iBr ;
                    *(pDest++) = (char)iBr ;
                }
            }
        }
        break ;
    default:
        _decompress_any(vfTemp);
    case BI_RGB: 
        {
            LPBYTE bmTemp = GetData(vfTemp);
            LPBYTE bm = GetData(vf);
            int iSize = vfTemp->lpBMIH->biWidth * vfTemp->lpBMIH->biHeight * 2;
            if (vfTemp->lpBMIH->biBitCount==16) 
                mapcolors15(vfTemp, vf, m_iOffsetPure, m_iOffsetMixed);
            else 
                if (vfTemp->lpBMIH->biBitCount==24) 
                    mapcolors24(vfTemp, vf, m_iOffsetPure, m_iOffsetMixed);
                else 
                    if (vfTemp->lpBMIH->biBitCount==8) 
                        mapcolors8(vfTemp, vf, m_iOffsetPure, m_iOffsetMixed);
                    else 
                        if (vfTemp->lpBMIH->biBitCount==32) 
                            mapcolors32(vfTemp, vf, m_iOffsetPure, m_iOffsetMixed);
                        else
                        {
                            vfTemp->Release(vfTemp);
                            vf->Release(vf);
                            return NULL;
                        }


                        vfTemp->RELEASE(vfTemp);
                        makeYUV9(vf);
        }
        break ;
    }
    if ( vf )
    {
        vf->CopyAttributes(pDataFrame);
    }
    return vf;
}

bool ColorMap::PrintProperties(FXString& text)
{
    FXPropertyKit pc;
    //   pc.WriteInt("OffsetPure",m_iOffsetPure);
    //   pc.WriteInt("OffsetMixed",m_iOffsetMixed);
    pc.WriteInt("U_Thres",m_iOffsetPure);
    pc.WriteInt("V_Thres",m_iOffsetMixed);
    text=pc;
    return true;
}

bool ColorMap::ScanProperties(LPCTSTR text, bool& Invalidate)
{
    FXPropertyKit pc(text);
    //   pc.WriteInt("OffsetPure",m_iOffsetPure);
    //   pc.WriteInt("OffsetMixed",m_iOffsetMixed);
    pc.GetInt("U_Thres",m_iOffsetPure);
    pc.GetInt("V_Thres",m_iOffsetMixed);
    return true;
}

bool ColorMap::ScanSettings(FXString& text)
{
    // commented version for RGB mapping
    // text.Format("template(Spin(OffsetPure,0,255),Spin(OffsetMixed,0,255))",TRUE,FALSE);
    // The below is for UV mapping
    text.Format("template(Spin(U_Thres,0,255),Spin(V_Thres,0,255))",TRUE,FALSE);
    return true;
}




