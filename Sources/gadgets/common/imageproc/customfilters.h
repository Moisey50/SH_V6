#ifndef CUSTOM_FILTERS
#define CUSTOM_FILTERS

#include <video\TVFrame.h>

typedef enum  { type3x3=0, type5x5=1, type7x7=2 } MatrixType;

union CustomMatrix
{
    double m_3x3[3][3];
    double m_5x5[5][5];
    double m_7x7[7][7];
};

class CCustomFilterMatrix
{
public:
    MatrixType m_MatrixType;
    CustomMatrix m_Matrix;
public:
    CCustomFilterMatrix(double d00, double d01, double d02,
                        double d10, double d11, double d12,
                        double d20, double d21, double d22)
    {
        m_MatrixType=type3x3;
        m_Matrix.m_3x3[0][0]=d00; m_Matrix.m_3x3[0][1]=d01; m_Matrix.m_3x3[0][2]=d02;
        m_Matrix.m_3x3[1][0]=d10; m_Matrix.m_3x3[1][1]=d11; m_Matrix.m_3x3[1][2]=d12;
        m_Matrix.m_3x3[2][0]=d20; m_Matrix.m_3x3[2][1]=d21; m_Matrix.m_3x3[2][2]=d22;
    }
    void SetMatrix(     double d00, double d01, double d02,
                        double d10, double d11, double d12,
                        double d20, double d21, double d22)
    {
        m_MatrixType=type3x3;
        m_Matrix.m_3x3[0][0]=d00; m_Matrix.m_3x3[0][1]=d01; m_Matrix.m_3x3[0][2]=d02;
        m_Matrix.m_3x3[1][0]=d10; m_Matrix.m_3x3[1][1]=d11; m_Matrix.m_3x3[1][2]=d12;
        m_Matrix.m_3x3[2][0]=d20; m_Matrix.m_3x3[2][1]=d21; m_Matrix.m_3x3[2][2]=d22;
    }
    CCustomFilterMatrix(double d00, double d01, double d02, double d03, double d04,
                        double d10, double d11, double d12, double d13, double d14,
                        double d20, double d21, double d22, double d23, double d24,
                        double d30, double d31, double d32, double d33, double d34,
                        double d40, double d41, double d42, double d43, double d44
                       )
    {
        m_MatrixType=type5x5;
        m_Matrix.m_5x5[0][0]=d00; m_Matrix.m_5x5[0][1]=d01; m_Matrix.m_5x5[0][2]=d02; m_Matrix.m_5x5[0][3]=d03; m_Matrix.m_5x5[0][4]=d04;
        m_Matrix.m_5x5[1][0]=d10; m_Matrix.m_5x5[1][1]=d11; m_Matrix.m_5x5[1][2]=d12; m_Matrix.m_5x5[1][3]=d13; m_Matrix.m_5x5[1][4]=d14;
        m_Matrix.m_5x5[2][0]=d20; m_Matrix.m_5x5[2][1]=d21; m_Matrix.m_5x5[2][2]=d22; m_Matrix.m_5x5[2][3]=d23; m_Matrix.m_5x5[2][4]=d24;
        m_Matrix.m_5x5[3][0]=d30; m_Matrix.m_5x5[3][1]=d31; m_Matrix.m_5x5[3][2]=d22; m_Matrix.m_5x5[3][3]=d33; m_Matrix.m_5x5[3][4]=d34;
        m_Matrix.m_5x5[4][0]=d40; m_Matrix.m_5x5[4][1]=d41; m_Matrix.m_5x5[4][2]=d22; m_Matrix.m_5x5[4][3]=d43; m_Matrix.m_5x5[4][4]=d44;
    }
    void SetMatrix(     double d00, double d01, double d02, double d03, double d04,
                        double d10, double d11, double d12, double d13, double d14,
                        double d20, double d21, double d22, double d23, double d24,
                        double d30, double d31, double d32, double d33, double d34,
                        double d40, double d41, double d42, double d43, double d44
                  )
    {
        m_MatrixType=type5x5;
        m_Matrix.m_5x5[0][0]=d00; m_Matrix.m_5x5[0][1]=d01; m_Matrix.m_5x5[0][2]=d02; m_Matrix.m_5x5[0][3]=d03; m_Matrix.m_5x5[0][4]=d04;
        m_Matrix.m_5x5[1][0]=d10; m_Matrix.m_5x5[1][1]=d11; m_Matrix.m_5x5[1][2]=d12; m_Matrix.m_5x5[1][3]=d13; m_Matrix.m_5x5[1][4]=d14;
        m_Matrix.m_5x5[2][0]=d20; m_Matrix.m_5x5[2][1]=d21; m_Matrix.m_5x5[2][2]=d22; m_Matrix.m_5x5[2][3]=d23; m_Matrix.m_5x5[2][4]=d24;
        m_Matrix.m_5x5[3][0]=d30; m_Matrix.m_5x5[3][1]=d31; m_Matrix.m_5x5[3][2]=d32; m_Matrix.m_5x5[3][3]=d33; m_Matrix.m_5x5[3][4]=d34;
        m_Matrix.m_5x5[4][0]=d40; m_Matrix.m_5x5[4][1]=d41; m_Matrix.m_5x5[4][2]=d42; m_Matrix.m_5x5[4][3]=d43; m_Matrix.m_5x5[4][4]=d44;
    }
    CCustomFilterMatrix(double d00, double d01, double d02, double d03, double d04, double d05, double d06,
                        double d10, double d11, double d12, double d13, double d14, double d15, double d16,
                        double d20, double d21, double d22, double d23, double d24, double d25, double d26,
                        double d30, double d31, double d32, double d33, double d34, double d35, double d36,
                        double d40, double d41, double d42, double d43, double d44, double d45, double d46,
                        double d50, double d51, double d52, double d53, double d54, double d55, double d56,
                        double d60, double d61, double d62, double d63, double d64, double d65, double d66
                       )
    {
        m_MatrixType=type7x7;
        m_Matrix.m_7x7[0][0]=d00; m_Matrix.m_7x7[0][1]=d01; m_Matrix.m_7x7[0][2]=d02; m_Matrix.m_7x7[0][3]=d03; m_Matrix.m_7x7[0][4]=d04; m_Matrix.m_7x7[0][5]=d05; m_Matrix.m_7x7[0][6]=d06;
        m_Matrix.m_7x7[1][0]=d10; m_Matrix.m_7x7[1][1]=d11; m_Matrix.m_7x7[1][2]=d12; m_Matrix.m_7x7[1][3]=d13; m_Matrix.m_7x7[1][4]=d14; m_Matrix.m_7x7[1][5]=d15; m_Matrix.m_7x7[1][6]=d16;
        m_Matrix.m_7x7[2][0]=d20; m_Matrix.m_7x7[2][1]=d21; m_Matrix.m_7x7[2][2]=d22; m_Matrix.m_7x7[2][3]=d23; m_Matrix.m_7x7[2][4]=d24; m_Matrix.m_7x7[2][5]=d25; m_Matrix.m_7x7[2][6]=d26;
        m_Matrix.m_7x7[3][0]=d30; m_Matrix.m_7x7[3][1]=d31; m_Matrix.m_7x7[3][2]=d32; m_Matrix.m_7x7[3][3]=d33; m_Matrix.m_7x7[3][4]=d34; m_Matrix.m_7x7[3][5]=d35; m_Matrix.m_7x7[3][6]=d36;
        m_Matrix.m_7x7[4][0]=d40; m_Matrix.m_7x7[4][1]=d41; m_Matrix.m_7x7[4][2]=d42; m_Matrix.m_7x7[4][3]=d43; m_Matrix.m_7x7[4][4]=d44; m_Matrix.m_7x7[4][5]=d45; m_Matrix.m_7x7[4][6]=d46;
        m_Matrix.m_7x7[5][0]=d50; m_Matrix.m_7x7[5][1]=d51; m_Matrix.m_7x7[5][2]=d52; m_Matrix.m_7x7[5][3]=d53; m_Matrix.m_7x7[5][4]=d54; m_Matrix.m_7x7[5][5]=d55; m_Matrix.m_7x7[5][6]=d56;
        m_Matrix.m_7x7[6][0]=d60; m_Matrix.m_7x7[6][1]=d61; m_Matrix.m_7x7[6][2]=d62; m_Matrix.m_7x7[6][3]=d63; m_Matrix.m_7x7[6][4]=d64; m_Matrix.m_7x7[6][5]=d65; m_Matrix.m_7x7[6][6]=d66;
    }
    void SetMatrix(     double d00, double d01, double d02, double d03, double d04, double d05, double d06,
                        double d10, double d11, double d12, double d13, double d14, double d15, double d16,
                        double d20, double d21, double d22, double d23, double d24, double d25, double d26,
                        double d30, double d31, double d32, double d33, double d34, double d35, double d36,
                        double d40, double d41, double d42, double d43, double d44, double d45, double d46,
                        double d50, double d51, double d52, double d53, double d54, double d55, double d56,
                        double d60, double d61, double d62, double d63, double d64, double d65, double d66
                  )
    {
        m_MatrixType=type7x7;
        m_Matrix.m_7x7[0][0]=d00; m_Matrix.m_7x7[0][1]=d01; m_Matrix.m_7x7[0][2]=d02; m_Matrix.m_7x7[0][3]=d03; m_Matrix.m_7x7[0][4]=d04; m_Matrix.m_7x7[0][5]=d05; m_Matrix.m_7x7[0][6]=d06;
        m_Matrix.m_7x7[1][0]=d10; m_Matrix.m_7x7[1][1]=d11; m_Matrix.m_7x7[1][2]=d12; m_Matrix.m_7x7[1][3]=d13; m_Matrix.m_7x7[1][4]=d14; m_Matrix.m_7x7[1][5]=d15; m_Matrix.m_7x7[1][6]=d16;
        m_Matrix.m_7x7[2][0]=d20; m_Matrix.m_7x7[2][1]=d21; m_Matrix.m_7x7[2][2]=d22; m_Matrix.m_7x7[2][3]=d23; m_Matrix.m_7x7[2][4]=d24; m_Matrix.m_7x7[2][5]=d25; m_Matrix.m_7x7[2][6]=d26;
        m_Matrix.m_7x7[3][0]=d30; m_Matrix.m_7x7[3][1]=d31; m_Matrix.m_7x7[3][2]=d32; m_Matrix.m_7x7[3][3]=d33; m_Matrix.m_7x7[3][4]=d34; m_Matrix.m_7x7[3][5]=d35; m_Matrix.m_7x7[3][6]=d36;
        m_Matrix.m_7x7[4][0]=d40; m_Matrix.m_7x7[4][1]=d41; m_Matrix.m_7x7[4][2]=d42; m_Matrix.m_7x7[4][3]=d43; m_Matrix.m_7x7[4][4]=d44; m_Matrix.m_7x7[4][5]=d45; m_Matrix.m_7x7[4][6]=d46;
        m_Matrix.m_7x7[5][0]=d50; m_Matrix.m_7x7[5][1]=d51; m_Matrix.m_7x7[5][2]=d52; m_Matrix.m_7x7[5][3]=d53; m_Matrix.m_7x7[5][4]=d54; m_Matrix.m_7x7[5][5]=d55; m_Matrix.m_7x7[5][6]=d56;
        m_Matrix.m_7x7[6][0]=d60; m_Matrix.m_7x7[6][1]=d61; m_Matrix.m_7x7[6][2]=d62; m_Matrix.m_7x7[6][3]=d63; m_Matrix.m_7x7[6][4]=d64; m_Matrix.m_7x7[6][5]=d65; m_Matrix.m_7x7[6][6]=d66;
    }

    MatrixType GetMatrixType() { return m_MatrixType;}
    double Get(int i, int j)
    {
        switch (m_MatrixType)
        {
        case type3x3:
            ASSERT((i>=0) && (i<3) && (j>=0) && (j<3));
            return m_Matrix.m_3x3[i][j];
        case type5x5:
            ASSERT((i>=0) && (i<5) && (j>=0) && (j<5));
            return m_Matrix.m_5x5[i][j];
        case type7x7:
            ASSERT((i>=0) && (i<7) && (j>=0) && (j<7));
            return m_Matrix.m_7x7[i][j];
        }
        return 0;
    }
};

__forceinline BYTE customfilterGetPointValue8(const pTVFrame frame, int x, int y, CCustomFilterMatrix& fMtrx, int offset)
{
    double norma=0;
    double sum=0;
    switch (fMtrx.GetMatrixType())
    {
    case type3x3:
        {
            for (int i=-1; i<=1; i++)
            {
                if ( (y+i>=0) && (y+i<Height(frame)) )
                {
                    for (int j=-1; j<=1; j++)
                    {
                        if ( (x+j>=0) && (x+j<Width(frame)) )
                        {
                            sum+=fMtrx.Get(i+1,j+1)*GetData(frame)[x+j+(y+i)*Width(frame)];
                            norma+=fMtrx.Get(i+1,j+1);
                        }
                    }
                }
            }
            norma=(norma==0)?1:norma;
            double res=sum/norma+offset+0.5; 
            res=(res<0)?0:(res>255)?255:res;
            return (BYTE)(res);
        }
    case type5x5:
        {
            for (int i=-2; i<=2; i++)
            {
                if ( (y+i>=0) && (y+i<Height(frame)) )
                {
                    for (int j=-2; j<=2; j++)
                    {
                        if ( (x+j>=0) && (x+j<Width(frame)) )
                        {
                            sum+=fMtrx.Get(i+2,j+2)*GetData(frame)[x+j+(y+i)*Width(frame)];
                            norma+=fMtrx.Get(i+2,j+2);
                        }
                    }
                }
            }
            norma=(norma==0)?1:norma;
            double res=sum/norma+offset+0.5; 
            res=(res<0)?0:(res>255)?255:res;
            return (BYTE)(res);
        }
    case type7x7:
        {
            for (int i=-3; i<=3; i++)
            {
                if ( (y+i>=0) && (y+i<Height(frame)) )
                {
                    for (int j=-3; j<=3; j++)
                    {
                        if ( (x+j>=0) && (x+j<Width(frame)) )
                        {
                            sum+=fMtrx.Get(i+3,j+3)*GetData(frame)[x+j+(y+i)*Width(frame)];
                            norma+=fMtrx.Get(i+3,j+3);
                        }
                    }
                }
            }
            norma=(norma==0)?1:norma;
            double res=sum/norma+offset+0.5; 
            res=(res<0)?0:(res>255)?255:res;
            return (BYTE)(res);
        }
    default:
        return 0;
    }
    return 0;
}

__forceinline WORD customfilterGetPointValue16(const pTVFrame frame, int x, int y, CCustomFilterMatrix& fMtrx,int offset)
{
    double norma=0;
    double sum=0;
    switch (fMtrx.GetMatrixType())
    {
    case type3x3:
        {
            for (int i=-1; i<=1; i++)
            {
                if ( (y+i>=0) && (y+i<Height(frame)) )
                {
                    for (int j=-1; j<=1; j++)
                    {
                        if ( (x+j>=0) && (x+j<Width(frame)) )
                        {
                            sum+=fMtrx.Get(i+1,j+1)*GetData16(frame)[x+j+(y+i)*Width(frame)];
                            norma+=fMtrx.Get(i+1,j+1);
                        }
                    }
                }
            }
            norma=(norma==0)?1:norma;
            double res=sum/norma+offset+0.5; 
            res=(res<0)?0:(res>65535)?65535:res;
            return (WORD)(res);
        }
    case type5x5:
        {
            for (int i=-2; i<=2; i++)
            {
                if ( (y+i>=0) && (y+i<Height(frame)) )
                {
                    for (int j=-2; j<=2; j++)
                    {
                        if ( (x+j>=0) && (x+j<Width(frame)) )
                        {
                            sum+=fMtrx.Get(i+2,j+2)*GetData16(frame)[x+j+(y+i)*Width(frame)];
                            norma+=fMtrx.Get(i+2,j+2);
                        }
                    }
                }
            }
            norma=(norma==0)?1:norma;
            double res=sum/norma+offset+0.5; 
            res=(res<0)?0:(res>65535)?65535:res;
            return (WORD)(res);
        }
    case type7x7:
        {
            for (int i=-3; i<=3; i++)
            {
                if ( (y+i>=0) && (y+i<Height(frame)) )
                {
                    for (int j=-3; j<=3; j++)
                    {
                        if ( (x+j>=0) && (x+j<Width(frame)) )
                        {
                            sum+=fMtrx.Get(i+3,j+3)*GetData16(frame)[x+j+(y+i)*Width(frame)];
                            norma+=fMtrx.Get(i+3,j+3);
                        }
                    }
                }
            }
            norma=(norma==0)?1:norma;
            double res=sum/norma+offset+0.5; 
            res=(res<0)?0:(res>65535)?65535:res;
            return (WORD)(res);
        }
    default:
        return 0;
    }
    return 0;
}

__forceinline pTVFrame _customfilter(const pTVFrame frame, CCustomFilterMatrix& fMtrx, int offset=0)
{
    pTVFrame retV=(pTVFrame)malloc(sizeof(TVFrame)); 
    memset(retV,0,sizeof(TVFrame));
    retV->lpBMIH=(LPBITMAPINFOHEADER)malloc(getsize4BMIH(frame));
    memcpy(retV->lpBMIH,frame->lpBMIH,frame->lpBMIH->biSize);
    int width=GetWidth(frame);
    int height=GetHeight(frame);
    for (unsigned y=0; y<GetHeight(retV); y++)
    {
        for (unsigned x=0; x<GetWidth(retV); x++)
        {
            switch (retV->lpBMIH->biCompression)
            {
            case BI_YUV12:
                memcpy(GetData(retV)+width*height,GetData(frame)+width*height, (width*height)/2);
            case BI_YUV9:
                memcpy(GetData(retV)+width*height,GetData(frame)+width*height, (width*height)/8);
            case BI_Y8:
                GetData(retV)[x+y*width]=customfilterGetPointValue8(frame,x,y, fMtrx, offset);
                break;
            case BI_Y16:
                GetData16(retV)[x+y*width]=customfilterGetPointValue16(frame,x,y, fMtrx, offset);
                break;
            }
        }
    }
    return retV;
}

#endif
