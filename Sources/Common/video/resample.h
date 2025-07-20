//  $File : resample.h - change scale of the image
//  (C) Copyright The File X Ltd 2002. 
//
//
//
//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 Firts release version, all followed changes must be listed below  (Andrey Chernushich)
//   26 Jun 02 Y8 format support for some of procedures is implemented

#ifndef _RESAMPLE_INC
#define _RESAMPLE_INC

bool FX_EXT_SHVIDEO _enlarge(pTVFrame frame);
bool FX_EXT_SHVIDEO _diminish(pTVFrame frame);
bool FX_EXT_SHVIDEO _diminishX(pTVFrame frame);

#endif _RESAMPLE_INC