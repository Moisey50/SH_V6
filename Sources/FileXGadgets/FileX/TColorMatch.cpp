// TColorMatch.h : Implementation of the CTColorMatch class


#include "StdAfx.h"
#include <gadgets\videoframe.h>
#include <gadgets\quantityframe.h>
#include <gadgets\textframe.h>
#include <gadgets\ContainerFrame.h>
#include <helpers\FramesHelper.h>

#include "TColorMatch.h"

#define THIS_MODULENAME "TColorMatch"

IMPLEMENT_RUNTIME_GADGET_EX(TColorMatch, CFilterGadget, LINEAGE_VIDEO , TVDB400_PLUGIN_NAME);




TColorMatch::TColorMatch(void):
  m_GoodMark( 128 , 128 ) 
  ,m_BadMark( 255 , 0 )
  ,m_pMaskFrame(NULL)
{
  m_pInput = new CInputConnector( transparent );
  m_pOutput = new COutputConnector( text * vframe );
  m_WorkingMode = Teaching ;
  m_OutputMode = modeReplace ;
  m_iYForPresentation = 128 ;
  m_iMinRecognitionIntens = 30 ;
  m_iMinInHistValue = 5 ;
  m_bYChanged = false ;
  m_AllowedColorRange.left = 1e30 ;
  m_bGetNextForTeaching = FALSE ;
  memset( m_UVHist , 0 , sizeof(m_UVHist) ) ;

  m_pOutputFrame = CVideoFrame::Create() ;
  // all next for 8 bits YUV12 image
  UINT uiImageWidth = ( SQUARE_SIZE + YCOLUMN_WIDTH ) ;
  UINT uiImageHeight = SQUARE_SIZE ;
  UINT uiYSize = uiImageWidth * uiImageHeight  ;
  UINT uiUSize = uiYSize / 4 ;
  UINT uiVSize = uiYSize / 4 ;
  UINT uiFRameSize = uiYSize + uiUSize + uiVSize ;
  m_pOutputFrame->lpBMIH = (LPBITMAPINFOHEADER)malloc( sizeof(BITMAPINFOHEADER) + uiFRameSize ) ;
  if ( m_pOutputFrame->lpBMIH )
  {
    m_pOutputFrame->lpBMIH->biCompression = BI_YUV12 ;
    m_pOutputFrame->lpBMIH->biWidth = uiImageWidth ;
    m_pOutputFrame->lpBMIH->biHeight = uiImageHeight ;
    m_pOutputFrame->lpBMIH->biSize = sizeof( BITMAPINFOHEADER ) ;
    m_pOutputFrame->lpBMIH->biSizeImage = uiFRameSize ;
    m_pOutputFrame->lpBMIH->biBitCount = 12 ;
    m_pOutputFrame->lpBMIH->biClrUsed = 0 ;
    m_pOutputFrame->lpBMIH->biClrImportant = 0 ;
    m_pOutputFrame->lpBMIH->biPlanes = 1 ;
    m_pOutputFrame->lpBMIH->biXPelsPerMeter =
      m_pOutputFrame->lpBMIH->biYPelsPerMeter = 0 ;

    LPBYTE pY = GetData( m_pOutputFrame ) ;
    memset( pY , 128 , uiFRameSize ) ; // fill Y with 1/2 brightness
    char * pU = (char*)( pY + uiYSize ) ;
    char * pV = (char*)( pU + uiUSize ) ;

    UINT uiColorWidth = SQUARE_SIZE / 2 ;
    UINT uiUColorStep = uiColorWidth + YCOLUMN_WIDTH/2 ;
    UINT uiColorHeight = SQUARE_SIZE / 2 ;
    for ( UINT uiY = 0 ; uiY < uiColorHeight ; uiY++ )
    {
      char * pURow = pU + uiUColorStep * uiY ;
      char * pVRow = pV + uiUColorStep * uiY ;
      memset( pVRow , (255 - uiY) & 0xff , uiColorWidth ) ;
      for ( UINT uiX = 0 ; uiX < uiColorWidth ; uiX++ )
      {
        *(pURow++) = (char)((/*255-*/uiX) & 0xff) ;
      }
    }
  }
  Resume();
}

void TColorMatch::ShutDown()
{
  //TODO: Add all destruction code here
  if ( m_pOutputFrame )
    m_pOutputFrame->Release() ;
  m_pOutputFrame = NULL ;
  if ( m_pMaskFrame )
    m_pMaskFrame->Release() ;
  m_pMaskFrame = NULL ;

  CFilterGadget::ShutDown();
  delete m_pInput;
  m_pInput = NULL;
  delete m_pOutput;
  m_pOutput = NULL;
}

void TColorMatch::FormUVPresentation( CContainerFrame * pOut , CDPoint * Pt , int * iY ) 
{
  LPCTSTR RectViewColor = _T("0xffffff") ;
  if ( Pt )
  {
    CFigureFrame * pPosition = CFigureFrame::Create() ;
    pPosition->Add( *Pt ) ;
    pPosition->Attributes()->Format( _T("color=ffffff;thickness=5;") ) ;
    pOut->AddFrame( pPosition ) ;
  }
  if ( iY )
  {
    CFigureFrame * pIntensity =  CFigureFrame::Create() ;
    CDPoint Bottom( 528. , 510. ) ;
    CDPoint Top( 528. , 510. - (*iY) * 2 ) ;
    pIntensity->Add( Bottom ) ;
    pIntensity->Add( Top ) ;
    *(pIntensity->Attributes()) = _T("color=ffffff;thickness=5;") ;
    pOut->AddFrame( pIntensity ) ;
  }
  else if ( m_WorkingMode == RecognitionText )
  {
    CPoint ColorPt( ROUND(Pt->x) , ROUND(Pt->y) ) ;
    RectViewColor = ( PtInside( m_UVRange , ColorPt ) )? _T("0x00ff00") : _T("0x0000ff") ;
  }
  if ( m_AllowedColorRange.left < 1e30 )
  {
    CFigureFrame * pRange = CFigureFrame::Create() ;
    pRange->Add( CDPoint( m_AllowedColorRange.left , m_AllowedColorRange.top ) ) ;
    pRange->Add( CDPoint( m_AllowedColorRange.right , m_AllowedColorRange.top ) ) ;
    pRange->Add( CDPoint( m_AllowedColorRange.right , m_AllowedColorRange.bottom ) ) ;
    pRange->Add( CDPoint( m_AllowedColorRange.left , m_AllowedColorRange.bottom ) ) ;
    pRange->Add( CDPoint( m_AllowedColorRange.left , m_AllowedColorRange.top ) ) ;
    ((FXPropertyKit*)(pRange->Attributes()))
      ->Format( _T("color=%s;thickness=5;") , RectViewColor ) ;
    pOut->AddFrame( pRange ) ;
  }
  if ( m_iYForPresentation != -1 )
  {
    if ( m_bYChanged )
    {
      memset( GetData( m_pOutputFrame ) , m_iYForPresentation , 
        GetI8Size( m_pOutputFrame ) ) ; // fill Y with  brightness)
      m_bYChanged = false ;
    }
  }
  else 
  {
    memset( GetData( m_pOutputFrame ) , *iY , 
      GetI8Size( m_pOutputFrame ) ) ; // fill Y with  brightness)
  }
  CVideoFrame * pVOut = (CVideoFrame*)m_pOutputFrame->Copy() ;
  pOut->AddFrame( pVOut ) ;
};

CDataFrame* TColorMatch::DoProcessing(const CDataFrame* pDataFrame)
{
  CContainerFrame * pOut = NULL ;
  const CTextFrame * tf = pDataFrame->GetTextFrame() ;
  if ( tf )
  {
    LPCTSTR pLabel = tf->GetLabel() ;
    if ( pLabel  &&  pLabel[0] != 0 )
    {
      if ( strstr( pLabel , "Teaching" ) == pLabel )
      {
        if ( tf->GetString() == _T("Teaching On")  ||  tf->GetString() == _T("Teaching") )
        {
          m_AllowedColorRange.left = 1e30 ;
          m_UVRange.left = 1000000 ;
          m_WorkingMode = Teaching ;
          memset( m_UVHist , 0 , sizeof(m_UVHist) ) ;
        }
        else if ( tf->GetString() == _T("Teaching Off") )
        {
          m_WorkingMode = RecognitionText ;
        }
        else if ( tf->GetString() == _T("AutoTeaching") )
        {
          m_AllowedColorRange.left = 1e30 ;
          m_UVRange.left = 1000000 ;          
          m_WorkingMode = AutoTeaching ;
          memset( m_UVHist , 0 , sizeof(m_UVHist) ) ;
        }
        else if ( tf->GetString() == _T("GetNextForTeaching") )
        {
          m_WorkingMode = AutoTeaching ;
          m_bGetNextForTeaching = true ;
        }
        else
        return NULL ;
      }
      else if ( strstr( pLabel , "Recognition" ) == pLabel )
      {
        if ( tf->GetString() == "Video"  ||  tf->GetString() == "Recognition On" )
          m_WorkingMode = RecognitionVideo ;
        else if ( tf->GetString() == "Pass"  ||   tf->GetString() == "Recognition Off")
          m_WorkingMode = Pass ;
        else
          m_WorkingMode = RecognitionText ;
        return NULL ;
      }
    }

    FXPropertyKit pk = tf->GetString() ;
    int iY , iU , iV ;
    
    if ( (m_WorkingMode != RecognitionVideo) && (m_WorkingMode != AutoTeaching )
      && pk.GetInt( "I" , iY ) && pk.GetInt( "U" , iU ) && pk.GetInt( "V" , iV ) )
    {
      pOut = CContainerFrame::Create() ;
      if ( m_WorkingMode == Teaching )
      {
        if ( m_UVRange.left == 1000000 )
        {
          m_UVRange.left = m_UVRange.right = iU ;
          m_UVRange.top = m_UVRange.bottom = iV ;
        }
        else
          UnionRectAndPt( m_UVRange , CPoint( iU , iV ) ) ;
      }
      TRACE("\nY=%d U=%d V=%d Zone[(%d,%d),(%d,%d)] " , iY , iU , iV , 
        m_UVRange.left , m_UVRange.top , m_UVRange.right , m_UVRange.bottom ) ;
      CPoint PtColor( iU , iV ) ;
      iU *= 2 ;
      iV *= 2 ;
      iU += 256 ;
      iV = 256 - iV ;
      CDPoint UV( (double) iU , (double)iV ) ;
      AddUV_Value( UV ) ;

      FormUVPresentation( pOut , &UV , &iY ) ;
      pOut->CopyAttributes( pDataFrame ) ;
      pOut->SetTime( (GetGraphTime() * 1.e-3)) ;
      if ( !m_pOutput->Put( pOut ) )
        pOut->Release( pOut ) ;
      pOut = NULL ;
    } 
  }
  const CVideoFrame * vf = pDataFrame->GetVideoFrame() ;
  DWORD dwCompression = (vf) ? GetCompression( vf ) : 0xffffffff ;
  if ( vf )
  {
    if ( strcmp( vf->GetLabel() , _T("Mask") ) == NULL ) // label is Mask
    {
      if ( m_pMaskFrame )
        m_pMaskFrame->Release() ;
      m_pMaskFrame = (CVideoFrame*) vf ;
      m_pMaskFrame->AddRef() ;
    }
    else if ( ( dwCompression == BI_YUV12 )  
           || ( dwCompression == BI_YUV9  ) ) 
    {
      CVideoFrame * pOutFrame = NULL ;
      int iActivePixelCount = 0 ;
      int iBadPiixelCount = 0 ;
      if ( m_WorkingMode == RecognitionVideo)  
      {
        pOutFrame = (CVideoFrame*)vf->Copy() ;
        UINT uiImageSize = GetImageSizeWH( pOutFrame ) ;
//         int iDebugX = 594 ;
//         int iDebugY = 226 ;

        if ( pOutFrame && ClearColor( pOutFrame ) )
        {
          LPBYTE pU = (LPBYTE)GetU( (CVideoFrame*)vf ) ;
          LPBYTE pV = (LPBYTE)GetV( (CVideoFrame*)vf ) ;
          LPBYTE pOutU = (LPBYTE)GetU( pOutFrame ) ;
          LPBYTE pOutV = (LPBYTE)GetV( pOutFrame ) ;
          LPBYTE pMask = ( m_pMaskFrame )? GetData( m_pMaskFrame ) : NULL ;
          LPBYTE pPix = pMask ;
          UINT uiIntenseWidth = GetWidth( vf ) ;
          LPBYTE pIntens = GetData( vf ) ;
          UINT uiWidth = GetWidth( m_pMaskFrame ) ;
          switch( dwCompression )
          {
          case BI_YUV9:
            {
              UINT uiColorSize = uiImageSize/16 ;
              if ( pPix )
                pPix += uiWidth * 2 + 2 ;
              LPBYTE pFirstPix = pPix ;
              LPBYTE pFirstIntens = pIntens + uiIntenseWidth + 2 ;
              for ( UINT i = 0 ; i < uiColorSize ; i++ )
              {
                if ( (!pPix || (*pPix < 255))  
                  && ((*pIntens > m_iMinRecognitionIntens)  &&  (*pIntens < 250)) )
                {
                  iActivePixelCount++ ;
                  bool bRes ;
                  if ( m_UVRange.left < 10000 )
                  {
                    int iV = (int)(*(pU++)) - 128 ;
                    int iU = (int)(*(pV++)) - 128 ;
                    CPoint Color( iU , iV ) ;
                    bRes = PtInside( m_UVRange , Color ) ;
                  }
                  else
                    bRes = IsGoodColor( (int)(*(pU++)) , (int)(*(pV++))  ) ;

                  if ( bRes )
                  {
                    *pOutU = (BYTE)m_GoodMark.x ;
                    *pOutV = (BYTE)m_GoodMark.y ;
                  }
                  else
                  {
                    iBadPiixelCount++ ;
                    *pOutU = (BYTE)m_BadMark.x ;
                    *pOutV = (BYTE)m_BadMark.y ;
                  }
                }
                else
                {
                  pU++ ;
                  pV++ ;
                }
                if ( pPix )
                {
                  pPix += 4 ;
                  if ( (UINT)(pPix - pFirstPix) >= uiWidth )
                    pPix = (pFirstPix += uiWidth * 4) ;
                }
                
                pOutU++ ;
                pOutV++ ;
                pIntens += 4 ;
                if ( (UINT)(pIntens - pFirstIntens) >= uiIntenseWidth )
                  pIntens = ( pFirstIntens += uiIntenseWidth * 4 ) ;
              }
            }
            break ;
          case BI_YUV12:
            {
              UINT uiColorSize = uiImageSize/4 ;
//              UINT CatchIndex = (iDebugX/2) + (iDebugY/2) * (vf->lpBMIH->biWidth/2) ;
              int SavedU = 1000 , SavedV = 1000 ;
              if ( pPix )
                pPix += uiWidth ;
              LPBYTE pFirstPix = pPix ;
              LPBYTE pFirstIntens = pIntens ;
              for ( UINT i = 0 ; i < uiColorSize ; i++ )
              {
                if ( (!pPix || (*pPix < 255))  
                  && ((*pIntens > m_iMinRecognitionIntens)  &&  (*pIntens < 250)) )
                {
                  iActivePixelCount++ ;
                  bool bRes ;
                  if ( m_UVRange.left < 10000 )
                  {
                    int iU = (int)(*(pU++)) - 128 ;
                    int iV = (int)(*(pV++)) - 128 ;
                    CPoint Color( iU , iV ) ;
                    bRes = PtInside( m_UVRange , Color ) ;
                  }
                  else
                    bRes = IsGoodColor( (int)(*(pU++)) , (int)(*(pV++))  ) ;

                  if ( bRes )
                  {
                    *pOutU = (BYTE)m_GoodMark.x ;
                    *pOutV = (BYTE)m_GoodMark.y ;
                  }
                  else
                  {
                    iBadPiixelCount++ ;
                    *pOutU = (BYTE)m_BadMark.x ;
                    *pOutV = (BYTE)m_BadMark.y ;
                  }
                }
                else
                {
                  pU++ ;
                  pV++ ;
                }
                pPix += 2 ;
                if ( (UINT)(pPix - pFirstPix) >= uiWidth )
                {
                  pPix = (pFirstPix += uiWidth * 2) ;
                }
                pOutU++ ;
                pOutV++ ;
                pIntens += 2 ;
                if ( (UINT)(pIntens - pFirstIntens) >= uiIntenseWidth )
                  pIntens = ( pFirstIntens += uiIntenseWidth * 2 ) ;
              }
//               TRACE("\n U=%d V=%d Zone[(%d,%d),(%d,%d)] " , SavedU , SavedV ,
//                 m_UVRange.left , m_UVRange.top , m_UVRange.right , m_UVRange.bottom ) ; 
            }
            break ;
          }
        }
      }
      if ( m_WorkingMode == AutoTeaching)  
      {
        if ( m_bGetNextForTeaching )
        {
          m_bGetNextForTeaching = FALSE ;
          UINT uiImageSize = GetImageSizeWH( vf ) ;

          LPBYTE pU = (LPBYTE)GetU( (CVideoFrame*)vf ) ;
          LPBYTE pV = (LPBYTE)GetV( (CVideoFrame*)vf ) ;
          LPBYTE pMask = ( m_pMaskFrame )? GetData( m_pMaskFrame ) : NULL ;
          LPBYTE pPix = pMask ;
          UINT uiIntenseWidth = GetWidth( vf ) ;
          LPBYTE pIntens = GetData( vf ) ;
          UINT uiWidth = GetWidth( m_pMaskFrame ) ;
          switch( dwCompression )
          {
          case BI_YUV9:
            {
              UINT uiColorSize = uiImageSize/16 ;
              if ( pPix )
                pPix += uiWidth * 2 + 2 ;
              LPBYTE pFirstPix = pPix ;
              LPBYTE pFirstIntens = pIntens + uiIntenseWidth + 2 ;
              for ( UINT i = 0 ; i < uiColorSize ; i++ )
              {
                if ( (!pPix || (*pPix < 255))  
                  && ((*pIntens > m_iMinRecognitionIntens)  &&  (*pIntens < 250)) )
                {
                  //                   int iU = (int)(*(pU++)) - 128 ;
                  //                   int iV = (int)(*(pV++)) - 128 ;
                  PutToHist( (int)(*(pU++)) , (int)(*(pV++)) ) ;
//                   if ( m_UVRange.left == 1000000 )
//                   {
//                     m_UVRange.left = m_UVRange.right = iU ;
//                     m_UVRange.top = m_UVRange.bottom = iV ;
//                   }
//                   else
//                     UnionRectAndPt( m_UVRange , CPoint( iV , iU ) ) ;
// //                   CDPoint Color( (double)iU , (double)iV ) ;
//                   AddUV_Value( Color ) ;
                }
                else
                {
                  pU++ ;
                  pV++ ;
                }
                if ( pPix )
                {
                  pPix += 4 ;
                  if ( (UINT)(pPix - pFirstPix) >= uiWidth )
                    pPix = (pFirstPix += uiWidth * 4) ;
                }

                pIntens += 4 ;
                if ( (UINT)(pIntens - pFirstIntens) >= uiIntenseWidth )
                  pIntens = ( pFirstIntens += uiIntenseWidth * 4 ) ;
              }
            }
            break ;
          case BI_YUV12:
            {
              UINT uiColorSize = uiImageSize/4 ;
              int SavedU = 1000 , SavedV = 1000 ;
              if ( pPix )
                pPix += uiWidth ;
              LPBYTE pFirstPix = pPix ;
              LPBYTE pFirstIntens = pIntens ;
              for ( UINT i = 0 ; i < uiColorSize ; i++ )
              {
                if ( (!pPix || (*pPix < 255))  
                  && ((*pIntens > m_iMinRecognitionIntens)  &&  (*pIntens < 250)) )
                {
//                   int iU = (int)(*(pU++)) - 128 ;
//                   int iV = (int)(*(pV++)) - 128 ;
                  PutToHist( (int)(*(pU++)) , (int)(*(pV++)) ) ;
//                   if ( m_UVRange.left == 1000000 )
//                   {
//                     m_UVRange.left = m_UVRange.right = iU ;
//                     m_UVRange.top = m_UVRange.bottom = iV ;
//                   }
//                   else
//                     UnionRectAndPt( m_UVRange , CPoint( iV , iU ) ) ;
                  //                   CDPoint Color( (double)iU , (double)iV ) ;
                  //                   AddUV_Value( Color ) ;
                }
                else
                {
                  pU++ ;
                  pV++ ;
                }
                pPix += 2 ;
                if ( (UINT)(pPix - pFirstPix) >= uiWidth )
                {
                  pPix = (pFirstPix += uiWidth * 2) ;
                }
                pIntens += 2 ;
                if ( (UINT)(pIntens - pFirstIntens) >= uiIntenseWidth )
                  pIntens = ( pFirstIntens += uiIntenseWidth * 2 ) ;
              }
              //               TRACE("\n U=%d V=%d Zone[(%d,%d),(%d,%d)] " , SavedU , SavedV ,
              //                 m_UVRange.left , m_UVRange.top , m_UVRange.right , m_UVRange.bottom ) ; 
            }
            break ;
          }
        }
        pOutFrame = m_pMaskFrame ;
        m_pMaskFrame->AddRef() ;
      }
      if ( pOutFrame )
      {
        pOutFrame->CopyAttributes( pDataFrame ) ;
        pOutFrame->SetTime( (GetGraphTime() * 1.e-3)) ;
        if ( iActivePixelCount )
        {
          FXString Result ;
          Result.Format( "BadPercents=%6.2f; Active=%d; Bad=%d; ImageSize=%d;" ,
            100. * (double)iBadPiixelCount/(double)iActivePixelCount ,
            iActivePixelCount , iBadPiixelCount , GetImageSizeWH( vf ) ) ;
          CTextFrame * pResult = CTextFrame::Create( (LPCTSTR)Result ) ;
          pResult->SetLabel( "PercentOfBad" ) ;
          pOut = CContainerFrame::Create() ;
          pOut->AddFrame( pOutFrame ) ;
          pOut->AddFrame( pResult ) ;
          pOut->CopyAttributes( pDataFrame ) ;
          if ( !m_pOutput->Put( pOut ) )
            pOut->Release( pOut ) ;
        }
        else
        {
          if ( !m_pOutput->Put( pOutFrame ) )
            pOutFrame->Release( pOutFrame ) ;
        }
      }
    }
  }
  return NULL;
}

bool TColorMatch::ScanProperties(LPCTSTR text, bool& Invalidate)
{
   FXPropertyKit pk(text);
   //pk.GetString( "FileName" , m_FileName ) ;
   m_bYChanged = pk.GetInt( "IntensForView" , m_iYForPresentation ) ;
   pk.GetInt( "AutoTeach" , m_bAutoTeach ) ;
   pk.GetInt( "MinIntensity" , m_iMinRecognitionIntens ) ;
   pk.GetInt( "MinInHist" , m_iMinInHistValue ) ;
   FXString Tmp ;
   if ( pk.GetString( "GoodMark" , Tmp ) ) 
   {
     int iU , iV ;
     if ( sscanf_s( (LPCTSTR)Tmp , "%d,%d" , &iU , &iV ) )
     {
       m_GoodMark.x = iU & 0xff ;
       m_GoodMark.y = iV & 0xff ;
     }
   };
   if ( pk.GetString( "BadMark" , Tmp ) ) 
   {
     int iU , iV ;
     if ( sscanf_s( (LPCTSTR)Tmp , "%d,%d" , &iU , &iV ) )
     {
       m_BadMark.x = iU & 0xff ;
       m_BadMark.y = iV & 0xff ;
     }
   };
   return true;
}

bool TColorMatch::PrintProperties(FXString& text)
{
  FXPropertyKit pk;
  CFilterGadget::PrintProperties(text);
//   pk.WriteString("FileName",m_FileName);
  pk.WriteInt( "IntensForView" , m_iYForPresentation ); 
  FXString Tmp ;
  Tmp.Format("%d,%d" , m_GoodMark.x , m_GoodMark.y ) ;
  pk.WriteString( "GoodMark" , (LPCTSTR)Tmp ) ;
  Tmp.Format("%d,%d" , m_BadMark.x , m_BadMark.y ) ;
  pk.WriteString( "BadMark" , (LPCTSTR)Tmp ) ;
  pk.WriteInt( "AutoTeach" , m_bAutoTeach ) ;
  pk.WriteInt( "MinIntensity" , m_iMinRecognitionIntens ) ;
  pk.WriteInt( "MinInHist" , m_iMinInHistValue ) ;
  text+=pk;
  return true;
}

bool TColorMatch::ScanSettings(FXString& text)
{
  text = "template(Spin(IntensForView,-1,255),"
    "ComboBox(AutoTeach(No(0),Yes(1))),"
    "Spin(MinIntensity,10,100),"
    "Spin(MinInHist,1,100000),"
    "EditBox(GoodMark),"
    "EditBox(BadMark))" ;
  
  return true;
}

