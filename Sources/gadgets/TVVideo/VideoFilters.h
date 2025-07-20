// VideoFilters.h: interface for the CVideoFilters class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VIDEOFILTERS_H__EF8DB849_6D98_4541_858A_36367483834C__INCLUDED_)
#define AFX_VIDEOFILTERS_H__EF8DB849_6D98_4541_858A_36367483834C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Gadgets\VideoFrame.h>
#include <Gadgets\vftempl.h>
#include <ImageProc\colors.h>

// simpleip.h
DECLARE_VIDEO_FILTER(Any2Yuv9, ;);
DECLARE_VIDEO_FILTER(VideoNegative, ;);
DECLARE_VIDEO_FILTER_WITH_STDSETUP( VideoY16toY8 , int m_iShift  );
DECLARE_VIDEO_FILTER_WITH_STDSETUP( VideoY8toY16 , int m_iShift  );
DECLARE_VIDEO_FILTER( VideoY8toYUV9 , ; );
DECLARE_VIDEO_FILTER(VideoNormalize, ;);
DECLARE_VIDEO_FILTER(VideoClearColor, ;);
DECLARE_VIDEO_FILTER_WITH_STDSETUP(GammaCorrection, double m_gamma; );
DECLARE_VIDEO_FILTER_WITH_STDSETUP(MassBinarize, double m_Ratio;);
DECLARE_VIDEO_FILTER_WITH_STDSETUP(PseudoColor, \
  int m_iMinIntensity ; int m_iMaxIntensity ; UVComps * m_pTable ; 
  int m_iTableLen ; int m_iUsedMin ; int m_iUsedMax ; );
DECLARE_VIDEO_FILTER_WITH_STDSETUP(RangeContrast, \
  int m_iMinIntensity ; int m_iMaxIntensity ; int * m_pTable ; 
  bool m_bFilled ; );
DECLARE_VIDEO_FILTER(ColorBalance, ;);
DECLARE_VIDEO_FILTER(Equalize, ;);
DECLARE_VIDEO_FILTER(Block8, ;);

DECLARE_VIDEO_FILTER_WITH_STDSETUP(VideoFlip, int m_Type);
DECLARE_VIDEO_FILTER(Deinterlace,;);
DECLARE_VIDEO_FILTER_WITH_STDSETUP(VideoSimpleBinarize, int m_Level; int m_MaxLevel);
DECLARE_VIDEO_FILTER_WITH_STDSETUP(VideoNormalizeEx, unsigned min; unsigned max; int m_MaxLevel);
DECLARE_VIDEO_FILTER_WITH_STDSETUP(VideoMassNormalize, int percent);
DECLARE_VIDEO_FILTER_WITH_STDSETUP(VideoPercentBinarize, int m_Percent);
DECLARE_VIDEO_FILTER(HTriggerBinarize, ;);
DECLARE_VIDEO_FILTER_WITH_STDSETUP(VideoClearFrames, DWORD m_Color; DWORD m_Marge);
DECLARE_VIDEO_FILTER_WITH_STDSETUP(ColorBalanceM,int m_U; int m_V ;);

// fstfilter.h
DECLARE_VIDEO_FILTER(Video81PassFilter, ;);
DECLARE_VIDEO_FILTER(VideoEdgeDetector, ;);
DECLARE_VIDEO_FILTER(VideoFeatureDetector, ;);
DECLARE_VIDEO_FILTER_WITH_STDSETUP(VideoFlatten, int m_Times);
DECLARE_VIDEO_FILTER_WITH_STDSETUP(VideoFlatten2, int m_Times);
DECLARE_VIDEO_FILTER_WITH_STDSETUP(VideoHighPass1DV,int m_FrNmb);
DECLARE_VFILTER_WITH_STDSETUP_ANDASYNC( VideoLowPass ,\
int m_FrNmb; CRect m_ROI; FXLockObject m_Protect ;);
DECLARE_VIDEO_FILTER_WITH_STDSETUP(VideoLowPassM, int m_FrNmb; int m_iMatrixSide ; DWORD * m_pSums; DWORD m_dwLastWidth; );
DECLARE_VIDEO_FILTER_WITH_STDSETUP(VideoHighPass, int m_FrNmb);
DECLARE_VIDEO_FILTER_WITH_STDSETUP(VideoLowPass1DH, int m_FrNmb);
DECLARE_VIDEO_FILTER_WITH_STDSETUP(VideoHighPass1DH, int m_FrNmb);
DECLARE_VIDEO_FILTER_WITH_STDSETUP(VideoLowPass1DV, int m_FrNmb);
DECLARE_VIDEO_FILTER_WITH_STDSETUP(VideoSharpen, int m_FrNmb);

// resample.h
DECLARE_VIDEO_FILTER(VideoEnlarge, ;);
DECLARE_VIDEO_FILTER(VideoDiminish, ;);
DECLARE_VIDEO_FILTER(VideoDiminishX, ;);
DECLARE_VIDEO_FILTER(VideoEnlargeY, ;);
DECLARE_VIDEO_FILTER_WITH_STDSETUP(VideoResample, int newWidth; int newHeight);

// videologic.h
DECLARE_VIDEO_FILTER_WITH_STDSETUP(VideoErode,int m_iNInversed);
DECLARE_VIDEO_FILTER_WITH_STDSETUP(VideoDilate,int m_iNInversed);
DECLARE_VIDEO_FILTER(VideoErodeHorizontal, ;);
DECLARE_VIDEO_FILTER(VideoDilateHorizontal, ;);
DECLARE_VIDEO_FILTER(VideoErodeVertical, ;);
DECLARE_VIDEO_FILTER(VideoDilateVertical, ;);

// convolution
DECLARE_VIDEO_FILTER_WITH_STDSETUP(VideoHMeander, int m_nQSize);

#endif // !defined(AFX_VIDEOFILTERS_H__EF8DB849_6D98_4541_858A_36367483834C__INCLUDED_)
