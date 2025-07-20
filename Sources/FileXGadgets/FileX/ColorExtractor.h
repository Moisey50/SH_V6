// ColorExtractor.h : Declaration of the ColorExtractor class

#pragma once
#include <gadgets\gadbase.h>

enum OutputColor
{
  Red = 0 ,
  Green ,
  Blue ,
  U ,
  V ,
  U_DIV_V ,
  V_DIV_U ,
  Radius
};


class ColorExtractor : public CFilterGadget
{
protected:
    OutputColor m_OutputColor;
    double      m_dContrast ;
private:
    ColorExtractor(void);
    void ShutDown();
public:
    CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	bool ScanProperties(LPCTSTR text, bool& Invalidate);
	bool PrintProperties(FXString& text);
	bool ScanSettings(FXString& text);

	DECLARE_RUNTIME_GADGET(ColorExtractor);
};
