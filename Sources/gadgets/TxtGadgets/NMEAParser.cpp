// NMEAParser.cpp: implementation of the NMEAParser class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NMEAParser.h"
#include "TxtGadgets.h"

IMPLEMENT_RUNTIME_GADGET_EX(NMEAParser, CFilterGadget, "Text.GPS", TVDB400_PLUGIN_NAME);


bool ParseNMEA(const FXString& Input, FXString& Output)
{
    FXSIZE idPos=Input.Find("$GP");
    if (idPos<0) { Output.Format("Error string: '%s'",Input); return false; };
    FXParser parser(Input);
    FXString cmd,time, status,latitude,hemisphere,longitude,longitudeh;
    FXSIZE pos=0;
    if (parser.GetWord(pos,cmd) && 
        parser.GetWord(pos,time) &&
        parser.GetWord(pos,status) && 
        status[0]=='A' &&
        parser.GetWord(pos,latitude) && 
        parser.GetWord(pos,hemisphere) &&
        parser.GetWord(pos,longitude) &&
        parser.GetWord(pos,longitudeh)
        )
    {
        Output.Format("%s%c, %s%c",latitude,hemisphere[0],longitude,longitudeh[0]);
    }
    else
        { Output.Format("Data is not actual: '%s'",Input); return false; };    
    return true;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

NMEAParser::NMEAParser()
{
	m_pInput = new CInputConnector(text);
	m_pOutput = new COutputConnector(text);
    Resume();
}

void NMEAParser::ShutDown()
{
    CFilterGadget::ShutDown();
	delete m_pInput;
	m_pInput = NULL;
	delete m_pOutput;
	m_pOutput = NULL;
}

CDataFrame* NMEAParser::DoProcessing(const CDataFrame* pDataFrame) 
{
	const CTextFrame* TextFrame = pDataFrame->GetTextFrame(DEFAULT_LABEL);
    FXString Output;
	CTextFrame* pResultFrame = NULL;

    if (TextFrame && ParseNMEA(TextFrame->GetString(), Output))
    {
        pResultFrame = CTextFrame::Create();
        pResultFrame->SetString(Output);
        pResultFrame->CopyAttributes(pDataFrame);
    }
	return pResultFrame;
}
