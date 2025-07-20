// MultiAver.h.h : Implementation of the MultiAver class


#include "StdAfx.h"
#include "MultiAver.h"


USER_FILTER_RUNTIME_GADGET(MultiAver,"Statistics");

static COLORREF GraphColors[] =
{
  RGB( 255 , 0 , 0 ) ,
  RGB( 0 , 255 , 0 ) ,
  RGB( 0 , 0 , 255 ) ,
  RGB( 255 , 255 , 255 ) ,
  RGB( 255 , 255 , 0 ) ,
  RGB( 0 , 255 , 255 ) ,
  RGB( 255 , 0 , 255 ) ,
  RGB( 192 , 64 , 0 ) ,
  RGB( 0 , 192 , 64 ) ,
  RGB( 64 , 0 , 192 ) ,
  RGB( 255 , 128 , 128 ) ,
  RGB( 128 , 255 , 128 ) ,
  RGB( 128 , 128 , 255 ) ,
  RGB( 128 , 128 , 128 )
} ;


LPCTSTR GetWorkingModeName( MA_WorkingMode mode )
{
  switch ( mode )
  {
    case MA_WM_Teaching: return _T( "Teaching" ) ;
    case MA_WM_Average: return _T( "Average" ) ;
  }
  return _T( "Unknown Mode" ) ;
}

MultiAver::MultiAver()
{
  m_WorkingMode = MA_WM_Average ;
  m_GadgetInfo = _T( "MultiAver" ) ;
  m_iIterator = 0 ;
  m_iRadius = 10 ;
  m_bAreasChanged = false ;
  m_iAveraging = 30 ;
  m_dBaseAverage = m_dLastBase = 0. ;
  m_dK = 0. ;
  m_dOffset = 0. ;
	init();
}

CDataFrame* MultiAver::DoProcessing(const CDataFrame* pDataFrame) 
{
  bool bResetGraphs = m_bAreasChanged ;
  if ( m_bAreasChanged )
  {
    FXAutolock al( m_ParametersProtect ) ;
    //m_Areas.RemoveAll();
    //m_Areas.Copy(m_AreasForGUI);
    m_bAreasChanged = false ;
    PropertiesReregistration() ;
  }
  LPCTSTR pLabel = pDataFrame->GetLabel();
  if ( pDataFrame->GetDataType() == text )
  {
    if ( pLabel && (*pLabel) )
    {
      if ( _tcscmp( pLabel , "LoadConfig" ) == 0 )
      {
        FXString FileName = pDataFrame->GetTextFrame()->GetString() ;
        LoadConfigParameters( FileName ) ;
        return NULL ;
      }
      else if ( _tcscmp( pLabel , "SaveConfig" ) == 0 )
      {
        FXString FileName = pDataFrame->GetTextFrame()->GetString() ;
        SaveConfigParameters( FileName ) ;
        return NULL ;
      }
    }
  }

	if ((pDataFrame) && (!Tvdb400_IsEOS(pDataFrame)))
	{
    if ( m_WorkingMode == MA_WM_Teaching )
    {
      const CTextFrame * ptf = pDataFrame->GetTextFrame( _T("Click") );
      if ( ptf )
      {
        FXPropertyKit pk( ptf->GetString() ) ;
        int x , y , Keys ;
        if (pk.GetInt(_T("x"), x) && pk.GetInt(_T("y"), y))
        {
          FXAutolock al(m_ParametersProtect);
          CPoint Cent( x , y ) ;
          // Check for available areas overlap
          bool bOverlap = false ;
          int i = 0 ;
          for (; i < m_AreasForGUI.GetCount(); i++)
          {
            CSize Dist = m_AreasForGUI[i].m_Rect.CenterPoint() - Cent;
            CSize Tol(m_AreasForGUI[i].m_Rect.Size());
            if ( (abs(Dist.cx) * 2 < Tol.cx )  && (abs( Dist.cy )* 2 < Tol.cy) )
            {
              bOverlap = true ;
              break ;
            }
          }
          if (!pk.GetInt(_T("Keys"), Keys))
          {
            int bKbLCntrlPressed = (GetAsyncKeyState(VK_LCONTROL) & 0x8000) != 0;
            int bKbRCntrlPressed = (GetAsyncKeyState(VK_RCONTROL) & 0x8000) != 0;
            int bKbCntrlPressed = bKbLCntrlPressed || bKbRCntrlPressed;
            int bKbLShiftPressed = (GetAsyncKeyState(VK_LSHIFT) & 0x8000) != 0;
            int bKbRShiftPressed = (GetAsyncKeyState(VK_RSHIFT) & 0x8000) != 0;
            int bKbShiftPressed = bKbLShiftPressed || bKbRShiftPressed;
            int bMousLeftPressed = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
            Keys = bKbCntrlPressed + (bKbShiftPressed << 1)
              + (bKbLCntrlPressed << 2) + (bKbRCntrlPressed << 3)
              + (bKbLShiftPressed << 4) + (bKbRShiftPressed << 5);
          }
          if ( bOverlap )
          {
            if ( Keys & 1 ) // Ctrl pressed
            {
              m_AreasForGUI.RemoveAt(i);
            }
          }
          else if ( Keys & 2 ) // shift is pressed
          {
            // New Area
            CRect r( x - m_iRadius , y - m_iRadius , x + m_iRadius , y + m_iRadius ) ;
            if ( r.left < 0 )
              r.left = 0 ;
            if ( r.top < 0 )
              r.top = 0 ;

            FXString Name ;
            int i = 1 ;
            int j = 0 ;
            Name.Format( _T( "Area%d" ) , i ) ;
            for ( ; j < m_AreasForGUI.GetCount() ; j++ )
            {
              if ( Name.MakeUpper() == m_AreasForGUI[ j ].m_ObjectName.MakeUpper() )
              {
                Name.Format( _T( "Area%d" ) , ++i ) ;
                j = -1 ;
                continue ;
              }
            }
            COLORREF Color = GraphColors[ ( i ) % ARRSZ( GraphColors ) ] ;
            NamedRectangle NewArea( Name , &r , Color ) ;
            m_AreasForGUI.Add( NewArea ) ;
            m_bAreasChanged = true ;
            SEND_GADGET_INFO( "New Area Added: %s" , (LPCTSTR)NewArea.m_AsString ) ;
          }
        }
      }
    }
    
    //	Get Picture Data from Input Package
		const CVideoFrame* pInputPictureData = pDataFrame->GetVideoFrame();
    if ( !pInputPictureData )
      return NULL ;
		//	Get Picture Description
		LPBITMAPINFOHEADER pInputInfoHeader = pInputPictureData->lpBMIH;
		DWORD dwCompression = pInputInfoHeader->biCompression ;
		
    int iBits = 0 ;
    switch ( dwCompression )
    {
      case BI_Y8:
      case BI_Y800:
      case BI_YUV9:
      case BI_YUV12: iBits = 8 ; break ;
      case BI_Y16:   iBits = 16 ; break ;
    }

    FXAutolock al(m_ParametersProtect);
    if (!iBits || !m_AreasForGUI.GetCount())
    {
      ( (CVideoFrame*) pInputPictureData )->AddRef() ;
      return (CDataFrame*) pInputPictureData ;
    }

    CContainerFrame * pOut = CContainerFrame::Create() ;
    CopyIdAndTime( pOut , pDataFrame ) ;
    pOut->AddFrame( pDataFrame ) ;
    CContainerFrame * pResultOut = CContainerFrame::Create() ;
    CopyIdAndTime( pResultOut , pDataFrame ) ;


    //	Get Picture Width
    LONG imgWidth = pInputInfoHeader->biWidth;
    //	Get Picture Height
    LONG imgHeight = pInputInfoHeader->biHeight;
    LPBYTE pImg = GetData( pInputPictureData ) ;
    LONG imgSize = imgHeight * imgWidth ; // doesn't process first and last row
    m_dLastBase = 0. ;
    m_dLastCorrection = 0. ;
    m_dLastWhiteValue = 0. ;

    for (int i = 0; i < m_AreasForGUI.GetCount(); i++)
    {
      NamedRectangle& Area = m_AreasForGUI[i];
      int iLen = Area.m_Rect.Width() + 1 ;
      if ( Area.m_Rect.left + iLen >= imgWidth )
        iLen = imgWidth - Area.m_Rect.left ;
      int iHeight = Area.m_Rect.Height() + 1 ;
      if ( Area.m_Rect.top + iHeight >= imgHeight )
        iHeight = imgHeight - Area.m_Rect.top ;
      int iYEnd = Area.m_Rect.top + iHeight ;
            
      int iSum = 0 ;
      if ( iBits == 8 )
      {
        LPBYTE pBegin = pImg + imgWidth * Area.m_Rect.top + Area.m_Rect.left ;
        for ( int iY = Area.m_Rect.top ; iY < iYEnd ; iY++ )
        {
          LPBYTE p = pBegin ;
          LPBYTE pEnd = pBegin + iLen ;
          do
          {
            iSum += *( p++ ) ;
          } while ( p < pEnd );
          pBegin += imgWidth ;
        }
      }
      else
      {
        LPWORD pImg16 = ( LPWORD ) pImg ;
        LPWORD pBegin = pImg16 + imgWidth * Area.m_Rect.top + Area.m_Rect.left ;
        for ( int iY = Area.m_Rect.top ; iY < iYEnd ; iY++ )
        {
          LPWORD p = pBegin ;
          LPWORD pEnd = pBegin + iLen ;
          do
          {
            iSum += *( p++ ) ;
          } while ( p < pEnd );
          pBegin += imgWidth ;
        }
      }
      Area.m_dLastValue = ( double ) iSum / (double)( iLen*iHeight );
      if ( Area.m_ObjectName.MakeUpper() == _T( "BASE" ) )
      {
        m_dLastBase = Area.m_dLastValue ;
        if ( ++m_iNAccumulated >= m_iAveraging )
          m_iNAccumulated = m_iAveraging ;
        m_dBaseAverage = m_dBaseAverage * ( m_iNAccumulated - 1. ) / m_iNAccumulated ;
        m_dBaseAverage += m_dLastBase / m_iNAccumulated ;
        m_dLastCorrection = m_dLastBase - m_dBaseAverage ; // additive error calculation
        CRectFrame * pBaseRectData = CRectFrame::Create( &Area.m_Rect ) ;
        pBaseRectData->SetLabel( _T( "BASE" ) ) ;
        *(pBaseRectData->Attributes()) = Area.ToString() ;
        pBaseRectData->SetTime( (GetGraphTime() * 1.e-3)) ;
        pResultOut->AddFrame( pBaseRectData ) ;
      }
      else if ( Area.m_ObjectName.MakeUpper() == _T( "WBASE" ) )
      {
        m_dLastWhiteValue = Area.m_dLastValue ;
        CRectFrame * pBaseRectData = CRectFrame::Create( &Area.m_Rect ) ;
        pBaseRectData->SetLabel( _T( "WBASE" ) ) ;
        *( pBaseRectData->Attributes() ) = Area.ToString() ;
        pBaseRectData->SetTime( (GetGraphTime() * 1.e-3)) ;
        pResultOut->AddFrame( pBaseRectData ) ;
      }
    }
    double dDiff = ( m_dLastBase - m_dOffset ) ;
    bool bEnoughDiff = (m_dLastBase > 0.)  && (( fabs( dDiff ) / m_dLastBase ) > 0.1) ;
    double dAmplWtoB = m_dLastWhiteValue - m_dLastBase ;
    for (int i = 0; i < m_AreasForGUI.GetCount(); i++)
    {
      NamedRectangle& Area = m_AreasForGUI[i];
      double dValue = Area.m_dLastValue ;
      bool bBases = false;
      if ( m_dLastWhiteValue != 0. && dAmplWtoB > 0. )
      {
        if ((Area.m_ObjectName.MakeUpper() == _T("BASE")))
        {
          bBases = true;
        }
        else if ((Area.m_ObjectName.MakeUpper() == _T("WBASE")))
        {
          bBases = true;
        }
        else
          dValue = (m_dK * (dValue - m_dLastBase) / dAmplWtoB) + m_dOffset;


      }
      else
      {
      if ( bEnoughDiff )
        dValue -= (m_dK * m_dLastCorrection * ( Area.m_dLastValue - m_dOffset ) / dDiff) ;
      }
      FXString PtText ;
      PtText.Format( _T( "%s=%.1f" ) , 
        ( LPCTSTR ) Area.m_ObjectName , dValue ) ;
      CTextFrame * pLabel = CTextFrame::Create( PtText ) ;
      pLabel->Attributes()->Format( _T( "x=%d;y=%d;color=0x%06x;" ) ,
        Area.m_Rect.right + 3 , Area.m_Rect.top + 3 , Area.m_Color ) ;
      CopyIdAndTime( pLabel , pOut ) ;
      pOut->AddFrame( pLabel ) ;
      CRectFrame * pAverAreaView = CRectFrame::Create( &Area.m_Rect ) ;
      pAverAreaView->Attributes()->Format( _T( "color=0x%06X;" ) , Area.m_Color ) ;
      CopyIdAndTime( pAverAreaView , pOut ) ;
      pAverAreaView->SetLabel( Area.m_ObjectName ) ;
      pOut->AddFrame( pAverAreaView ) ;

      CQuantityFrame * pOutValue = CQuantityFrame::Create( dValue ) ;
      FXString ValueLabel ;
      ValueLabel.Format( ( bResetGraphs ) ? _T( "RemoveAll:%s" ) : _T( "Append:%s" ) ,
        ( LPCTSTR ) Area.m_ObjectName ) ;
      bResetGraphs = false ;
      pOutValue->SetLabel( ValueLabel ) ;
      pOutValue->Attributes()->Format( _T( "color=0x%06X;" ) , Area.m_Color ) ;
      CopyIdAndTime( pOutValue , pOut ) ;
      pResultOut->AddFrame( pOutValue ) ;
      if ( !bBases )
      {
        pOutValue->AddRef();
        pOut->AddFrame(pOutValue);
      }
    }
    if ( pResultOut->GetFramesCount() )
    {
      FXString ResultLabel(_T("Result_"));
      FXString Label = pInputPictureData->GetLabel();
      ResultLabel += Label.Trim() ;
      pResultOut->SetLabel( ResultLabel );
//      pResultOut->AddRef() ;
//       pOut->AddFrame( pResultOut ) ;
      COutputConnector * pDataAndAreasPin = GetOutputConnector( 1 ) ;
      PutFrame( pDataAndAreasPin , pResultOut ) ;
    }
		return  pOut;
	}
	return NULL ;
}

void MultiAver::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame) 
{
  if ( !pParamFrame )
    return;

  CTextFrame* tf = pParamFrame->GetTextFrame( DEFAULT_LABEL );
  if ( tf )
  {

  }
  pParamFrame->Release( pParamFrame );
};

bool MultiAver::ScanSettings( FXString& Settings )
{
  Settings = _T( "template("
    "ComboBox(WorkingMode(Unknown(0),Teaching(1),Measurement(2)))"
    ",Spin(Radius,0,100)"
    ",EditBox(Delete)"
    ",EditBox(K)"
    ",EditBox(Offset)"
    ",Spin(Average,1,1000)" ) ;
    for ( int i = 0 ; i < m_AreasForGUI.GetCount() ; i++ )
    {
      FXString DlgFieldName ;
      DlgFieldName.Format( _T( ",EditBox(Area%d)" ) , i + 1 ) ;
      Settings += DlgFieldName ;
    }
    Settings += _T(")") ;
  return true;
}
void MultiAver::PropertiesRegistration() 
{
  addProperty( SProperty::COMBO , _T( "WorkingMode" ) , &m_WorkingMode ,
    SProperty::Int , _T( "Unknown;Teaching;Measurement;" ) ) ;
  addProperty( SProperty::SPIN , _T( "Radius" ) , &m_iRadius ,
    SProperty::Int , 0 , 100 ) ;
  if ( m_AreasForGUI.GetCount() )
  {
    addProperty( SProperty::EDITBOX , _T( "Delete" ) , &m_DeleteAsString , SProperty::String ) ;
    SetChangeNotification( _T( "Delete" ) , ConfigParamChange , this ) ;

    for ( int i = 0 ; i < m_AreasForGUI.GetCount() ; i++ )
    {
      FXString DlgFieldName ;
      DlgFieldName.Format( _T( "Area%d" ) , i + 1 ) ;
      addProperty( SProperty::EDITBOX , DlgFieldName ,
        &m_AreasForGUI[ i ].m_AsString , SProperty::String ) ;
    }
  }
};

bool MultiAver::PrintProperties( FXString& txt )
{
  CleanDoubledAreas() ;
  FXAutolock al( m_ParametersProtect ) ;
  FXPropertyKit pk ;
  CFilterGadget::PrintProperties( pk ) ;
  pk.WriteInt( _T( "WorkingMode" ) , (int)m_WorkingMode ) ;
  pk.WriteInt( _T( "Radius" ) , m_iRadius ) ;
  pk.WriteInt( _T( "NAreas" ) , (int)m_AreasForGUI.GetCount() ) ;
  pk.WriteString( _T( "Delete" ) , m_DeleteAsString ) ;
  pk.WriteDouble( _T( "K" ) , m_dK ) ;
  pk.WriteDouble( _T( "Offset" ) , m_dOffset ) ;
  pk.WriteInt( _T( "Average" ) , m_iAveraging ) ;
  if ( m_AreasForGUI.GetCount() )
  {
    for ( int i = 0 ; i < m_AreasForGUI.GetCount() ; i++ )
    {
      FXString ItemName ;
      ItemName.Format( _T( "Area%d" ) , i + 1) ;
      pk.WriteString( ItemName , m_AreasForGUI[ i ].ToString() ) ;
    }
  }
  txt = pk ;
  return true ;
}
bool MultiAver::PrintOwnProperties( FXString& txt )
{
  CleanDoubledAreas() ;
  FXAutolock al( m_ParametersProtect ) ;
  FXPropertyKit pk ;
  pk.WriteInt( _T( "WorkingMode" ) , (int) m_WorkingMode ) ;
  pk += _T( "\r\n" ) ;
  pk.WriteInt( _T( "Radius" ) , m_iRadius ) ;
  pk += _T( "\r\n" ) ;
  pk.WriteInt( _T( "NAreas" ) , (int)m_AreasForGUI.GetCount() ) ;
  pk += _T( "\r\n" ) ;
  pk.WriteString( _T( "Delete" ) , m_DeleteAsString ) ;
  pk += _T( "\r\n" ) ;
  pk.WriteDouble( _T( "K" ) , m_dK ) ;
  pk += _T( "\r\n" ) ;
  pk.WriteDouble( _T( "Offset" ) , m_dOffset ) ;
  pk += _T( "\r\n" ) ;
  pk.WriteInt( _T( "Average" ) , m_iAveraging ) ;
  pk += _T( "\r\n" ) ;
  if ( m_AreasForGUI.GetCount() )
  {
    for ( int i = 0 ; i < m_AreasForGUI.GetCount() ; i++ )
    {
      FXString ItemName ;
      ItemName.Format( _T( "Area%d" ) , i + 1 ) ;
      pk.WriteString( ItemName , m_AreasForGUI[ i ].ToString() ) ;
      pk += _T( "\r\n" ) ;
    }
  }
  txt = pk ;
  return true ;
}

bool MultiAver::ScanProperties( LPCTSTR text , bool& bInvalidate )
{

  {
  FXAutolock al( m_ParametersProtect ) ;
    FXPropKit2 pk( text ) ;
  CFilterGadget::PrintProperties( pk ) ;
  pk.GetInt( _T( "WorkingMode" ) , *((int*)&(m_WorkingMode)) ) ;
  pk.GetInt( _T( "Radius" ) , m_iRadius ) ;
  bool bRes = pk.GetInt( _T( "Average" ) , m_iAveraging ) ;
  bRes |= pk.GetDouble( _T( "K" ) , m_dK ) ;
  bRes |= pk.GetDouble( _T( "Offset" ) , m_dOffset ) ;
  if ( bRes  )    
  {
    m_iNAccumulated = 0 ;
    m_dBaseAverage = 0. ;
  }
  if ( pk.GetString( _T( "Delete" ) , m_DeleteAsString )
    && !m_DeleteAsString.IsEmpty() )
  {
    FXString DeleteArea = m_DeleteAsString ;
    m_DeleteAsString.Empty() ;
    DeleteArea.Trim() ;
    if ( DeleteArea == _T( "All" ) )
    {
      m_AreasForGUI.RemoveAll() ;
      m_bAreasChanged = true ;
      bInvalidate = true ;
    }
    else
    {
      if ( isdecimal( DeleteArea ) )
      {
        int iNum = atoi( DeleteArea ) ;
        if ( iNum < m_AreasForGUI.GetCount() )
          m_AreasForGUI.RemoveAt( iNum ) ;
      }
      else 
      {
        for ( int i = 0 ; i < m_AreasForGUI.GetCount() ; i++ )
        {
          if ( m_AreasForGUI[i].m_ObjectName == DeleteArea )
          {
            m_AreasForGUI.RemoveAt( i ) ;
            m_bAreasChanged = true ;
            bInvalidate = true ;
            break ;
          }
        }
      }
    }
  }
  
  int iNAreas = 0 ;
  if ( pk.GetInt( _T( "NAreas" ) , iNAreas ) &&  iNAreas )
  {
    for ( int i = 0 ; i < iNAreas ; i++ )
    {
      FXString ItemName ;
      ItemName.Format( _T( "Area%d" ) , i + 1 ) ;
      FXString AreaContent ;
        if ( pk.GetStringWithBrackets( ItemName , AreaContent ) )
      {
        NamedRectangle NewRect;
        if ( NewRect.ScanProperties( AreaContent ) )
        {
          int i = 0 ;
          for ( ; i < m_AreasForGUI.GetCount() ; i++ )
          {
            if ( m_AreasForGUI[i].m_ObjectName == NewRect.m_ObjectName )
              m_AreasForGUI.SetAt( i , NewRect ) ;
          }
            if ( i >= m_AreasForGUI.GetCount() )
            m_AreasForGUI.Add( NewRect );
        }
        m_bAreasChanged = true ;
        bInvalidate = true ;
      }
    }
  }
  else
  {
    for ( int i = 0 ; i < m_AreasForGUI.GetCount() ; i++ )
    {
      FXString ItemName ;
      ItemName.Format( _T( "Area%d" ) , i + 1 ) ;
      FXString Content ;
      if ( pk.GetString( ItemName , Content ) )
      {
        m_AreasForGUI[ i ].ScanProperties( Content ) ;
        m_bAreasChanged = true ;
        bInvalidate = true ;
      }
    }
  }
  }
  CleanDoubledAreas() ;
  return true ;
}

void MultiAver::ConnectorsRegistration() 
{
	addInputConnector( transparent , "ImgInput");
	addOutputConnector(	transparent, "Result");
  addOutputConnector( transparent , "SpectDataAndAreas" ) ;
};

void MultiAver::ConfigParamChange( LPCTSTR pName , void* pThisGadget , bool& bInvalidate , bool& bRescanProperties)
{
  MultiAver * pGadget = ( MultiAver* ) pThisGadget ;
  if ( pGadget )
  {
    FXString DeleteArea ;
    FXAutolock al( pGadget->m_ParametersProtect ) ;
    if ( !_tcscmp( _T( "Delete" ) , pName ) )
    {
      DeleteArea = pGadget->m_DeleteAsString ;
      pGadget->m_DeleteAsString.Empty() ;
      DeleteArea = DeleteArea.Trim() ;
      if (DeleteArea == _T( "All" ))
      {
        pGadget->m_AreasForGUI.RemoveAll() ;
        pGadget->m_bAreasChanged = true ;
        bInvalidate = true ;
      }
      else
      {
        if ( isdecimal( DeleteArea ) )
        {
          int iNum = atoi( DeleteArea ) ;
          if ( iNum < pGadget->m_AreasForGUI.GetCount() )
            pGadget->m_AreasForGUI.RemoveAt( iNum ) ;
        }
        else
        {
          for ( int i = 0 ; i < pGadget->m_AreasForGUI.GetCount() ; i++ )
          {
            if ( pGadget->m_AreasForGUI[ i ].m_ObjectName == DeleteArea )
            {
              pGadget->m_AreasForGUI.RemoveAt( i ) ;
              pGadget->m_bAreasChanged = true ;
              bInvalidate = true ;
              break ;
            }
          }
        }
      }
      return ;
    }
  }
}
bool MultiAver::SaveConfigParameters( LPCTSTR pFileName ) 
{
  CFileException ex ;
  TCHAR ErrBuff[ 1000 ] ;
  FXString ConfigFileName( pFileName );
  CFile ConfigFile;
  FXPropKit2 pk;
  GetGadgetName( m_GadgetInfo ) ;
  CleanDoubledAreas() ;
  if ( ConfigFile.Open( pFileName , CFile::modeWrite | CFile::modeCreate , &ex ) )
  {
    PrintOwnProperties( pk ) ;
    FXString Unreg = ::FxUnregularize( pk ) ;
    try
    {
      ConfigFile.Write( (LPCTSTR) Unreg , sizeof(TCHAR) * ((int)Unreg.GetLength() + 1) ) ;
      ConfigFile.Close() ;
      SEND_GADGET_INFO( _T( "Config Params Written: File %s, Working mode %s, size=%d" ) ,
        pFileName , GetWorkingModeName( m_WorkingMode ) ,
        sizeof( TCHAR ) * (pk.GetLength() + 1) ) ;
      return true ;
    }
    catch ( CFileException * exw )
    {
      if ( exw->GetErrorMessage( ErrBuff , sizeof( ErrBuff ) ) )
        SEND_GADGET_ERR( _T( "Error %s for writing to file %s" ) , ErrBuff , pFileName ) ;
      else
        SEND_GADGET_ERR( _T( "Unknown Error for writing to file %s" ) , pFileName ) ;
    }
  }
  else
  {
    if ( ex.GetErrorMessage( ErrBuff , sizeof( ErrBuff ) ) )
      SEND_GADGET_ERR( _T( "Error %s for Save Config file %s opening" ) , ErrBuff , pFileName ) ;
    else
      SEND_GADGET_ERR( _T( "Unknown Error for Save Config file %s opening" ) , pFileName ) ;

  }
  return false ;
}

bool MultiAver::LoadConfigParameters( LPCTSTR pFileName )
{
  CFileException ex ;
  TCHAR ErrBuff[ 1000 ] ;
  FXString ConfigFileName( pFileName );
  CFile ConfigFile;
  FXPropKit2 pk;
  GetGadgetName( m_GadgetInfo ) ;
  if ( ConfigFile.Open( pFileName , CFile::modeRead , &ex ) )
  {
    DWORD dwFLen = (DWORD) ConfigFile.GetLength() ;
    char * Buf = new TCHAR[ dwFLen + 5 ] ;
    UINT uiLen = ConfigFile.Read( Buf , dwFLen );
    ConfigFile.Close();
    if ( uiLen && uiLen <= dwFLen ) // is length and no too much
    {
      Buf[ uiLen ] = 0;
      pk += Buf;
    }
    else
    {
      SEND_GADGET_ERR( "ERROR Config loading: short file %s (L=%d)" ,
        pFileName , uiLen ) ;
      return false ;
    }
    bool bInvalidate = false ;

    ScanProperties( _T( "Delete=All;" ) , bInvalidate ) ;
    bool bRes = ScanProperties( pk , bInvalidate ) ;
    if ( bRes )
    {
      FXString Msg ;
      FXString ObjectNames ;
      for ( int i = 0; i < m_AreasForGUI.GetCount(); i++ )
      {
        ObjectNames += m_AreasForGUI[ i ].m_ObjectName ;
        if ( i < m_AreasForGUI.GetCount() - 1 )
          ObjectNames += _T( ',' ) ;
      }
      CleanDoubledAreas() ;
      SEND_GADGET_INFO( "Spectrum Config is OK, %d objects are %s" ,
        m_AreasForGUI.GetCount() , (LPCTSTR) ObjectNames ) ;
    }
    else
    {
      SEND_GADGET_ERR( "ERROR in ScanProperties" ) ;
    }
    return bRes ;
  }
  else
  {
    if ( ex.GetErrorMessage( ErrBuff , sizeof( ErrBuff ) ) )
      SEND_GADGET_ERR( _T( "Error %s for Load Config file %s opening " ) , ErrBuff , pFileName ) ;
    else
      SEND_GADGET_ERR( _T( "Unknown Error for Load Config file %s opening" ) , pFileName ) ;

  }
  return false ;
}

int MultiAver::CleanDoubledAreas()
{
  FXAutolock al( m_ParametersProtect ) ;
  int iNRemoved = 0 ;
  for ( int i = 0; i < m_AreasForGUI.GetCount(); i++ )
  {
    NamedRectangle& ThisRect = m_AreasForGUI.GetAt(i) ;
    for ( int j = i + 1; j < m_AreasForGUI.GetCount(); j++ )
    {
      NamedRectangle& OtherRect = m_AreasForGUI.GetAt( j ) ;
      if ( OtherRect.m_Rect == ThisRect.m_Rect )
      {
        m_AreasForGUI.RemoveAt( j ) ;
        j-- ;
        iNRemoved++ ;
      }
    }

  }
  return iNRemoved ;
};
