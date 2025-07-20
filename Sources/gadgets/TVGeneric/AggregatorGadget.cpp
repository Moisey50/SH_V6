// AggregatorGadget.cpp: implementation of the Aggregator class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TVGeneric.h"
#include "AggregatorGadget.h"
#include "ParamSetupDlg.h"

#define THIS_MODULENAME "Aggregator"

__forceinline FXString GetPathRoot(FXString& path, const char separator = '/')
{
	FXString root;
	FXSIZE pos = path.Find(separator);
	if (pos < 0)
	{
		root = path;
		path.Empty();
	}
	else
	{
		root = path.Left(pos);
		path = path.Mid(pos + 1);
	}
	return root;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_RUNTIME_GADGET_EX(Aggregator, CCollectorGadget, LINEAGE_GENERIC, TVDB400_PLUGIN_NAME);

Aggregator::Aggregator()
{
	RemoveInputs();
    m_Config="pin1=;pin2=;";
    m_pOutput = new COutputConnector(transparent);
    m_SetupObject = new CParamSetupDlg(this,"pin", NULL);
    Resume();
}

void Aggregator::ShutDown()
{
    CCollectorGadget::ShutDown();
}

bool Aggregator::PrintProperties(FXString& text)
{
	text = m_Config;
	return true;
}

bool Aggregator::ScanProperties(LPCTSTR text, bool& Invalidate)
{
	Suspend();
	ClearBuffers();
  if (strlen(text))
      m_Config = text;
	int i = 0;
	FXString pin, path;
	pin.Format("pin%d", ++i);
	while (m_Config.GetString(pin, path)) // more pins
		pin.Format("pin%d", ++i);
	CreateInputs(--i, nulltype, false);
	Resume();
    Status().WriteBool(STATUS_REDRAW, true);
	return true;
}

bool Aggregator::ScanSettings(FXString& text)
{
    text="calldialog(true)"; 
    return true;
}

CDataFrame* Aggregator::DoProcessing(CDataFrame const*const* frames, int nmb)
{
	if ( !frames || !(*frames) ) 
  {
    ASSERT(0) ;
    return NULL ;
  }

	CContainerFrame* pResult = CContainerFrame::Create();
  pResult->ChangeId(0) ;
	for (int i = 0; i < nmb; i++)
	{
		FXString pin, path;
		pin.Format("pin%d", i + 1);
		if (m_Config.GetString(pin, path))
		{
			const CDataFrame* Frame = *(frames + i);
			ASSERT(Frame);

      if ( (pResult->GetId() == 0)  &&  (Frame->GetId() != 0) )
        pResult->CopyAttributes( Frame );
		
      CContainerFrame* Branch = pResult;
			while (!path.IsEmpty())
			{
				FXString root = GetPathRoot(path);
				if (root.IsEmpty())
					continue;		// ignore nameless containers
				CDataFrame* SubBranch = NULL;
				CFramesIterator* Iterator = Branch->CreateFramesIterator(nulltype);
				if (Iterator)
				{
					SubBranch = Iterator->NextChild(root);
					while (SubBranch)
					{
						CFramesIterator* SubIterator = SubBranch->CreateFramesIterator(nulltype);
						if (SubIterator)
						{
							delete SubIterator;
							break;
						}
						SubBranch = Iterator->NextChild(root);
					}
					delete Iterator;
				}
				if (!SubBranch)
				{
					SubBranch = CContainerFrame::Create();
          SubBranch->CopyAttributes(Branch);
					SubBranch->SetLabel(root);
					Branch->AddFrame(SubBranch);
				}
				Branch = (CContainerFrame*)SubBranch;
			}
			Branch->AddFrame(Frame);
		}
	}
  return pResult;
}


