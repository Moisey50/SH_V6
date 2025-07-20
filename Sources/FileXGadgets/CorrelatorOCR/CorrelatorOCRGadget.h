// CorrelatorOCRGadget.h : Declaration of the CorrelatorOCR

#pragma once

#define THIS_MODULENAME "Radial.CorrelatorOCR"

#include <math\intf_sup.h>
#include <Gadgets\gadbase.h>
#include <gadgets\videoframe.h>
#include <helpers\FramesHelper.h>
#include <Pattern.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include "stdafx.h"
using namespace std;

enum Mode
{
	Learning,
	Recognition
};

class CorrelatorOCR : public CFilterGadget
{
public:
    CorrelatorOCR(void);
    void ShutDown();

	bool ScanProperties(LPCTSTR text, bool& Invalidate);
	bool PrintProperties(FXString& text);
	bool ScanSettings(FXString& text);

    CDataFrame* DoProcessing(const CDataFrame* pDataFrame);

	FXString m_sDirPath ;
	string POSSIBLE_MODES[2];
	//const double m_dMinimalCoef = 0.7; // TODO: decide on a minimal coefficient to determine compatibility 

	int m_iHeight;
	int m_iWidth;

	std::vector<Pattern*> m_pPatterns; 

	Mode m_Mode; 

    DECLARE_RUNTIME_GADGET(CorrelatorOCR); //What is this?
};

