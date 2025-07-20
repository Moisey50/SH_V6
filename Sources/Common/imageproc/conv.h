#ifndef _IP_CONVOLUTION_METHODS_INC
#define _IP_CONVOLUTION_METHODS_INC

#include "math\hbmath.h"
#ifndef PI
#define PI (3.1415926535897932384626433832795)
#endif
#define _2PI (6.283185307179586476925286766559)

#define DECOMPOSE_HARMONIC	0
//#define DECOMPOSE_MEANDR	1

inline double* _prepare_fft_lut(int m, int rev)
{
  double pi=3.141592653589793;
  double* lut = (double*)malloc(2 * m * sizeof(double));
  for (int i = 0; i < m; i++)
  {
    double arg = pi * (double)rev / (double)((1 << (i + 1)) / 2);
#if defined DECOMPOSE_HARMONIC
    lut[2 * i] = cos(arg);
    lut[2 * i + 1] = -sin(arg);
#elif defined DECOMPOSE_MEANDR
    lut[2 * i] = ((cos(arg) < 0) ? -1. : 1.) * (pi / 2. / (double)m);
    lut[2 * i + 1] = ((-sin(arg) < 0) ? -1. : 1.) * (pi / 2./ (double)m);
#endif
  }
  return lut;
}

inline void _cfft_1(double *xr, double *xi, int m, double* lut)
{
  double ur,ui,ut,tr,ti,wr,wi;
  int n,n1,n2,i,j,l,k,l1,l2,ip;
  n= 1<<m;
  n2 = n/2;
  n1=n-1;
  j=1;
  for (i=1;i<=n1;i++)
  {
    if((i-j) < 0)
    {
      tr=*(xr + j-1);
      ti=*(xi + j-1);
      *(xr + j-1)=*(xr + i-1);
      *(xi + j-1)=*(xi + i-1);
      *(xr + i-1)=tr;
      *(xi + i-1)=ti;
    }
    k=n2;
    while ((k-j) <0) { j -= k; k /= 2; }
    j += k;
  }
  for (l=1;l<=m; l++)
  {
    l2 = 1<<l;
    l1 = l2/2;
    ur = 1.0;
    ui = 0.0;
    wr = lut[2 * l - 2];
    wi = lut[2 * l - 1];
    for (j=1; j <= l1; j++)
    {
      for (i=j;i <= n; i += l2)
      {
        ip = i+ l1;
        tr = *(xr + ip-1)*ur - *(xi+ip-1)*ui;
        ti = *(xr+ip-1)*ui + *(xi+ip-1)*ur;
        *(xr+ip-1) = *(xr+i-1)-tr;
        *(xi+ip-1) = *(xi+i-1)-ti;
        *(xr+i-1) = *(xr+i-1)+tr;
        *(xi+i-1) = *(xi+i-1)+ti;
      }
      ut = ur*wr - ui*wi;
      ui = ur*wi + ui*wr;
      ur = ut;
    }
  }
}


inline void _cfft(double *xr, double *xi, int m, int rev)
{
  const double pi=3.141592653589793;

  int n = 1 << m, n2 = n / 2, j = 1;
  int i;
  for (i = 1; i < n; i++)
  {
    if((i - j) < 0)
    {
      double tmp = xr[j-1]; xr[j-1] = xr[i-1]; xr[i-1] = tmp;
      tmp = xi[j-1]; xi[j-1] = xi[i-1]; xi[i-1] = tmp;
    }
    int k = n2;
    while ((k - j) < 0) { j -= k; k /= 2; }
    j += k;
  }
  for (int l = 1; l <= m; l++)
  {
    int l2 = 1 << l, l1 = l2 / 2;
    double p1 = pi * (double)rev / (double)l1;
    double wr = cos(p1), wi = -sin(p1), ur = 1., ui = .0;
    for (j = 1; j <= l1; j++)
    {
      for (i = j; i <= n; i += l2)
      {
        int ip = i + l1;
        double tmp = xr[ip-1] * ur - xi[ip-1] * ui;
        xr[ip-1] = xr[i-1] - tmp;
        xr[i-1] += tmp;
        tmp = xr[ip-1] * ui + xi[ip-1] * ur;
        xi[ip-1] = xi[i-1] - tmp;
        xi[i-1] += tmp;
      }
      double tmp = ur * wr - ui * wi;
      ui = ur * wi + ui * wr; ur = tmp;
    }
  }
}

inline double* _align_pow2(LPBYTE inData, DWORD& width, DWORD& height, int& pwr)
{
  if (!inData) return NULL;
  DWORD side = 1;
  pwr = 0;
  while ((side < width) || (side < height)) { side *= 2; pwr++; }
  DWORD size = side * side;
  double* outData = (double*)calloc(size, sizeof(double));
  if (!outData) return NULL;
  int avg=0;
  for (unsigned i=0; i<width * height; i++)
  {
    avg+=inData[i];
  }
  avg/=width*height;
  DWORD iSrc = 0, iDst = 0;
  while (iSrc < width * height)
  {
    outData[iDst] = (double)inData[iSrc]-avg;
    iSrc++;
    iDst++;
    if (iSrc % width == 0) iDst += (side - width);
  }
  height = width = side;
  return outData;
}

inline double* _align_preset(LPBYTE inData, DWORD width, DWORD height, DWORD side)
{
  if (!inData || (side < width) || (side < height)) return NULL;
  double* outData = (double*)calloc(side * side, sizeof(double));
  if (!outData) return NULL;
  int avg=0;
  for (unsigned i=0; i<width * height; i++)
  {
    avg+=inData[i];
  }
  avg/=width*height;
  DWORD iSrc = 0, iDst = 0;
  while (iSrc < width * height)
  {
    outData[iDst] = (double)inData[iSrc]-avg;
    iSrc++;
    iDst++;
    if (iSrc % width == 0) iDst += (side - width);
  }
  return outData;
}

inline void _restore_align(LPBYTE outData, double* inData, DWORD side, DWORD width, DWORD height)
{
  if (!inData || !outData || (side < width) || (side < height)) return;
  DWORD iSrc = 0, iDst = 0;
  double* crsr = inData, min = inData[0], max = min;
  while (crsr < inData + side * side)
  {
    if (min > *crsr) min = *crsr;
    if (max < *crsr) max = *crsr;
    crsr++;
  }
  double rate = 1.;
  if (max > min) rate = 255. / (max - min);
  while (iDst < width * height)
  {
    outData[iDst] = (BYTE)((inData[iSrc] - min) * rate);
    iSrc++;
    iDst++;
    if (iDst % width == 0) iSrc += (side - width);
  }
}

inline void _restore_align_center(LPBYTE outData, double* inData, DWORD side)
{
  if (!inData || !outData) return;
  double* crsr = inData, min = inData[0], max = min;
  while (crsr < inData + side * side)
  {
    if (min > *crsr) min = *crsr;
    if (max < *crsr) max = *crsr;
    crsr++;
  }
  double rate = 1.;
  if (max > min) rate = 255. / (max - min);
  DWORD iSrc = 0, iDst = (side / 2) * side + side / 2;
  while (iSrc < side * side)
  {
    outData[iDst] = (BYTE)((inData[iSrc] - min) * rate);
    iSrc++;
    iDst++;
    if (iDst % side == 0) iDst -= side;
    else if (iDst % (side / 2) == 0) iDst += side;
    if (iDst >= side * side) iDst = side / 2;
  }
}

inline void _restore_align_center2(LPBYTE outData, double* inData, DWORD side)
{
  if (!inData || !outData) return;
  DWORD iSrc = 1;
  double min = inData[0] * inData[0], max = min;
  while (iSrc < side * side)
  {
    if (min > inData[iSrc] * inData[iSrc]) min = inData[iSrc] * inData[iSrc];
    if (max < inData[iSrc] * inData[iSrc]) max = inData[iSrc] * inData[iSrc];
    iSrc++;
  }
  double rate = 1.;
  if (max > min) rate = 255. / (max - min);
  iSrc = 0;
  DWORD iDst = (side / 2) * side + side / 2;
  while (iSrc < side * side)
  {
    outData[iDst] = (BYTE)((inData[iSrc] * inData[iSrc] - min) * rate);
    iSrc++;
    iDst++;
    if (iDst % side == 0) iDst -= side;
    else if (iDst % (side / 2) == 0) iDst += side;
    if (iDst >= side * side) iDst = side / 2;
  }
}

inline void _correct_R(LPBYTE outData, DWORD side)
{
  int half=side/2;
  for (DWORD y=1; y<side/2; y++)
  {
    for (DWORD x=1; x<side/2; x++)
    {
      double index=sqrt(x*x+y*y);
      outData[(half+y)*side+half+x]=(unsigned char)(outData[(half+y)*side+half+x]*index);
      outData[(half-y)*side+half+x]=(unsigned char)(outData[(half-y)*side+half+x]*index);
      outData[(half+y)*side+half-x]=(unsigned char)(outData[(half+y)*side+half-x]*index);
      outData[(half-y)*side+half-x]=(unsigned char)(outData[(half-y)*side+half-x]*index);
    }
  }
}

inline void _correct_R2(LPBYTE outData, DWORD side)
{
  int half=side/2;
  for (DWORD y=1; y<side/2; y++)
  {
    for (DWORD x=1; x<side/2; x++)
    {
      double radius=/*sqrt*/(x*x+y*y);
      int index1=(half+y)*side*3+(half+x)*3;
      int index2=(half-y)*side*3+(half+x)*3;
      int index3=(half+y)*side*3+(half-x)*3;
      int index4=(half-y)*side*3+(half-x)*3;
      outData[index1]=(unsigned char)(outData[index1]*radius);
      outData[index1+1]=(unsigned char)(outData[index1+1]*radius);
      outData[index1+2]=(unsigned char)(outData[index1+2]*radius);
      outData[index2]=(unsigned char)(outData[index2]*radius);
      outData[index2+1]=(unsigned char)(outData[index2+1]*radius);
      outData[index2+2]=(unsigned char)(outData[index2+2]*radius);
      outData[index3]=(unsigned char)(outData[index3]*radius);
      outData[index3+1]=(unsigned char)(outData[index3+1]*radius);
      outData[index3+2]=(unsigned char)(outData[index3+2]*radius);
      outData[index4]=(unsigned char)(outData[index4]*radius);
      outData[index4+1]=(unsigned char)(outData[index4+1]*radius);
      outData[index4+2]=(unsigned char)(outData[index4+2]*radius);
    }
  }
}

inline void _color_frame(LPBYTE outData, double* inData, DWORD side)
{
  DWORD dstSide = side / 4;
  DWORD iSrc = 0, iDst = (dstSide / 2) * dstSide + dstSide / 2;
  while (iSrc < side * (side - 3))
  {
    double phs = inData[iSrc] + inData[iSrc+1] + inData[iSrc+2] + inData[iSrc+3] +
      inData[iSrc+side] + inData[iSrc+side+1] + inData[iSrc+side+2] + inData[iSrc+side+3] +
      inData[iSrc+2*side] + inData[iSrc+2*side+1] + inData[iSrc+2*side+2] + inData[iSrc+2*side+3] +
      inData[iSrc+3*side] + inData[iSrc+3*side+1] + inData[iSrc+3*side+2] + inData[iSrc+3*side+3];
    phs /= 8;	// average phase of 4x4 square * 2
    outData[iDst] = 128 + (BYTE)(127. * sin(phs));
    outData[iDst+dstSide*dstSide] = 128 + (BYTE)(127. * cos(phs));

    iSrc += 4;
    if (iSrc % side == 0) 
      iSrc += 3 * side;
    iDst++;
    if (iDst % dstSide == 0) iDst -= dstSide;
    else if (iDst % (dstSide / 2) == 0) iDst += dstSide;
    if (iDst >= dstSide * dstSide) iDst = dstSide / 2;
  }
}

inline void _color_center_24bit(LPBYTE outData, double* amp, double* phs, DWORD side, double rcf = 1.)
{
  if (!amp || !phs || !outData) 
    return;
  double* atmp = (double*)malloc(side * side * sizeof(double));
  if (!atmp) return;
  double* ptmp = (double*)malloc(side * side * sizeof(double));
  if (!ptmp) 
  { 
    free(atmp); 
    return; 
  }

  DWORD iSrc = 0, iDst = side / 2 * side + side / 2;
  atmp[iDst] = amp[iSrc];
  ptmp[iDst] = phs[iSrc];
  double min = atmp[iDst], max = min;
  iDst++;
  for (iSrc = 1; iSrc < side * side; iSrc++)
  {
    int x = (iSrc % side > side / 2) ? side - iSrc % side : iSrc % side;
    int y = (iSrc / side > side / 2) ? side - iSrc / side : iSrc / side;
    double radius = /*sqrt*/(x * x + y * y);
    atmp[iDst] = amp[iSrc] * pow(radius, rcf/2);
    //atmp[iDst] *=atmp[iDst];
    if (min > atmp[iDst])  min = atmp[iDst];
    if (max < atmp[iDst])  max = atmp[iDst];
    ptmp[iDst] = phs[iSrc];
    iDst++;
    if (iDst % side == 0) iDst -= side;
    else if (iDst % (side/2) == 0) iDst += side;
    if (iDst >= side * side) iDst = side / 2;
  }
  double rate = 1.;
  if (max > min) rate = 255. / (max - min);

  for (iSrc = 0, iDst = 0; iSrc < side * side; iSrc++, iDst+=3)
  {
    double k = ptmp[iSrc];
    while (k<0)  k+=_2PI;
    while (k>_2PI) k-=_2PI;

    double I=((atmp[iSrc] - min) * rate)*0.298;
    double r,g,b;

    double cosk=k/PI-1;//cos(k);
    ASSERT(cosk>=-1.0);
    ASSERT(cosk<=1.0);
    //TRACE("cosk=%g\n",cosk);
    if (cosk<0)
    {
      r=-cosk*I/0.298;
      if (r>255) r=255;
      g=(I-0.298*r)/0.586;
      if (g>255)
      {
        g=255;
        b=(I-0.586*g-0.298*r)/0.116;
      }
      else b=0;
    }
    else
    {
      b=cosk*I/0.298;
      if (b>255) b=255;
      g=(I-0.116*b)/0.586;
      if (g>255)
      {
        g=255;
        r=(I-0.586*g-0.116*b)/0.298;
      }
      else r=0;
    } 
    outData[iDst]   = (BYTE)(b);
    outData[iDst+1] = (BYTE)(g);
    outData[iDst+2] = (BYTE)(r);
  }
  free(atmp);
  free(ptmp);
}

inline void _rotate180(LPBYTE inData, DWORD width, DWORD height)
{
  DWORD size = width * height, i = 0;
  while (i < size / 2)
  {
    BYTE tmp = inData[i]; inData[i] = inData[size - i - 1]; inData[size - i - 1] = tmp;
    i++;
  }
}

inline void _fft_2D(double* xr, double* xi, int m, int rev)
{
  int side = 1 << m;
  double *real = xr, *imag = xi, *endr = xr + side * side;
  double* lut = _prepare_fft_lut(m, rev);
  while (real < endr)
  {
    _cfft_1(real, imag, m, lut);
    real += side; imag += side;
  }
  int j;
  for (j = 0; j < side; j++)
  {
    for (int i = j + 1; i < side; i++)
    {
      double tmp = xr[j * side + i]; xr[j * side + i] = xr[i * side + j]; xr[i * side + j] = tmp;
      tmp = xi[j * side + i]; xi[j * side + i] = xi[i * side + j]; xi[i * side + j] = tmp;
    }
  }
  real = xr; imag = xi; endr = xr + side * side;
  while (real < endr)
  {
    _cfft_1(real, imag, m, lut);
    real += side; imag += side;
  }
  for (j = 0; j < side; j++)
  {
    for (int i = j + 1; i < side; i++)
    {
      double tmp = xr[j * side + i]; xr[j * side + i] = xr[i * side + j]; xr[i * side + j] = tmp;
      tmp = xi[j * side + i]; xi[j * side + i] = xi[i * side + j]; xi[i * side + j] = tmp;
    }
  }
  free(lut);
}

inline void _complex_multiply(double* xr, double* xi, double* yr, double* yi, DWORD length)
{
  DWORD i = 0;
  while (i < length)
  {
    double tmp = xr[i];
    xr[i] = xr[i] * yr[i] - xi[i] * yi[i];
    xi[i] = tmp * yi[i] + xi[i] * yr[i];
    i++;
  }
}

inline void _complex_magnitude(double* xr, double* xi, DWORD length)
{
  DWORD i = 0;
  while (i < length)
  {
    double tr = xr[i];
    xr[i] = sqrt(xr[i] * xr[i] + xi[i] * xi[i]); // amplitude
    xi[i] = (tr == 0) ? ((xi[i] > 0) ? acos(0.0) : (xi[i] < 0) ? -acos(0.0) : 0) :
      (tr > 0) ? atan(xi[i] / tr) : atan(xi[i] / tr) + acos(-1.0);
    i++;
  }
}

inline double _ampl( double* pdReal , double* pdImag , 
  double * pdAmpl , DWORD length )
{
  double * pdEnd = pdReal + length ;
  double dMax = 0. ;
  while ( pdReal < pdEnd )
  {
    double dAmpl = *(pdAmpl++) = sqrt( (*pdReal) * (*pdReal) + (*pdImag) * (*pdImag) ) ;
    if ( dAmpl > dMax )
      dMax = dAmpl ;
    pdReal++ ;
    pdImag++ ;
  }
  return dMax ;
}

// inline double _ampl( double* pdReal , double* pdImag ,
//   cmplx * pcAmpl , DWORD length )
// {
//   double * pdEnd = pdReal + length ;
//   double dMax = 0. ;
//   while ( pdReal < pdEnd )
//   {
//     double dAmpl = *(pdAmpl++) = sqrt( (*pdReal) * (*pdReal) + (*pdImag) * (*pdImag) ) ;
//     if ( dAmpl > dMax )
//       dMax = dAmpl ;
//     pdReal++ ;
//     pdImag++ ;
//   }
//   return dMax ;
// }


inline void _complex_decompose(double* xr, double* xi, DWORD length)
{
  DWORD i = 0;
  while (i < length)
  {
    double ta = xr[i];	// amplitude;
    xr[i] = ta * cos(xi[i]);
    xi[i] = ta * sin(xi[i]);
    i++;
  }
}

inline void _fft_conv(LPBYTE Image, DWORD iWidth, DWORD iHeight, LPBYTE mask, DWORD mWidth, DWORD mHeight)
{
  if ((iWidth <= mWidth) || (iHeight <= mHeight)) return;
  DWORD width = iWidth, height = iHeight;

  _rotate180(mask, mWidth, mHeight);
  int pwr;
  double* rImage = _align_pow2(Image, width, height, pwr);
  double* iImage = (double*)calloc(width * height, sizeof(double));
  double* rMask = _align_preset(mask, mWidth, mHeight, width);
  double* iMask = (double*)calloc(width * height, sizeof(double));
  _fft_2D(rImage, iImage, pwr, 1);
  _fft_2D(rMask, iMask, pwr, 1);
  _complex_multiply(rImage, iImage, rMask, iMask, width * height);
  _fft_2D(rImage, iImage, pwr, -1);
  //	_complex_magnitude(rImage, iImage, width * height);
  _restore_align(Image, rImage, width, iWidth, iHeight);
  LPBYTE Color = Image + iWidth * iHeight;
  memset(Color, 0x7f, iWidth * iHeight / 8);

  free(iMask);
  free(rMask);
  free(iImage);
  free(rImage);
}

inline void _fft_do_fft(LPBYTE Image, LPBYTE* Real, LPBYTE* Imag, DWORD& iWidth, DWORD& iHeight)
{
  int pwr;
  double* rImage = _align_pow2(Image, iWidth, iHeight, pwr);
  double* iImage = (double*)calloc(iWidth * iWidth, sizeof(double));
  *Real = (LPBYTE)calloc(iWidth * iHeight * 9 / 8, sizeof(BYTE));
  *Imag = (LPBYTE)calloc(iWidth * iHeight * 9 / 8, sizeof(BYTE));
  if (rImage && iImage && *Real && *Imag)
  {
    _fft_2D(rImage, iImage, pwr, 1);
    _complex_magnitude(rImage, iImage, (DWORD)iWidth * iWidth);
    _restore_align(*Real, rImage, iWidth, iWidth, iHeight);
    _restore_align(*Imag, iImage, iWidth, iWidth, iHeight);
    memset(*Real + iWidth * iHeight, 0x7F, iWidth * iHeight / 8);
    memset(*Imag + iWidth * iHeight, 0x7F, iWidth * iHeight / 8);
  }
  free(iImage);
  free(rImage);
}

inline LPBYTE _fft_do_ifft(LPBYTE Amp, LPBYTE Phase, DWORD width, DWORD height)
{
  int pwr;
  double* rImage = _align_pow2(Amp, width, height, pwr);
  double* iImage = _align_pow2(Phase, width, height, pwr);
  LPBYTE Image = (LPBYTE)calloc(width * height, sizeof(BYTE));
  if (rImage && iImage && Image)
  {
    _complex_decompose(rImage, iImage, width * height);
    _fft_2D(rImage, iImage, pwr, -1);
    _restore_align(Image, rImage, width, width, height);
  }
  return Image;
}

#endif