#include "StdAfx.h"
#include "Subtitle.h"
#include "gadgets\VideoFrame.h"
#include "gadgets\containerframe.h"

IMPLEMENT_RUNTIME_GADGET_EX(Subtitle, CFilterGadget, "Helpers", TVDB400_PLUGIN_NAME);

void CALLBACK onAddedText(CDataFrame* lpData, void* lpParam, CConnector* lpInput)
{
	((Subtitle*)lpParam)->onText(lpData);
}

#define PASSTHROUGH_NULLFRAME(vfr, fr)			\
{												\
	if (!(vfr) || ((vfr)->IsNullFrame()))	    \
  {	                                        \
	    return NULL;                            \
  }                                           \
}

Subtitle::Subtitle()
{
	prevTime = 0; 
	m_sFormatName = ""; 
	m_xText=0; 
	m_yText=0; 
	m_size = 1; 
	m_color="0xff0000"; 
	m_Lock.Lock();
	m_AppendString = "";
	m_Lock.Unlock();

	m_pInput = new CInputConnector(transparent);
	m_pInput2 = new CInputConnector(text, ::onAddedText, this);
	m_pOutput = new COutputConnector(transparent);

	Resume();
}

void Subtitle::ShutDown()
{
	CFilterGadget::ShutDown();
	delete m_pInput;
	m_pInput = NULL;
	delete m_pInput2;
	m_pInput2 = NULL;
	delete m_pOutput;
	m_pOutput = NULL;
	
}

void Subtitle::onText(CDataFrame *lpData)
{
	CTextFrame *pCf = NULL;
	if (!lpData)
		return;

	pCf = lpData->GetTextFrame();
	if (!pCf)
	{
		lpData->RELEASE(lpData);
		return;
	}
	m_Lock.Lock();
	m_AppendString = pCf->GetString();
	m_Lock.Unlock();
	lpData->RELEASE(lpData);
	return;
}

CDataFrame* Subtitle::DoProcessing(const CDataFrame *pDataFrame)
{
	const CVideoFrame* vf = pDataFrame->GetVideoFrame(DEFAULT_LABEL);
	PASSTHROUGH_NULLFRAME(vf, pDataFrame);

	if (!vf)
		return NULL;

	CContainerFrame *retCont = CContainerFrame::Create();
    retCont->CopyAttributes(pDataFrame);
	
	CTextFrame *ViewText = CTextFrame::Create();
  ViewText->CopyAttributes(pDataFrame);
	ViewText->Attributes()->WriteString("message", GenerateName(pDataFrame));
	ViewText->Attributes()->WriteInt("x", m_xText);
	ViewText->Attributes()->WriteInt("y", m_yText);
	ViewText->Attributes()->WriteInt("Sz", m_size);
	ViewText->Attributes()->WriteString("color", m_color);
	retCont->AddFrame(ViewText);
	retCont->AddFrame(pDataFrame);
	
	prevTime = pDataFrame->GetTime();
	return retCont;
}

FXString Subtitle::GenerateName(const CDataFrame *pDataFrame)
{
	FXString retV=m_sFormatName;
	CTime t = CTime::GetCurrentTime();
	FXString tmpS;
	FXSIZE pos;
	while((pos=retV.Find('%'))>=0)
	{
		tmpS=retV.Mid(pos,2);
		if (tmpS.GetLength()==2)
		{
			retV.Delete(pos,2);
			if (tmpS.Compare("%k")==0) //Frame time stamp
			{
				tmpS.Format("%lf", pDataFrame->GetTime()*1e-6);
			}
			else if (tmpS.Compare("%l")==0) //Frame ID
			{
				tmpS.Format("%Ld", pDataFrame->GetId());
			}
			else if (tmpS.Compare("%L")==0) // Frame Label
			{
				tmpS.Format("%s", pDataFrame->GetLabel());
			}
			else if (tmpS.Compare("%N")==0)
			{
				m_Lock.Lock();
				tmpS.Format("%s", m_AppendString);
				m_Lock.Unlock();
			}
			else if (tmpS.Compare("%T")==0) // Difference between incoming packets
			{
				tmpS.Format("%lf", (pDataFrame->GetTime()-prevTime)*1e-6);
			}
			else if (tmpS.Compare("%D")==0 || tmpS.Compare("%H")==0 || tmpS.Compare("%M")==0 || tmpS.Compare("%S")==0)// treat like standard time format string
			{
				tmpS=t.Format(tmpS);
				// remove forbidden chars
				tmpS.Replace( ':', '-');
				tmpS.Replace( '/', '.');
				tmpS.Replace( '\\', ' ');
				tmpS.Replace( '*', '+');
				tmpS.Replace( '|', 'I');
				tmpS.Replace( '?', ' ');
				tmpS.Replace( '>', ' ');
				tmpS.Replace( '<', ' ');
				tmpS.Replace( '"', '\'');
			}
			else
				tmpS = tmpS.GetAt(1);
			//not a valid format symbol!
			retV.Insert(pos,tmpS);
		}
	}
	return retV;
}

bool Subtitle::PrintProperties(FXString& text)
{
	FXPropertyKit pc;
	pc.WriteString("FormatString",m_sFormatName);
	pc.WriteInt("XOffset", m_xText);
	pc.WriteInt("YOffset", m_yText);
	pc.WriteInt("Size", m_size);
	pc.WriteString("Color", m_color);
	m_Lock.Lock();
	pc.WriteString("AppendString", m_AppendString);
	m_Lock.Unlock();
	text=pc;
	return true;
}

bool Subtitle::ScanProperties(LPCTSTR text, bool& Invalidate)
{
	FXPropertyKit pc(text);
	pc.GetString("FormatString",m_sFormatName);
	pc.GetInt("XOffset", m_xText);
	pc.GetInt("YOffset", m_yText);
	pc.GetInt("Size", m_size);
	pc.GetString("Color", m_color);
	m_Lock.Lock();
	pc.GetString("AppendString", m_AppendString);
	m_Lock.Unlock();
	return true;
}


bool Subtitle::ScanSettings(FXString& text)
{
	text.Format("template(EditBox(FormatString),Spin(XOffset,0,640),Spin(YOffset,0,480),Spin(Size,1,100),EditBox(Color))",TRUE,FALSE);
	return true;
}

