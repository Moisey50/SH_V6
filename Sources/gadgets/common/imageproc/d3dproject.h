#ifndef _D3DPROJECT_INC
#define _D3DPROJECT_INC

#define NEW_PALETTE
#include <math.h>

__forceinline LPBITMAPINFOHEADER _prepare_dib24(int width, int height)
{
	DWORD size = sizeof(BITMAPINFOHEADER) + 3 * width * height;
	LPBITMAPINFOHEADER bmih = (LPBITMAPINFOHEADER)malloc(size);
  if (!bmih)
    return NULL ;
	bmih->biBitCount = 24;
	bmih->biClrImportant = 0;
	bmih->biClrUsed = 0;
	bmih->biCompression = BI_RGB;
	bmih->biHeight = height;
	bmih->biPlanes = 1;
	bmih->biSize = sizeof(BITMAPINFOHEADER);
	bmih->biSizeImage = 3 * width * height;
	bmih->biWidth = width;
	bmih->biXPelsPerMeter = 0;
	bmih->biYPelsPerMeter = 0;
	LPBYTE bits = (LPBYTE)bmih + bmih->biSize;
	memset(bits, 255, bmih->biSizeImage);
	return bmih;
}

const static BYTE rgbScale_o[][3] =
{
 { 128, 000, 128 }, { 131, 000, 125 }, { 134, 000, 122 }, { 137, 000, 119 }, { 140, 000, 116 }, { 143, 000, 113 }, { 146, 000, 110 }, { 149, 000, 107 },
 { 152, 000, 104 }, { 155, 000, 101 }, { 158, 000,  98 }, { 161, 000,  95 }, { 164, 000,  92 }, { 167, 000,  89 }, { 170, 000,  86 }, { 173, 000,  83 },
 { 176, 000,  80 }, { 179, 000,  77 }, { 182, 000,  74 }, { 185, 000,  71 }, { 188, 000,  68 }, { 191, 000,  65 }, { 194, 000,  62 }, { 197, 000,  59 },
 { 200, 000,  56 }, { 203, 000,  53 }, { 206, 000,  50 }, { 209, 000,  47 }, { 212, 000,  44 }, { 215, 000,  41 }, { 218, 000,  38 }, { 221, 000,  35 },
 { 224, 000,  32 }, { 227, 000,  29 }, { 230, 000,  26 }, { 233, 000,  23 }, { 236, 000,  20 }, { 239, 000,  17 }, { 242, 000,  14 }, { 245, 000,  11 },
 { 248, 000,   8 }, { 251, 000,   5 }, { 254, 000,   2 }, { 254,   5,   3 }, { 254,  10,   4 }, { 254,  15,   5 }, { 254,  20,   6 }, { 254,  25,   7 },
 { 254,  30,   8 }, { 254,  35,   9 }, { 254,  40,  10 }, { 254,  45,  11 }, { 254,  50,  12 }, { 254,  55,  13 }, { 254,  60,  14 }, { 254,  65,  15 },
 { 254,  70,  16 }, { 254,  75,  17 }, { 254,  80,  18 }, { 254,  85,  19 }, { 254,  90,  20 }, { 254,  95,  21 }, { 254, 100,  22 }, { 254, 105,  23 },
 { 254, 110,  24 }, { 254, 115,  25 }, { 254, 120,  26 }, { 254, 125,  27 }, { 254, 130,  28 }, { 254, 135,  29 }, { 254, 140,  30 }, { 254, 145,  31 },
 { 254, 150,  32 }, { 254, 155,  33 }, { 254, 160,  34 }, { 254, 165,  35 }, { 254, 170,  40 }, { 254, 175,  41 }, { 254, 180,  42 }, { 254, 185,  43 },
 { 254, 190,  44 }, { 254, 195,  45 }, { 254, 200,  46 }, { 254, 205,  47 }, { 254, 205,  48 }, { 252, 206,  50 }, { 250, 207,  52 }, { 248, 208,  54 },
 { 246, 209,  56 }, { 244, 210,  58 }, { 242, 211,  60 }, { 240, 212,  62 }, { 238, 213,  64 }, { 236, 214,  66 }, { 234, 215,  68 }, { 232, 216,  70 },
 { 230, 217,  72 }, { 228, 218,  74 }, { 226, 219,  76 }, { 224, 220,  78 }, { 222, 221,  80 }, { 220, 222,  82 }, { 218, 223,  84 }, { 216, 224,  86 },
 { 214, 225,  88 }, { 212, 226,  90 }, { 210, 227,  92 }, { 208, 228,  94 }, { 206, 229,  96 }, { 204, 230,  98 }, { 202, 231, 100 }, { 200, 232, 102 },
 { 198, 233, 104 }, { 196, 234, 106 }, { 194, 235, 108 }, { 192, 236, 110 }, { 190, 237, 112 }, { 188, 238, 114 }, { 186, 239, 116 }, { 184, 240, 118 },
 { 182, 241, 120 }, { 180, 242, 122 }, { 178, 243, 124 }, { 176, 244, 126 }, { 174, 245, 128 }, { 172, 246, 130 }, { 170, 247, 132 }, { 168, 248, 134 },
 { 166, 249, 136 }, { 164, 250, 138 }, { 162, 251, 140 }, { 160, 252, 142 }, { 158, 253, 144 }, { 156, 254, 148 }, { 154, 255, 150 }, { 152, 255, 152 },
 { 150, 255, 154 }, { 148, 255, 156 }, { 146, 255, 158 }, { 144, 255, 160 }, { 142, 255, 162 }, { 140, 255, 164 }, { 138, 255, 166 }, { 136, 255, 168 },
 { 134, 255, 170 }, { 132, 255, 172 }, { 130, 255, 174 }, { 128, 255, 176 }, { 126, 255, 178 }, { 124, 255, 180 }, { 122, 255, 182 }, { 120, 255, 184 },
 { 118, 255, 186 }, { 116, 255, 188 }, { 114, 255, 190 }, { 112, 255, 192 }, { 110, 255, 194 }, { 108, 255, 196 }, { 106, 255, 198 }, { 104, 255, 200 },
 { 102, 255, 202 }, { 100, 255, 204 }, {  98, 255, 206 }, {  96, 255, 208 }, {  94, 255, 210 }, {  92, 255, 212 }, {  90, 255, 214 }, {  88, 255, 216 },
 {  86, 255, 218 }, {  84, 255, 220 }, {  82, 255, 222 }, {  80, 255, 224 }, {  78, 255, 226 }, {  76, 255, 228 }, {  74, 255, 230 }, {  72, 255, 232 },
 {  70, 255, 234 }, {  68, 255, 236 }, {  66, 255, 238 }, {  64, 255, 240 }, {  62, 255, 242 }, {  60, 255, 244 }, {  58, 255, 246 }, {  56, 255, 248 },
 {  54, 255, 250 }, {  52, 255, 252 }, {  50, 255, 254 }, {  53, 254, 255 }, {  56, 253, 255 }, {  59, 252, 255 }, {  62, 251, 255 }, {  65, 250, 255 },
 {  68, 249, 255 }, {  71, 248, 255 }, {  74, 247, 255 }, {  77, 246, 255 }, {  80, 245, 255 }, {  83, 244, 255 }, {  86, 243, 255 }, {  89, 242, 255 },
 {  92, 241, 255 }, {  95, 240, 255 }, {  98, 239, 255 }, { 101, 238, 255 }, { 104, 237, 255 }, { 107, 236, 255 }, { 110, 235, 255 }, { 113, 234, 255 },
 { 116, 233, 255 }, { 119, 232, 255 }, { 122, 231, 255 }, { 125, 230, 255 }, { 128, 229, 255 }, { 131, 228, 255 }, { 134, 227, 255 }, { 137, 226, 255 },
 { 140, 225, 255 }, { 143, 224, 255 }, { 146, 223, 255 }, { 149, 222, 255 }, { 152, 221, 255 }, { 155, 220, 255 }, { 158, 219, 255 }, { 161, 218, 255 },
 { 164, 217, 255 }, { 167, 216, 255 }, { 170, 215, 255 }, { 173, 214, 255 }, { 176, 213, 255 }, { 179, 212, 255 }, { 182, 211, 255 }, { 185, 210, 255 },
 { 188, 209, 255 }, { 191, 208, 255 }, { 194, 207, 255 }, { 197, 206, 255 }, { 200, 205, 255 }, { 203, 204, 255 }, { 205, 206, 254 }, { 207, 208, 253 },
 { 209, 210, 252 }, { 211, 212, 251 }, { 213, 214, 250 }, { 215, 216, 249 }, { 217, 218, 248 }, { 219, 220, 247 }, { 221, 222, 246 }, { 223, 224, 245 },
 { 225, 226, 244 }, { 227, 228, 243 }, { 229, 230, 242 }, { 231, 232, 241 }, { 233, 234, 240 }, { 235, 236, 239 }, { 237, 238, 238 }, { 238, 240, 237 },
};

BYTE rgbScale_m[256][3];

#ifdef NEW_PALETTE
    #define rgbScale rgbScale_m
#else
    #define rgbScale rgbScale_o
#endif

__forceinline void _prepare_mpalette()
{
    for (int i=0; i<256; i++)
    {
      int g = ( i < 128 ) ? i * 2 : ( 255 - i ) * 2 ;    //255-(i-128)*(i-128)/64;
        g=(g<0)?0:g;
        rgbScale_m[i][0]=255-i;
        rgbScale_m[i][1]=(BYTE)g;
        rgbScale_m[i][2]=i;
    }
}

__forceinline double _d3dangle(double A, double viewTg)
{
	const double pi = acos(-1.);
	double a = atan(tan(A) * viewTg);
	if (cos(A) < 0)
		a += pi;
	return a;
}

__forceinline void* _d3dproject(LPBYTE Src, int width, int height, int alpha, 
  int beta, int grid = 30, bool grayscale = true , int iLow = 0 , int iHigh = 255 )
{
	LPBITMAPINFOHEADER bmih = _prepare_dib24(width, height);
  if (!bmih)
    return NULL ;

  LPBYTE Dst = (LPBYTE)bmih + bmih->biSize;

	const double pi = acos(-1.);
	double A = ((double)alpha / 180.) * pi;
	double B = ((double)beta / 180.) * pi;

/*	const double viewTg = 1;
	if (alpha != 90 && alpha != 270)
		A = _d3dangle(A, viewTg);
	if (beta != 90 && beta != 270)
		B = _d3dangle(B, viewTg);*/
	double cosA = cos(A);
	double cosB = cos(B);
	double sinA = sin(A);
	double sinB = sin(B);
	double xMin = 0, xMax = 0, yMin = 0, yMax = 0;
	double vertX[3] = { -(double)width * cosA, (double)height * cosB, -(double)width * cosA + (double)height * cosB };
	double vertY[3] = { -(double)width * sinA, -(double)height * sinB, -(double)width * sinA - (double)height * sinB };
	int jSt = height - 1, jEn = 0, jStep = -1;
	int iSt = width - 1, iEn = 0, iStep = -1;
	bool xPriority = (vertY[0] < vertY[1]);
	int iSts[3] = { 0, width - 1, 0 };
	int iEns[3] = { width - 1, 0, width - 1 };
	int iSteps[3] = { 1, -1, 1 };
	int jSts[3] = { height - 1, 0, 0 };
	int jEns[3] = { 0, height - 1, height - 1 };
	int jSteps[3] = { -1, 1, 1 };
	bool xPriorities[3] = { (vertY[2] > 0), (vertY[2] < 0), (vertY[1] < vertY[0]) };

	int i, j;
	for (i = 0; i < 3; i++)
	{
		if (xMin > vertX[i])
			xMin = vertX[i];
		else if (xMax < vertX[i])
			xMax = vertX[i];
		if (yMin > vertY[i])
		{
			yMin = vertY[i];
			iSt = iSts[i];
			iEn = iEns[i];
			iStep = iSteps[i];
			jSt = jSts[i];
			jEn = jEns[i];
			jStep = jSteps[i];
			xPriority = xPriorities[i];
		}
		else if (yMax < vertY[i])
			yMax = vertY[i];
	}

	double kx = (double)(width - 1) / (xMax - xMin);
	double ky = (double)(height - 1) / (yMax - yMin + 255.);
	double k = (kx < ky) ? kx : ky;
	int x0 = (int)((double)(width - 1) * xMax / (xMax - xMin));
	int y0 = (int)((double)(height - 1) * yMax / (yMax - yMin + 255.));

	int jVirtStep = (xPriority) ? 0 : jStep;
	int iVirtStep = (xPriority) ? iStep : 0;

	for (j = jSt, i = iSt; ; j += jVirtStep, i += iVirtStep)
	{
		if (i == iEn)
		{
			i = iSt;
			j += jStep;
			if (j == jEn)
				break;
		}
		if (j == jEn)
		{
			j = jSt;
			i += iStep;
			if (i == iEn)
				break;
		}
		{
			int x = x0 + (int)((double)i * cosA * kx - (double)j * cosB * kx);
			ASSERT(x >= 0 && x < width);
			int yb = y0 + (int)((double)i * sinA * ky + (double)j * sinB * ky);
			ASSERT(yb >= 0 && yb < height);
			double val = (double)Src[(height - j - 1) * width + i];
			int yt = yb + (int)(val * ky);
			ASSERT(yt >= 0 && yt < height);
			int off = yt * 3 * width + x * 3;
			int end = yb * 3 * width + x * 3;
			BYTE cR, cG, cB;

// 			if (grayscale)
// 			{
// 				cR = (BYTE)val; cG = (BYTE)val; cB = (BYTE)val;
// 			}
// 			else
// 			{
// 				cR = rgbScale[(BYTE)val][2]; cG = rgbScale[(BYTE)val][1]; cB = rgbScale[(BYTE)val][0];
// 			}
			bool bGrid = false;
			if (grid && ((i % grid) * (j % grid) == 0))
			{
				cB = cG = cR = 0;
				bGrid = true;
			}
			double dval = (yt == yb) ? 0 : (val - 1) / (double)(yt - yb);
			while (off >= end)
			{
				if ((Dst[off] != 255) || (Dst[off + 1] != 255) || (Dst[off + 2] != 255))
				{
					if (bGrid)
					{
						Dst[off] = (BYTE)(Dst[off] * 0.75);
						Dst[off + 1] = (BYTE)(Dst[off + 1] * 0.75);
						Dst[off + 2] = (BYTE)(Dst[off + 2] * 0.75);
					}
					break;
				}
        if ( iLow <= val && val <= iHigh )
        {
          if ( grayscale )
          {
            cR = ( BYTE )val; cG = ( BYTE )val; cB = ( BYTE )val;
          }
          else
          {
            cR = rgbScale[ ( BYTE )val ][ 2 ]; cG = rgbScale[ ( BYTE )val ][ 1 ]; cB = rgbScale[ ( BYTE )val ][ 0 ];
          }
        }
        else
          cR = cG = cB = 255 ;
				Dst[off] = cB;
				Dst[off + 1] = cG;
				Dst[off + 2] = cR;
        
				val -= dval;
				off -= 3 * width;
			}
		}
	}
	return bmih;
}



__forceinline pTVFrame _d3dproject(pTVFrame frame, int alpha, int beta)
{
	const double pi = acos(-1.);
	int width = frame->lpBMIH->biWidth;
	int height = frame->lpBMIH->biHeight;
	double A = ((double)alpha / 180.) * pi;
	double B = ((double)beta / 180.) * pi;

	double kx = (double)width / ((double)width * cos(A) + (double)height * cos(B));
	double ky = (double)height / ((double)width * sin(A) + (double)height * sin(B) + 255.);
	double k = (kx < ky) ? kx : ky;
	double x0 = (double)width * cos(A) * kx;
	double y0 = 255. * ky;

	pTVFrame res = makecopyTVFrame(frame);
	return res;
}



#endif