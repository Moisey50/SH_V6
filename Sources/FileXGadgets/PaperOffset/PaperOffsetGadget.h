#pragma once

#include <gadgets\gadbase.h>
 #include "classes/dpoint.h"

class PaperOffset : public CFilterGadget {
	CPoint m_FirstPoint;
	CPoint m_SecondPoint;
	CPoint m_TopFirstPoint;
	CPoint m_TopSecondPoint;
	int m_PartsNum;
	int m_IntensOffset;
	int m_GradOffset;
	int m_AveragePointCount;

	CDPoint GetWhitePointUp(LPBYTE data, int xSize, int ySize, int x, int y, bool firstPoint);
	double GetIntensityAt(LPBYTE data, int xSize, int ySize, int x, int y);
	double GetSquare(CDPoint *bottom, CDPoint *top, double xStep);
public:
	PaperOffset();
	~PaperOffset();
	CDataFrame *DoProcessing(const CDataFrame* pDataFrame);
	bool PrintProperties(FXString& text);
	bool ScanProperties(LPCTSTR text, bool& Invalidate);
	bool ScanSettings(FXString& text);
	DECLARE_RUNTIME_GADGET(PaperOffset);
};