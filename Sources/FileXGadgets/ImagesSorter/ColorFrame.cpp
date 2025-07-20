#include "stdafx.h"
#include "ColorFrame.h"

bool GetFrameNumberAndTime(FXString& src, __out UINT64& i64FrameID, __out UINT64& i64Time)
{
  return (_stscanf_s(src, _T("%llu_%llu"), &i64FrameID, &i64Time) == 2);
}
//bool GetFrameIdTimestampAndColorID(const CVideoFrame * pVf, __out UINT64& i64FrameID, __out UINT64& i64Time, __out int &iColorID)
//{
//  PVFEI pEmbedInfo = (PVFEI)GetData(pVf);
//  if (pEmbedInfo->m_dwIdentificationPattern == EMBED_INFO_PATTERN)
//  {
//    i64FrameID = pEmbedInfo->m_CameraFrameID;
//    i64Time = pEmbedInfo->m_CameraFrameTime;
//    iColorID = pEmbedInfo->m_ColorSign;
//    return true;
//  }
//  return false;
//}