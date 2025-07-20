//  $File : statistics.h - calculate some statistic data for the image
//  (C) Copyright The FileX Ltd 2002. 
//
//
//
//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 Firts release version, all followed changes must be listed below  (Andrey Chernushich)


#ifndef STATISTICS_INC
#define STATISTICS_INC

#include <video\tvframe.h>
#include <imageproc\simpleip.h>
#include <imageproc\imagebits.h>
#include <afxtempl.h>
#include <afxmt.h>
#include <helpers\LockObject.h>

class CSData: public CArray<double,double>
{
public:
    CLockObject m_Busy;
};

inline void GetSection(pTVFrame frame, CPoint a, CPoint b, CSData& DataBuffer)
{
   DataBuffer.m_Busy.Lock(); 
   int dx=b.x-a.x; int dy=b.y-a.y;
   int x=a.x,y=a.y; LPBYTE data;
   DataBuffer.RemoveAll();
   if (dx==0)
   {
       for (y=a.y; y!=b.y; y+=((dy<0)?-1:1))
       {
            data=__getdata_I_XY(frame,x,y);
            if (data) DataBuffer.Add(*data);
       }
   }
   else if (dy==0)
   {
       for (x=a.x; x!=b.x; x+=((dx<0)?-1:1))
       {
            data=__getdata_I_XY(frame,x,y);
            if (data) DataBuffer.Add(*data);
       }
   }
   else if (abs(dx)>abs(dy))
   {

       double r=((double)dy)/dx;
       for (x=a.x; x!=b.x; x+=((dx<0)?-1:1))
       {
            y=a.y+(int)((x-a.x)*r+0.5);
            data=__getdata_I_XY(frame,x,y);
            if (data) DataBuffer.Add(*data);
       }
   }
   else
   {
       double r=((double)dx)/dy;
       for (y=a.y; y!=b.y; y+=((dy<0)?-1:1))
       {
            x=a.x+(int)((y-a.y)*r+0.5);
            data=__getdata_I_XY(frame,x,y);
            if (data) DataBuffer.Add(*data);
       }
   }
   DataBuffer.m_Busy.Unlock();
}

inline void GetHistogram(pTVFrame frame, CSData& DataBuffer, CSData& U, CSData& V )
{
	DataBuffer.m_Busy.Lock();
    DataBuffer.RemoveAll();
    DataBuffer.SetSize(256,-1);
    U.RemoveAll();
    U.SetSize(256,-1);
    V.RemoveAll();
    V.SetSize(256,-1);

    LPBYTE s=GetData(frame);
    LPBYTE e=s+frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
    while (s<e)
    {
            DataBuffer[*s]++; s++;
    }
	s=GetData(frame)+frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
    e=s+frame->lpBMIH->biWidth*frame->lpBMIH->biHeight/16;
    while (s<e)
    {
		U[*s]++; s++;
	}
	e=s+frame->lpBMIH->biWidth*frame->lpBMIH->biHeight/16;
    while (s<e)
    {
		V[*s]++; s++;
	}
	DataBuffer.m_Busy.Unlock();
}

inline void GetHistogram(pTVFrame frame, RECT rc, CSData& DataBuffer,CSData* U=NULL, CSData* V=NULL)
{
    DataBuffer.m_Busy.Lock();
    DataBuffer.RemoveAll();
    int size=(rc.right-rc.left)*(rc.bottom-rc.top);
    if (size) 
    {
        DataBuffer.SetSize(256,-1);
        for (int i=0; i<256; i++) DataBuffer[i]=0;
    }
    if (U)
    {
        U->RemoveAll();
        if (size) 
        {
            U->SetSize(256,-1);
            for (int i=0; i<256; i++) (*U)[i]=0;
        }
    }
    if (V)
    {
        V->RemoveAll();
        if (size) 
        {
            V->SetSize(256,-1);
            for (int i=0; i<256; i++) (*V)[i]=0;
        }
    }

    for (int y=rc.top; y<rc.bottom; y++)
    {
        for (int x=rc.left; x<rc.right; x++)
        {
            if ((!U) && (!V))
            {
                DataBuffer[*__getdata_I_XY(frame,x,y)]++;
            }
            else
            {
                int _i,_u,_v;
                __getdata_IUV(frame,x, y, _i, _u, _v);
                DataBuffer[_i]++;
                if (U) (*U)[_u]++;
                if (V) (*V)[_v]++;
            }
        }
    }
    DataBuffer.m_Busy.Unlock();
}

inline void GetHistogram(pTVFrame frame, int DataBuffer[256])
{
    memset(DataBuffer,0,sizeof(int)*256);
    LPBYTE s=GetData(frame);
    LPBYTE e=s+frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
    while (s<e)
    {
            DataBuffer[*s]++; s++;
    }
}

inline void GetHistogram(pTVFrame frame, int DataBuffer[256], int &maxV,int &maxPos)
{
    memset(DataBuffer,0,sizeof(int)*256);
    maxV=0;
    LPBYTE s=GetData(frame);
    LPBYTE e=s+frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
    while (s<e)
    {
            DataBuffer[*s]++; 
            if (DataBuffer[*s]>maxV)
            {
                maxV=DataBuffer[*s]; maxPos=*s;
            }
            s++;
    }
}


#endif