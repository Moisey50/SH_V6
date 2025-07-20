// Trigger.cpp: implementation of the Trigger class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "blogic.h"
#include "Trigger.h"
#include <gadgets\QuantityFrame.h>
#include <gadgets\TextFrame.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_RUNTIME_GADGET_EX( Trigger , CControledFilter , LINEAGE_GENERIC , TVDB400_PLUGIN_NAME );
// IMPLEMENT_RUNTIME_GADGET_EX( ShiftRegister , CControledFilter , LINEAGE_LOGIC , TVDB400_PLUGIN_NAME );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Trigger::Trigger():
    m_Count(0)
  , m_bLastOnly(false)
{
  NeverSync(true);
}

CDataFrame* Trigger::DoProcessing(const CDataFrame* pDataFrame, const CDataFrame* pParamFrame)
{
  if (pParamFrame)
  {
    if ( !pParamFrame->IsContainer() )
    {
    switch (pParamFrame->GetDataType())
    {
      case quantity:
      {
        const CQuantityFrame* qFrame = pParamFrame->GetQuantityFrame(DEFAULT_LABEL);
          if ( qFrame )
          {
        m_Count = (long)*qFrame;
        m_Attrib = *(qFrame->Attributes());
      }
        }
      break;
      case text:
      {
        const CTextFrame* tFrame = pParamFrame->GetTextFrame(DEFAULT_LABEL);
          if ( tFrame )
          {
        FXString tmpS = tFrame->GetString();
        m_Count = atoi((LPCTSTR)tmpS);
        if (m_Count)
          m_Attrib = *(tFrame->Attributes());
      }
        }
      break;
      default: m_Count = 1 ; break ;
      }
    }
    else
        m_Count = 1;
  }
  if ((m_Count==0) && !Tvdb400_IsEOS(pDataFrame))
    return NULL;
  CDataFrame* retFrame=NULL;
  if (pDataFrame)
  {
    m_Count--; 
    m_Count=(((int)m_Count)<0) ? 0 : m_Count ;
    if ( m_bLastOnly && m_Count != 0 )
      return retFrame ; // NULL
    if ( m_Attrib.IsEmpty() )
    {
      retFrame=(CDataFrame*)pDataFrame;
      retFrame->AddRef(); // should be released once in parent class or marked as referenced when new
    }
    else
    {
      retFrame = pDataFrame->Copy() ;
      retFrame->Attributes()->Append( m_Attrib ) ;
    }
  }
  return retFrame ;
}
bool Trigger::ScanSettings(FXString& txt)
{
  txt.Format("template(ComboBox(LastOnly(true(%d),false(%d))))",TRUE,FALSE);
  return true;
}

bool Trigger::ScanProperties(LPCTSTR txt, bool& Invalidate)
{
  CControledFilter::ScanProperties(txt, Invalidate);
  FXPropertyKit pk(txt);
  pk.GetInt("LastOnly", m_bLastOnly);
  return true;
}

bool Trigger::PrintProperties(FXString& txt)
{
  FXPropertyKit pk;
  CControledFilter::PrintProperties(txt);
  pk.WriteInt("LastOnly",m_bLastOnly);
  txt+=pk;
  return true;
}

// ShiftRegister::ShiftRegister() :
// m_Count( 0 )
// , m_bLastOnly( false )
// {
//   NeverSync( true );
// }
