#include "stdafx.h"
#include "LabelFilter.h"
#include <Math\Intf_sup.h>

IMPLEMENT_RUNTIME_GADGET_EX(LabelFilter, CFilterGadget, LINEAGE_GENERIC, TVDB400_PLUGIN_NAME);

const int typesdecoder[]={0,    // all types
  -1,     // containers
  text,
  vframe,
  figure,
  quantity,
  rectangle,
  wave,
  logical,
  metafile,
  userdata,
  nulltype};

const int typesdecoder_size=sizeof(typesdecoder)/sizeof(int);

CompareMode CompareModes[] =
{
  { COMPARE_EXACT , _T("exact") } , 
  { COMPARE_WITH_PREFIX , _T("with_prefix") } ,
  { COMPARE_BEGIN , _T("begin") } ,
  { COMPARE_ANY , _T("any_place") }
} ;

LabelFilter::LabelFilter(void):
m_Label("")
{
  m_OutputMode=modeReplace;
  m_pInput = new CInputConnector(transparent);
  m_pOutput = new COutputConnector(transparent);
  m_TypeForFiltering = 0; //all types
  m_CompareMode = TRUE ;
  m_bInverseRule = FALSE ;
  m_bAnalyzeLabels = FALSE ;
  Resume();
}

void LabelFilter::ShutDown()
{
  CFilterGadget::ShutDown();
  delete m_pInput;
  m_pInput = NULL;
  delete m_pOutput;
  m_pOutput = NULL;
}

bool LabelFilter::PrintProperties(FXString& text)
{
  FXPropertyKit pk;
  pk.WriteString(_T("Label"), m_Label.Trim());
  pk.WriteInt( _T("Available") , -1 ) ;
  //pk.WriteString( _T("Compare") , CompareModes[m_CompareMode].ModeName ) ;
  pk.WriteInt( _T("Compare") , m_CompareMode ) ;
  pk.WriteInt( _T("InverseRule") , m_bInverseRule ) ;
  pk.WriteString( _T("Attributes") , 
    ( !m_Attributes.IsEmpty() ) ? m_Attributes : _T(" ")) ;
  for ( int i = 0 ; i < typesdecoder_size ; i++ )
  {
    if ( typesdecoder[i] == m_TypeForFiltering )
    {
      pk.WriteInt( "Type" , i ) ;
      break ;
    }
  }
  text = pk;
  return true;
}

FXString _getnext(LPCTSTR& label)
{
  FXString retV;
  while ( (*label!=_T('/')) && (*label!=0))
  {
    retV+=*label; 
    label++;
  }
  if (*label==_T('/')) 
    label++;
  return retV;
}

bool LabelFilter::ScanProperties(LPCTSTR text, bool& Invalidate)
{
  FXAutolock al( m_Lock ) ;
  FXPropertyKit pk(text);
  int iIndex ;
  Invalidate = true ;
  if ( pk.GetString(_T("Label"), m_Label) )
  {
    LPCTSTR Labels = ( LPCTSTR ) m_Label ;
    FXString NextLabel = _getnext( Labels ) ;
    m_Protect.Lock() ;
    m_PatternLabels.RemoveAll() ;
    while ( !NextLabel.IsEmpty() )
    {
      m_PatternLabels.Add( NextLabel ) ;
      NextLabel = _getnext( Labels ) ;
    }
    m_Protect.Unlock() ;
  }
  if ( pk.GetInt( _T("Available") , iIndex ) 
    && (iIndex >= 0) && ( iIndex < m_LastLabels.GetCount() ) )
  {
    m_Label.Trim();
    if (m_Label.GetLength())
    {
      m_Label += "/";
    }
    m_Label += m_LastLabels[iIndex].Trim(); ;
  }
  int iType ;
  if (pk.GetInt( _T("Type") , iType ) ) 
  {
    DWORD dwOldType = m_TypeForFiltering ;
    m_TypeForFiltering = typesdecoder[iType] ;
    if ( dwOldType != m_TypeForFiltering )
    {
      m_LastLabels.RemoveAll() ;
      Sleep( 20 ) ;
    }
  }
//   FXString ModeAsString ;
//   if ( pk.GetString( _T("Compare") , ModeAsString ) )
//   {
//     for ( int i = 0 ; i < ARRSZ( CompareModes ) ; i++ )
//     {
//       if ( ModeAsString == CompareModes[i].ModeName )
//       {
//         m_CompareMode = i ;
//         break ;
//       }
//     }
//   }
//  
  pk.GetInt( _T("Compare") , m_CompareMode ) ;
  pk.GetInt( _T("InverseRule") , m_bInverseRule ) ;
  if ( pk.GetString( _T("Attributes") , m_Attributes ) )
  {
    m_Attributes.Trim( _T(" \t") ) ;
  }
  if (pk.GetInt( "AnalyzeLabels" , m_bAnalyzeLabels ))
  {
    if (!m_bAnalyzeLabels)
      m_LastLabels.RemoveAll() ;
  }
  return true;
}

bool LabelFilter::ScanSettings(FXString& text)
{
  FXAutolock al( m_Lock ) ;
  text = _T("template(EditBox(Label)"
    ",ComboBox(Type(All(0),Container(1),Text(2),VFrame(3),Figure(4)"
    ",Quantity(5),Rect(6),Wave(7),Logical(8),Metafile(9)"
    ",UserData(10),Null(11),Control(12)))") ;
  text += _T(",ComboBox(Compare(") ;
  FXString Addition ;
  for ( int i = 0 ; i < ARRSZ(CompareModes) ; i++ )
  {
    Addition.Format( _T("%s(%d)%s") , CompareModes[i].ModeName , i , 
      ( i < ARRSZ(CompareModes) - 1 ) ? "," : "))" ) ;
    text += Addition ;
  }
  text += _T(",ComboBox(InverseRule(No(0),Yes(1)))") ;
  text += _T(",EditBox(Attributes)") ;
  if ( m_bAnalyzeLabels )
  {
    text += _T(",ComboBox(Available(") ;
    if (m_LastLabels.GetCount())
    {
      FXString Combo ;
      for (int i = 0 ; i < m_LastLabels.GetCount() ; i++)
      {
        FXString NextLabel ;
        NextLabel.Format( _T( "%s(%d)%s" ) , m_LastLabels[ i ] , i ,
          ( ( i + 1 ) < m_LastLabels.GetCount() ) ? _T( "," ) : _T( "" ) ) ;
        Combo += NextLabel ;
      }
      Combo += _T( "))" ) ;
      text += Combo ;
    }
    else
      text += "))" ;
  }
  text += _T( ",ComboBox(AnalyzeLabels(No(0),Yes(1)))" ) ;
  text += _T(")") ;

  return true;
}


bool    LabelFilter::CheckLabel(LPCTSTR label)
{
  if ( label && (label[0] != 0) ) //enum all labels
  {
    if ( m_bAnalyzeLabels )
    {
      int i = 0 ;
      for (  ; i < m_LastLabels.GetCount() ; i++ )
      {
        if ( m_LastLabels[i] == label )
          break ;
      }
      if ( i == m_LastLabels.GetCount() )
        m_LastLabels.Add(label) ;
    }
  }
  else
    return ( m_PatternLabels.GetCount() == 0 ) ;
  if ( m_PatternLabels.GetCount() == 0 )
    return true;
  for ( int i = 0 ; i < m_PatternLabels.GetCount() ; i++ )
  {
    LPCTSTR pSubstr = _tcsstr( label , m_PatternLabels[i] ) ;
    if ( !pSubstr )
      continue ;
    switch ( m_CompareMode )
    {
    case COMPARE_EXACT:
      return (m_PatternLabels[i] == label) ;
    case COMPARE_BEGIN:
      return ( pSubstr == label ) ;
    case COMPARE_ANY:
      return true ;
    case COMPARE_WITH_PREFIX:
      return ( pSubstr && (pSubstr > label)
        && (*(pSubstr - 1) == _T(':')) ) ;
    default:
      break ;
    }
  }
  return false;
}

void LabelFilter::InspectFrame(const CDataFrame* pDataFrame, LabelFilter_FramesSet& frames, bool AppendFrames)
{
  LPCTSTR Label = pDataFrame->GetLabel() ;
  bool bIsContainer = pDataFrame->IsContainer() ;
  bool bIsLabelMatching = CheckLabel( Label ) ;
  bool bIsTypeMatching = !bIsContainer && ((m_TypeForFiltering == 0)
    || (pDataFrame->GetDataType() == m_TypeForFiltering)) ;
  if ( bIsLabelMatching )
  {
    bool bMatch = bIsTypeMatching
      || ( (m_TypeForFiltering == -1)  && bIsContainer  ) ; // looking for container
    
    if ( !m_bInverseRule )
    {
      if ( bMatch )
        // and this into container
      {
        if ( pDataFrame->GetUserCnt() > 1 )
          frames.Add( pDataFrame->Copy() ) ;
        else
        {
          ((CDataFrame*) pDataFrame)->AddRef() ;
          frames.Add( (CDataFrame*) pDataFrame ) ;
        }
      }
      if ( !bIsContainer )
        return ;
    }
    if ( bMatch )
      return ;
  }
  else if ( m_bInverseRule )
  {
    if ( pDataFrame->GetUserCnt() > 1 )
      frames.Add( pDataFrame->Copy() ) ;
    else
    {
      ((CDataFrame*)pDataFrame)->AddRef() ;
      frames.Add( (CDataFrame*)pDataFrame ) ;
    }
    return ;
  }
  CFramesIterator* Iterator = pDataFrame->CreateFramesIterator( nulltype ) ;
  if ( Iterator )
  {
    CDataFrame * pNext = NULL ;
    while (pNext = Iterator->NextChild(NULL)) 
    {
      Label = pNext->GetLabel() ;
      if ( pNext->IsContainer() )
      {
        if ( m_TypeForFiltering == -1 )
        {
          if ( CheckLabel( Label ) )
          {
            if ( !m_bInverseRule )
            {
              if ( pNext->GetUserCnt() > 1 )
                frames.Add( pNext->Copy() ) ;
              else
              {
                pNext->AddRef();
                frames.Add( pNext ) ;
              }
              continue ;
            }
          }
        }
        InspectFrame( pNext , frames , AppendFrames );
        continue ;
      }
      bool bIsLabelMatching = CheckLabel( Label ) ;
      bool bIsTypeMatching = (m_TypeForFiltering == 0)
        || (pNext->GetDataType() == m_TypeForFiltering) ;
      if ( bIsTypeMatching )
      {
        if ( bIsLabelMatching )
        {
          if ( !m_bInverseRule )
          {
            if ( pNext->GetUserCnt() > 1 )
              frames.Add( pNext->Copy() ) ;
            else
            {
              pNext->AddRef();
              frames.Add( pNext ) ;
            }
            continue ;
          }
        }
      }
      if ( m_bInverseRule && ( !bIsLabelMatching || !bIsTypeMatching) )
      {
        if ( pNext->GetUserCnt() > 1 )
          frames.Add( pNext->Copy() ) ;
        else
        {
          pNext->AddRef();
          frames.Add( pNext ) ;
        }
      }
    }
    delete Iterator ;
  }
};

CDataFrame* LabelFilter::DoProcessing(const CDataFrame* pDataFrame)
{
  FXAutolock   al(m_Lock);
  CDataFrame * pData = NULL ;
  CDataFrame * pDescription = NULL ;

  m_LastLabels.RemoveAll();
  bool AppendFrames=true;
  LabelFilter_FramesSet frames;
  m_Protect.Lock() ;
  InspectFrame(pDataFrame, frames, AppendFrames);
  m_Protect.Unlock() ;
  if (frames.GetSize()==0)
    return NULL;
  if (frames.GetSize()==1)
  {
    if ( !m_Attributes.IsEmpty() )
    {
      if ( frames[ 0 ]->GetUserCnt() > 2 )
      {
        frames[ 0 ]->Release() ;
        frames[ 0 ] = frames[ 0 ]->Copy() ;
      }
      *(frames[ 0 ]->Attributes()) += m_Attributes ;
    }

    return (CDataFrame*) frames[ 0 ];
  }
  CContainerFrame* retV=CContainerFrame::Create();
  retV->CopyAttributes(pDataFrame);
  for (int i=0; i<frames.GetSize(); i++)
  {
    if ( !m_Attributes.IsEmpty() )
    {
      if ( frames[i]->GetUserCnt() > 2 )
      {
        frames[i]->Release() ;
        frames[i] = frames[i]->Copy() ;
      }
      *(frames[i]->Attributes()) += m_Attributes ;
    }
    retV->AddFrame(frames[i]);
  }
  return retV;
}
