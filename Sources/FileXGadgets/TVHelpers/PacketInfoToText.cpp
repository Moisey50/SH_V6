#include "StdAfx.h"
#include "PacketInfoToText.h"
#include <gadgets\VideoFrame.h>
#include <gadgets\textframe.h>
#include <map>
#include <algorithm>

IMPLEMENT_RUNTIME_GADGET_EX(PacketInfoToText, CFilterGadget, "Helpers", TVDB400_PLUGIN_NAME);

#define PASSTHROUGH_NULLFRAME(vfr, fr)	 	\
{											\
	if (!(vfr) || ((vfr)->IsNullFrame()))	\
{	                                    \
	return NULL;                        \
}                                       \
}


class Info
{
	FXString m_id;
	FXString m_label;
	FXString m_dataType;
	FXString m_timeStamp;
	FXString m_timeFromStart;
	FXString m_isRegistred;
	FXString m_formatName;
	FXString m_frameH;
	FXString m_frameW;
	std::map<int, std::pair<FXString, FXString> > m_params;

public:
	Info(const CDataFrame& src) : m_id("EOS"), m_label(), m_dataType("Undefined"), m_timeStamp("Undefined"), m_timeFromStart("Undefined"),m_isRegistred(),m_formatName(),m_frameH(),m_frameW()
	{
		bool isEOS = Tvdb400_IsEOS(&src);
		double frTime=src.GetTime();
		
		m_dataType = Tvdb400_TypeToStr(src.GetDataType());
		if(frTime != -1)
			m_timeFromStart.Format("%.0f", frTime);	
		
		if (!isEOS)
		{
			m_id. Format("%d", src.GetId());
			m_label = src.GetLabel();
			m_isRegistred.Format("%s", (src.IsRegistered())?"true":"false");

			AddParam(3, "Label", m_label);
			AddParam(4, "Registered", m_isRegistred);

			m_timeFromStart = getTimeAsText(frTime);

			if (src.GetDataType()==vframe)
			{
				const CVideoFrame* vf=src.GetVideoFrame(DEFAULT_LABEL);
				char FOURCC[5]={0};
				memcpy(FOURCC, &vf->lpBMIH->biCompression, 4);
				FOURCC[4]=0;

				m_formatName.Format("%s", FOURCC);
				m_frameH.Format("%d", vf->lpBMIH->biWidth);
				m_frameW.Format("%d", vf->lpBMIH->biHeight);

				AddParam(5, "Video format", m_formatName);
				AddParam(6, "Frame size", m_frameH + "x" + m_frameW);
			}
		}
		AddParam(0, "ID", m_id);
		AddParam(1, "Time", m_timeFromStart);
		AddParam(2, "Type", m_dataType);
	}

	FXString getTimeAsText(DOUBLE ftime)
	{
		FXString sTime;
		sTime.Empty();
		if(ftime!=-1)
		{
			unsigned sec = (unsigned)(ftime / 1000000);
			int hours = sec / 3600;
			int mins = (sec % 3600) / 60;
			sec %= 60;
			int msec = ((unsigned)(ftime/1000))%1000;

			sTime.Format("%02d:%02d:%02d.%03d", hours, mins, sec, msec);
		}
		return sTime;
	}

	void AddParam(int id, const FXString& key, const FXString& val)
	{		
		m_params[id].first = key;
		m_params[id].second = val;		
	}

	struct InfoPredicate
	{
		FXString m_res;
		FXString getResult()const{return m_res;}
		void operator()(std::pair<int, std::pair<FXString, FXString> > p)
		{
			m_res+=(p.second.first + ": " + p.second.second + ";");
		}
	};

	FXString ToString()
	{
		InfoPredicate prd;
		prd = for_each(m_params.begin(), m_params.end(), prd);
		return prd.getResult();
	}
};


PacketInfoToText::PacketInfoToText(void)
{
	m_pInput = new CInputConnector(vframe);
	m_pOutput = new COutputConnector(text);
	Resume();
}

void PacketInfoToText::ShutDown()
{
    CFilterGadget::ShutDown();
	delete m_pInput;
	m_pInput = NULL;
	delete m_pOutput;
	m_pOutput = NULL;
}

CDataFrame* PacketInfoToText::DoProcessing(const CDataFrame* pDataFrame)
{
	const CVideoFrame* vFrame = pDataFrame->GetVideoFrame(DEFAULT_LABEL);
	PASSTHROUGH_NULLFRAME(vFrame, pDataFrame);
	Info inf(*pDataFrame);
    CTextFrame* retVal= CTextFrame::Create(inf.ToString());
    retVal->CopyAttributes(pDataFrame);
    return retVal;
}
