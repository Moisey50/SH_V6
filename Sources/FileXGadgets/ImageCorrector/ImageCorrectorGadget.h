// ImageCorrectorGadget.h : Declaration of the ImageCorrectorGadget class
#ifndef __INCLUDE__ImageCorrectorGadgetGadget_H__
#define __INCLUDE__ImageCorrectorGadgetGadget_H__


#pragma once
#include "helpers\UserBaseGadget.h"
#include "helpers\FramesHelper.h"
#include <imageproc\VFrameEmbedInfo.h>
#include "RGBContainerFrame/RGB2ContainerFrame2RGBDeSerializer.h"
#include "RGBMixer.h"

enum RGB_MODE
{
	MODE_UNKNOWN = 0,
	MODE_R = 1,
	MODE_G = MODE_R << 1,
	MODE_B = MODE_G << 1,
	MODE_RGB = MODE_R | MODE_G | MODE_B
};

class ImageCorrectorGadget
  : public UserBaseGadget
{
private:
#pragma region | Fields |
  RGBMixer      m_mixerRGB;
  CVideoFrame*  m_pVFOutput_MixedOrMono;
#pragma endregion | Fields |

  bool SendOut(CVideoFrame* pVF_MonochromOrRGB, RGB_STATE eColor, bool isSnapshot);

  ImageCorrectorGadget(const ImageCorrectorGadget&);
  ImageCorrectorGadget& operator=(const ImageCorrectorGadget&);


protected:

public:
  DECLARE_RUNTIME_GADGET(ImageCorrectorGadget);

	ImageCorrectorGadget();
	~ImageCorrectorGadget();

//	Mandatory functions
	void PropertiesRegistration();
	void ConnectorsRegistration();

	CDataFrame* DoProcessing(const CDataFrame* pDataFrame);  
	void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);

  static bool SendVideoFramesHandler(UserBaseGadget& source, RGB_STATE eColor, CVideoFrame* pVF_MonochromOrRGB, bool isSnapshot);
};
#endif