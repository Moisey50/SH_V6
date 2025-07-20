// FieldChooser.h.h : Implementation of the FieldChooser class


#include "StdAfx.h"
#include "FieldChooser.h"


// Do replace "TODO_Unknown" to necessary gadget folder name in gadgets tree
IMPLEMENT_RUNTIME_GADGET_EX(FieldChooser, CFilterGadget, "Helpers", TVDB400_PLUGIN_NAME);

FieldChooser::FieldChooser(void)
{
  m_pInput = new CInputConnector( text );
  m_pOutput = new COutputConnector( figure * quantity );
  m_pDuplexConnector = new CDuplexConnector( this , text, text) ;
  m_OutputMode = modeReplace ;
  m_iNSamples = 100 ;
  m_iWorkingMode = 0 ; // full data set sending
  m_iLastSelectedField = -1 ;
  m_DescriptionPrefix = _T("ResultsOrder") ;
  m_Attributes = _T("MinMaxes=0,1000,0,750") ; // Xmin,Xmax,Ymin,Ymax
  m_bOneSample = false ;
  m_bXYLissajous = false ;
  m_iNAutomaticNamesForDeletion = 0 ;
  m_iNLastAutomaticNames = 0 ;
  
  Resume();
}

void FieldChooser::ShutDown()
{
  //TODO: Add all destruction code here
  CFilterGadget::ShutDown();
  delete m_pInput;
  m_pInput = NULL;
  delete m_pOutput;
  m_pOutput = NULL;
  delete m_pDuplexConnector ;
  m_pDuplexConnector = NULL ;
}

CDataFrame* FieldChooser::DoProcessing(const CDataFrame* pDataFrame)
{
  FXAutolock al( m_Lock ) ;
  CDataFrame * pOutFrame = NULL ;  
  CFramesIterator * Iterator = pDataFrame->CreateFramesIterator( text ) ;
  const CDataFrame * pNext = NULL ;
  const CTextFrame * pTxt = (Iterator) ? NULL : pDataFrame->GetTextFrame( DEFAULT_LABEL ) ;
  if ( Iterator )
  {
    pNext = Iterator->Next(NULL) ;
    if ( pNext )
      pTxt = pNext->GetTextFrame( DEFAULT_LABEL ) ;
  }
  else
    pTxt = pDataFrame->GetTextFrame( DEFAULT_LABEL ) ;

  if ( !pTxt )
    return NULL ;

  CContainerFrame * pOutputContainer = CContainerFrame::Create() ;
  int iNAdded = 0 ;
  FXDblArray LastValues ;
  FXStringArray LastLabels ;
  LastValues.SetSize( m_Fields.GetCount() ) ;
  LastLabels.SetSize( m_Fields.GetCount() ) ;
  while ( pTxt )
  {
    FXString Label = pTxt->GetLabel() ;
    if ( Label.Find( m_DescriptionPrefix ) >= 0 ) // description frame
    {
      ProcessDescriptionFrame( (LPCTSTR)pTxt->GetString() ) ;
    }
    else
    {
      bool bDataLabel = (Label.Find( _T("Data_") ) == 0) ;
      if ( bDataLabel ) // data frame and necessary to choose something
      {
        FXString Txt = pTxt->GetString() ;
        Txt.Trim( _T(" \t,;=()") ) ;
        FXDblArray NewValues ;
        FXSIZE iPos = 0 ;//Txt.ReverseFind( _T(':')) ;
        //       if ( iPos < 0 )
        //         iPos = 0 ;
        //       else
        //         iPos++ ;
        //       m_LastString = Txt.Mid(iPos) ;
        m_LastString = Txt ;
        FXString NextToken = Txt.Tokenize( _T(" \t=,;():\n") , iPos ) ;
        int iConvertedValues = 0 ;
        while ( 0 <= iPos  )
        {
          double dY ;
          if ( _stscanf( (LPCTSTR)NextToken , _T("%lg") , &dY ) )
          {
            NewValues.Add( dY ) ;
            iConvertedValues++ ;
          }
          else
            NewValues.Add( 1e300 ) ;

          NextToken = Txt.Tokenize( _T(" \t=(),;:") , iPos ) ;
        }
        // More than one of selected fields and graphs for deletion

        if ( ! m_bXYLissajous  ||  !m_Fields.GetCount() )
        {
          //         if ( m_Data.GetCount() < m_Fields.GetCount() )
          //           m_Data.SetSize( m_Fields.GetCount() ) ;
          if ( m_Fields.GetCount() )  // selection by field names
          {
            for ( int i = 0 ; i < m_Fields.GetCount() ; i++ )
            {
              int iIndex =  m_Fields[i] ;
              if ( iIndex >= NewValues.GetCount() )
                continue ;
              double dValue = ( iIndex >= 0 ) ? NewValues[iIndex] : 0. ;
              if ( iNAdded == 0 )
              {
                LastLabels[i] = Label ;
                LastValues[i] = dValue ;
              }
              else
              {
                if ( LastLabels[i] == Label )
                {
                  if ( LastValues[i] == dValue )
                    continue ;
                  else
                    LastValues[i] = dValue ;
                }
                else
                  LastLabels[i] = Label ;
              }
              CQuantityFrame * pNewQuantity = CQuantityFrame::Create( dValue ) ;
              if ( pNewQuantity )
              {
                pNewQuantity->ChangeId( pDataFrame->GetId() ) ;
                FXString Label( _T("Insert:") ) ;
                Label += m_SelectedFieldNames[i] ;
                pNewQuantity->SetLabel( Label ) ;
                if ( !m_Attributes.IsEmpty() )
                  *(pNewQuantity->Attributes()) = m_Attributes ;
                pOutputContainer->AddFrame( pNewQuantity ) ;
                iNAdded++ ;
              }
            }
            m_iNLastAutomaticNames = 0 ;
          }
          else
          {
            int iFieldCount = 0 ;
            int iNApproved = 0 ;
            for ( int i = 0 ; i < DEFAULT_N_FIELDS  &&  iFieldCount < NewValues.GetCount() ; 
              i++ , iFieldCount++ )
            {
              while ( NewValues[ iFieldCount ] > 1e299 )
              {
                if ( ++iFieldCount >= NewValues.GetCount() )
                  break ;
              }
              if ( iFieldCount >= NewValues.GetCount() )
                break ;
              CQuantityFrame * pNewQuantity = CQuantityFrame::Create(
                NewValues[iFieldCount]  ) ;
              if ( pNewQuantity )
              {
                pNewQuantity->ChangeId( pDataFrame->GetId() ) ;
                FXString Label ;
                Label.Format( _T("Insert:Unknown%d") , iFieldCount ) ;
                pNewQuantity->SetLabel( Label ) ;
                if ( !m_Attributes.IsEmpty() )
                  *(pNewQuantity->Attributes()) = m_Attributes ;
                pOutputContainer->AddFrame( pNewQuantity ) ;
                iNApproved++ ;
                iNAdded++ ;
              }
            }
            m_iNLastAutomaticNames = iNApproved ;
          }
        }
        else if ( m_Fields.GetCount() >= 2 )  // m_bXYLissajous == true
        {
          for ( int i = 0 ; i + 1 < m_Fields.GetCount() ; i += 2 )
          {
            CDPoint NewPt( NewValues[ (m_Fields[i] >= 0) ? m_Fields[i] : 0 ] , 
              NewValues[ (m_Fields[i+1] >= 0) ? m_Fields[i+1] : 0] ) ;

            CFigureFrame * pNewFig = CFigureFrame::Create() ;
            pNewFig->Add( NewPt ) ;
            FXString Label( _T("Insert:") ) ;
            Label.Format( _T("Insert:%s_vs_%s") , m_SelectedFieldNames[i+1] ,
              m_SelectedFieldNames[i] ) ;
            pNewFig->SetLabel( Label ) ;
            if ( !m_Attributes.IsEmpty() )
              *(pNewFig->Attributes()) = m_Attributes ;
            pOutputContainer->AddFrame( pNewFig ) ;
            iNAdded++ ;
          }
        }
      }
    }
    if ( Iterator )
    {
      pNext = Iterator->Next(NULL) ;
      pTxt = ( pNext ) ? pNext->GetTextFrame( DEFAULT_LABEL ) : NULL ;
    }
    else
      break ; // no additional frames, nothing to do
  }
  if ( Iterator )
    delete Iterator ;

  for ( int i = 0 ; i < m_NamesForDeletion.GetCount() ; i++ )
  {
    CQuantityFrame * pDummyQuantityForDeletion = CQuantityFrame::Create( 0. ) ;
    FXString Label( _T("Remove:") ) ;
    Label += m_NamesForDeletion[i] ;
    pDummyQuantityForDeletion->SetLabel( Label ) ;
    pOutputContainer->AddFrame( pDummyQuantityForDeletion ) ;
    iNAdded++ ;
 }
  for ( int i = 0 ; i < m_iNAutomaticNamesForDeletion ; i++ )
  {
    CQuantityFrame * pDummyQuantityForDeletion = CQuantityFrame::Create( 0. ) ;
    FXString Label ;
    Label.Format( _T("Remove:Unknown%d") , i ) ;
    pDummyQuantityForDeletion->SetLabel( Label ) ;
    pOutputContainer->AddFrame( pDummyQuantityForDeletion ) ;
    iNAdded++ ;
  }
  m_NamesForDeletion.RemoveAll() ;
  m_iNAutomaticNamesForDeletion = 0 ;

  if ( iNAdded )
  {
    pOutputContainer->CopyAttributes( pDataFrame ) ;
    return pOutputContainer ;
  }
  else 
  {
     pOutputContainer->Release( pOutputContainer ) ;
     return NULL ;
  }
}

bool FieldChooser::ScanProperties(LPCTSTR text, bool& Invalidate)
{
  FXAutolock al(m_Lock) ;
  FXPropertyKit pk(text);

  if ( pk.GetInt(_T("FieldNames") , m_iLastSelectedField ))
  {
    if (  (0 <= m_iLastSelectedField)  
      &&  (m_iLastSelectedField < m_AllLastFieldNames.GetCount()) )
    {
      // Necessary to replace status reading in future (for from UI disconnection)
      FXString Name = m_AllLastFieldNames[m_iLastSelectedField] ;
      if ( Name.IsEmpty() )
        Name.Format( _T("{%d}") , m_iLastSelectedField ) ;
      bool bRemove = (GetAsyncKeyState( VK_CONTROL ) & 0x8000) != 0 ;
      if ( ! bRemove ) // Add
      {
        m_SelectedNames += _T(' ') ;
        m_SelectedNames += Name ;
        m_Fields.Add( m_iLastSelectedField ) ;
        m_iLastSelectedField = -1 ;
      }
      else //Remove
      {
        FXSIZE iPos = m_SelectedNames.Find( Name ) ;
        if ( iPos >= 0 )
        {
          m_SelectedNames.Delete( iPos ,  Name.GetLength() + 1 ) ;
        }
      }
      BuildFieldNames() ;
      BuildIndexes() ;
      Invalidate = true ;
    }
    m_NamesForDeletion.Append( m_SelectedFieldNames ) ;
    m_iNAutomaticNamesForDeletion = m_iNLastAutomaticNames ;
  }
  int iNSamples = m_iNSamples ;
  int iNTokens = (int) m_Fields.GetCount() ;
  if ( pk.GetString( _T("Selected") , m_SelectedNames ) )
  {
    BuildFieldNames() ;
    BuildIndexes() ;
  }
  FXString sOneSample ;
  bool bNSamplesChanged = false ;
  if ( pk.GetString( _T("Length") , sOneSample ) )
  {
    if ( sOneSample == _T("auto") )
    {
      if ( !m_bOneSample )
      {
        m_bOneSample = true ;
        bNSamplesChanged = true ;

      }
    }
    else
    {
      if ( _stscanf( _T("%d") , (LPCTSTR)sOneSample , &iNSamples ) )
      {
        bNSamplesChanged = true ;
        m_bOneSample = ( iNSamples <= 1 ) ;
      }
    }
  }
//   if ( m_Fields.GetCount() && ((iNTokens != m_Fields.GetCount()) || bNSamplesChanged) )
//   {
//     m_Data.SetSize( m_Fields.GetCount() ) ;
//     for ( int i = 0 ; i < m_Fields.GetCount() ; i++ )
//     {
//       m_Data[i].SetSize( (m_bOneSample) ? 1 : iNSamples ) ;
//     }
//     m_iNSamples = iNSamples ;
//   }
  pk.GetInt( _T("XY_Lissajous") , m_bXYLissajous ) ;
  if ( m_Fields.GetCount() < 2 )
    m_bXYLissajous = FALSE ;
  pk.GetString( _T("FlagWord") , m_DescriptionPrefix ) ;
  if ( pk.GetString( _T("Attributes") , m_Attributes ) )
  {
    m_NamesForDeletion.Append( m_SelectedFieldNames ) ;
    m_iNAutomaticNamesForDeletion = m_iNLastAutomaticNames ;
  }

  return true;
}

bool FieldChooser::PrintProperties(FXString& text)
{
  FXPropertyKit pk;
  CFilterGadget::PrintProperties(text);
  FXAutolock al(m_Lock) ;
  pk.WriteString( _T("Selected") , m_SelectedNames ) ;
  if ( !m_bOneSample )
    pk.WriteInt( _T("Length") , m_iNSamples ) ;
  else
    pk.WriteString( _T("Length") , _T("auto") ) ;
  FXString LastStringShow(m_LastString) ;
  LastStringShow.Replace( _T("\n") , _T("\\n")) ;
  pk.WriteString( _T("LastData") , LastStringShow ) ;
  pk.WriteInt( _T("FieldNames") , m_iLastSelectedField ) ;
  pk.WriteInt( _T("XY_Lissajous") , m_bXYLissajous ) ;
  pk.WriteString( _T("FlagWord") , m_DescriptionPrefix ) ;
  pk.WriteString( _T("Attributes") , m_Attributes ) ;
  text+=pk;
  return true;
}

bool FieldChooser::ScanSettings(FXString& text)
{
  text = _T("template("
    "EditBox(Selected)"
//     ",Spin&Bool(Length,2,400)"
    ",EditBox(LastData)" 
    ",EditBox(FlagWord)"
    ",EditBox(Attributes)"
    ",ComboBox(FieldNames(" ) ;
  FXAutolock al(m_Lock) ;
  if ( m_AllLastFieldNames.GetCount() )
  {
    FXString Combo ;
    for ( int i = 0 ; i < m_AllLastFieldNames.GetCount() ; i++ )
    {
      FXString NextLabel ;
      NextLabel.Format( _T("%s(%d)%s") , m_AllLastFieldNames[i] , i ,
        ((i + 1) < m_AllLastFieldNames.GetCount()) ? _T(",") : _T("") ) ;
      Combo += NextLabel ;
    }
    text += Combo ;
  }
  text += _T("))") ;
  if ( m_Fields.GetCount() >= 2 )
  {
    text += _T(",ComboBox(XY_Lissajous(No(0),Yes(1)))") ;
  }
  text += _T(")") ;
  return true;
}

int FieldChooser::GetInputsCount()
{  
  return CFilterGadget::GetInputsCount();
}

CInputConnector* FieldChooser::GetInputConnector(int n)
{
  return CFilterGadget::GetInputConnector(n);
}

int FieldChooser::GetOutputsCount()
{
  return CFilterGadget::GetOutputsCount();
}

COutputConnector* FieldChooser::GetOutputConnector(int n)
{
   return CFilterGadget::GetOutputConnector(n);
}

int FieldChooser::GetDuplexCount()
{
  return 1 ;
}

CDuplexConnector* FieldChooser::GetDuplexConnector(int n)
{
  return ( n == 0 ) ? m_pDuplexConnector : NULL ;
}

void FieldChooser::ProcessDescriptionFrame( LPCTSTR pDescription )
{
  FXString VariableNames(pDescription);
  int iNamesLen = (int) VariableNames.GetLength() ;
  int iPrevPos = 0 ;
  int iPos = (int) VariableNames.FindOneOf(",;") ;
  if ( iPos >= 0 )
    m_AllLastFieldNames.RemoveAll() ;
  int iToken = 0 ;
  while ( iPos < iNamesLen )
  {
    FXString Token ;
    if ( iPos - iPrevPos <= 1 ) 
    {
      Token.Format( "{%d}" , iToken ) ;
    }
    else
      Token = VariableNames.Mid( iPrevPos + 1 , iPos - iPrevPos - 1 ) ;
    Token.Trim( " \t" ) ;
    m_AllLastFieldNames.Add( Token ) ;
    if ( VariableNames[iPos] == _T(';') )
      break ;
    iPrevPos = iPos ;
    while ( ++iPos < iNamesLen ) 
    {
      TCHAR Char = VariableNames[iPos] ;
      if ( Char == _T(',') || Char == _T(';') )
        break ;
    }
    iToken++ ;
  }
  BuildIndexes() ;
}


void FieldChooser::AsyncTransaction(
CDuplexConnector* pConnector, CDataFrame* pParamFrame)
{
  if ( pParamFrame->IsRegistered() )
    return ;
  CTextFrame * pTxt = pParamFrame->GetTextFrame() ;
  if ( !pTxt )
    return ;
  FXString Label = pTxt->GetLabel() ;
  if ( Label.Find( m_DescriptionPrefix ) >= 0 ) // description frame
  {
    FXAutolock al(m_Lock) ;
    ProcessDescriptionFrame( (LPCTSTR)pTxt->GetString() ) ;
  }
  pParamFrame->Release( pParamFrame ) ;
}

int FieldChooser::BuildIndexes(void)
{
  m_Fields.RemoveAll() ;
  for ( int i = 0 ; i < m_SelectedFieldNames.GetCount() ; i++ )
  {
    FXString Name = m_SelectedFieldNames[i] ;
    int j = 0 ;
    if ( Name[0] == _T('{') && _istdigit(Name[1]) ) // selection by number
      m_Fields.Add( atoi( (LPCTSTR)Name + 1 ) ) ;
    else
    {
      for ( ; j < m_AllLastFieldNames.GetCount() ; j++ )
      {
        if ( m_AllLastFieldNames[j] == Name )
        {
          m_Fields.Add( j ) ;
          break ;
        }
      }
      if ( j >= m_AllLastFieldNames.GetCount() ) // name is not found
        m_Fields.Add(-1) ;            // mark as -1 for zero showing          
    }
  }
  return 0;
}

int FieldChooser::BuildFieldNames(void)
{
//   FXStringArray SavedFieldNames ;
//   SavedFieldNames.Append( m_SelectedFieldNames ) ; 
  FXSIZE iPos = 0 ;
  m_NamesForDeletion.Append( m_SelectedFieldNames ) ;
  m_SelectedFieldNames.RemoveAll() ;
  while ( iPos >= 0 )
  {
    FXString Token = m_SelectedNames.Tokenize(_T(" \t;,=") , iPos ) ;
    if ( Token.GetLength() )
      m_SelectedFieldNames.Add( Token ) ;
  }
//   for ( int i = 0 ; i < SavedFieldNames.GetCount() ; i++ )
//   {
//     if ( i >= m_SelectedFieldNames.GetCount() )
//       break ;
//     if ( SavedFieldNames[i] != m_SelectedFieldNames[i] )
//       m_NamesForDeletion.Add( SavedFieldNames[i] ) ;
//   }

  return (int) m_SelectedFieldNames.GetCount() ;
}
