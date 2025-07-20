// UserExampleGadget.h.h : Implementation of the UserExampleGadget class


#include "StdAfx.h"
#include "math\intf_sup.h"
#include "ObjectMatcherGadget.h"

//
//using namespace cv;
IMPLEMENT_RUNTIME_GADGET_EX( ObjectMatcherGadget , CFilterGadget , "Matchers" , TVDB400_PLUGIN_NAME );
//USER_FILTER_RUNTIME_GADGET(ObjectMatcherGadget,"ObjectMatcher");	//	Mandatory

const char* ObjectMatcherGadget::pList = "Match_I1; Match_I2; Match_I3";	//	Example

ObjectMatcherGadget::ObjectMatcherGadget()
{
  m_pContainer = NULL;
  m_pOutput = new COutputConnector( vframe * text );


  iMinimalFigLength = 30;
  dMaxAllowedDev = 3;
  sMatchMethod = "Match_I2";
  iMaxSizeDeviation = 10;
  bIgnoreSizeCompare = true;
  FXPropertyKit pk;
  FXString text;
  bool Invalidate = false;


  init();
  //LoadTemplatesFromFile();
}

CDataFrame* ObjectMatcherGadget::DoProcessing( const CDataFrame* pDataFrame )
{
  using namespace std;
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );

  if ( !VideoFrame || !VideoFrame->lpBMIH )
    return NULL;

  CFigureFrame *pFf = NULL;
  FXString fstr;
  CContainerFrame* resVal;
  resVal = CContainerFrame::Create();
  CFramesIterator* Iterator = pDataFrame->CreateFramesIterator( figure );
  if ( Iterator != NULL )
  {
    do
    {
      pFf = (CFigureFrame*) Iterator->Next( DEFAULT_LABEL );
      if ( pFf == NULL )
        break;
      if ( iMinimalFigLength <= pFf->GetSize() )
      {
        //vector <cv::Point> SourceCountour;
        Contour SourceCountour;
        double dMatchCoeff = 1000;
        for ( int counter = 0; counter < pFf->GetCount(); counter++ )
        {
          CDPoint cdPoint = pFf->GetAt( counter );
          cv::Point pt;
          pt.x = int( cdPoint.x );
          pt.y = int( cdPoint.y );
          SourceCountour.CountourVector.push_back( pt );
        }

        //             
        double dAngle;
        CDPoint cdCenterPoint;
        GetFigureCenterandAngle( pDataFrame , pFf , dAngle , cdCenterPoint ) ;
        //

        fstr = "Unknown";
        SourceCountour.dAngle = dAngle;
        SourceCountour.dCenter = cdCenterPoint;
        SourceCountour.Label = fstr;
        FindFigureHeightandWidth( SourceCountour );

        for ( int i = 0; i < m_TemplateCountourArr.GetSize(); i++ )
        {
          double d0;
          if ( sMatchMethod.Find( "0" ) > -1 )
            d0 = cv::matchShapes( cv::Mat( SourceCountour.CountourVector ) , cv::Mat( m_TemplateCountourArr.GetAt( i ).CountourVector ) , CV_CONTOURS_MATCH_I1 , 0 );
          else if ( sMatchMethod.Find( "1" ) > -1 )
            d0 = cv::matchShapes( cv::Mat( SourceCountour.CountourVector ) , cv::Mat( m_TemplateCountourArr.GetAt( i ).CountourVector ) , CV_CONTOURS_MATCH_I2 , 0 );
          else if ( sMatchMethod.Find( "2" ) > -1 )
            d0 = cv::matchShapes( cv::Mat( SourceCountour.CountourVector ) , cv::Mat( m_TemplateCountourArr.GetAt( i ).CountourVector ) , CV_CONTOURS_MATCH_I3 , 0 );

          //
          double deltaheight = m_TemplateCountourArr.GetAt( i ).dHeight - SourceCountour.dHeight  ;
          double deltawidth = m_TemplateCountourArr.GetAt( i ).dWidth - SourceCountour.dWidth  ;
          double dMaxDeviation = iMaxSizeDeviation / 100.;

          //
          if ( bIgnoreSizeCompare ) // sizes no matter 
          {
            if ( d0 < dMatchCoeff && d0 < dMaxAllowedDev )
            {
              dMatchCoeff = d0;
              fstr = m_TemplateCountourArr.GetAt( i ).Label;
            }
          }
          else if ( abs( (deltaheight) / m_TemplateCountourArr.GetAt( i ).dHeight ) < dMaxDeviation
            &&    abs( (deltawidth) / m_TemplateCountourArr.GetAt( i ).dWidth ) < dMaxDeviation )
          {
            if ( d0 < dMatchCoeff && d0 < dMaxAllowedDev )
            {
              dMatchCoeff = d0;
              fstr = m_TemplateCountourArr.GetAt( i ).Label;
            }
          }

        }
        /*
       CDPoint cdCenterPoint;
       int iIndex;
       CFramesIterator* TextIterator = pDataFrame->CreateFramesIterator(text);
       if (TextIterator!=NULL && fstr.Find("Unknown")==-1)
       {
         CTextFrame* TextFrame = (CTextFrame*)TextIterator->Next();
         while (TextFrame)
         {
           FXString Data;

           FXString fxText = TextFrame->GetString();
           if(fxText.Find("Spot")>-1)
           {
             int iTok = 0;

             Data = fxText.Tokenize(":",iTok);
             if(iTok!=-1)
               Data = fxText.Tokenize(":",iTok);


             double d1,d2,d3,d4,d5,d6,d7,d8,dAngle;
             int iNItems = sscanf( (LPCTSTR)Data ,"%d %lf %lf %lf %lf %lf %lf %lf %lf %lf" ,&iIndex , &d1 , &d2, &d3 ,&d4, &d5, &d6, &d7, &d8, &dAngle ) ;
             FXString fxptr;
             fxptr.Format("_%d]",iIndex);
             FXString  Label = pFf->GetLabel();
             if (Label.Find(fxptr)>-1)
             {
               TextFrame = (CTextFrame*)TextIterator->Next();
               if (TextFrame)
               {
                 FXPropertyKit * pProp = TextFrame->Attributes();
                 if (!pProp->IsEmpty())
                 {
                  pProp->GetDouble("x",cdCenterPoint.x) && pProp->GetDouble("y",cdCenterPoint.y);
                 }
               }
             }
           }


           TextFrame = (CTextFrame*)TextIterator->Next();
         }
       }

       */

       //if (cdCenterPoint.x == 0)
       //{
       //  AfxMessageBox("lll");
       //}

        cdCenterPoint.y -= 10;
        resVal->ChangeId( pDataFrame->GetId() );
        resVal->SetTime( pDataFrame->GetTime() );
        resVal->SetLabel( "ResultsRes" );

        CFigureFrame* ff = CFigureFrame::Create();
        ff->Attributes()->WriteString( "color" , "0xff0000" ) ;
        ff->AddPoint( cdCenterPoint ) ;
        ff->ChangeId( pDataFrame->GetId() );
        resVal->AddFrame( ff );

        FXString CoordView ;
        CoordView.Format( "(%6.2f,%6.2f) - %s" , cdCenterPoint.x , cdCenterPoint.y , (LPCTSTR) fstr ) ;
        CTextFrame * ViewText = CTextFrame::Create( CoordView ) ;
        ViewText->Attributes()->WriteInt( "x" , (int) (cdCenterPoint.x) );
        ViewText->Attributes()->WriteInt( "y" , (int) (cdCenterPoint.y) );
        ViewText->Attributes()->WriteString( "color" , "0xff0000" );
        ViewText->Attributes()->WriteInt( "Sz" , 12 ) ;
        ViewText->SetLabel( "spot" );
        ViewText->ChangeId( pDataFrame->GetId() ) ;
        ViewText->SetTime( pDataFrame->GetTime() ) ;
        resVal->AddFrame( ViewText );
      }

      pFf = (CFigureFrame*) Iterator->Next( DEFAULT_LABEL );
    } while ( pFf != NULL );
  }

  resVal->AddFrame( VideoFrame );
  m_pOutput->Put( resVal );

  CVideoFrame* retV = NULL;
  return retV;
}

void ObjectMatcherGadget::AsyncTransaction( CDuplexConnector* pConnector , CDataFrame* pParamFrame )
{
  if ( !pParamFrame )
    return;

  CTextFrame * ParamText = pParamFrame->GetTextFrame( DEFAULT_LABEL );
  if ( ParamText )
  {
    if ( ParamText->GetString().Find( "MaxDeviation" ) > -1 )
    {
      //
    }
    else if ( ParamText->GetString().Find( "MinLength" ) > -1 )
    {
    }
    else if ( ParamText->GetString().Find( "MatchMethod" ) > -1 )
    {
      //
    }
    else if ( ParamText->GetString().Find( "SaveTemplate" ) > -1 )
    {
      FXSIZE iTok = 0 ;
      FXString sPath = ParamText->GetString().Tokenize( _T( "_" ) , iTok );
      if ( iTok != -1 )
        sPath = ParamText->GetString().Tokenize( _T( "_" ) , iTok );
      SaveTemplatesToFile( sPath );
    }
    else if ( ParamText->GetString().Find( "LoadTemplate" ) > -1 )
    {
      FXSIZE iTok = 0 ;
      FXString sPath = ParamText->GetString().Tokenize( _T( "_" ) , iTok );
      if ( iTok != -1 )
        sPath = ParamText->GetString().Tokenize( _T( "_" ) , iTok );
      LoadTemplatesFromFile( sPath );
    }
    else if ( ParamText->GetString().Find( "ClearTemplates" ) > -1 )
    {
      m_TemplateCountourArr.RemoveAll();
    }
    else
      FigureParser( pParamFrame );
  }
  pParamFrame->Release( pParamFrame );
};

void ObjectMatcherGadget::PropertiesRegistration()
{
  addProperty( SProperty::EDITBOX , _T( "MaxAllowed" ) , &dMaxAllowedDev , SProperty::Double , 0.0001 , 1000 );
  addProperty( SProperty::EDITBOX , _T( "MinimalLength" ) , &iMinimalFigLength , SProperty::Int , 3 , 20000 );
  addProperty( SProperty::COMBO , _T( "MatchMethod" ) , &sMatchMethod , SProperty::String , pList );
  addProperty( SProperty::SPIN_BOOL , _T( "MaxSizeDeviation" ) , &iMaxSizeDeviation , SProperty::SpinBool , 1 , 100 , &bIgnoreSizeCompare );
  //addProperty(SProperty::SPIN_BOOL	,	_T("bool_int4")	,	&iProp4		,	SProperty::SpinBool	,	-10		,	20, &bProp4	);
};

void ObjectMatcherGadget::ConnectorsRegistration()
{
  addInputConnector( transparent , "InputName1" );
  addOutputConnector( createComplexDataType( 3 , rectangle , text , vframe ) , "OutputName1" );
  addDuplexConnector( transparent , transparent , "DuplexName1" );
};

/*
bool ObjectMatcherGadget::ScanSettings(FXString& text)
{
  text = "template(Spin(MaxAllowed),"
    "Spin(MinimalLength,3,40),"
    "EditBox(MatchMethod)"
    ")";
  return true;
}
*/

/*
bool ObjectMatcherGadget::ScanProperties(LPCTSTR text, bool& Invalidate)
{
  FXPropertyKit pk(text);
  pk.GetDouble( "MaxAllowed" , dMaxAllowedDev ) ;
  pk.GetInt( "MinimalLength" , iMinimalFigLength ) ;
  pk.GetInt( "Max Size Deviation(%)" , iMaxSizeDeviation ) ;
  pk.GetString( "MatchMethod" , sMatchMethod ) ;

  return true;
}

bool ObjectMatcherGadget::PrintProperties(FXString& text)
{
  FXPropertyKit pk;
  CFilterGadget::PrintProperties(text);
  pk.WriteString("MatchMethod" , sMatchMethod ) ;
  pk.WriteDouble( "MaxAllowed" , dMaxAllowedDev ) ;
  pk.WriteInt( "MinimalLength" , iMinimalFigLength ) ;
  pk.WriteInt( "Max Size Deviation(%)" , iMaxSizeDeviation ) ;
  text+=pk;

  return true;
}
  */

int ObjectMatcherGadget::FigureParser( CDataFrame* pParamFrame )
{
  FXString  FullLabel = pParamFrame->GetLabel();
  if ( FullLabel.Find( "template_" ) > -1 )
  {
    FXSIZE iTok = 0 ;
    FXString Label = FullLabel.Tokenize( _T( "_" ) , iTok );
    if ( iTok != -1 )
      Label = FullLabel.Tokenize( _T( "_" ) , iTok );
    if ( Label.Find( "." ) > -1 )
    {
      iTok = 0;
      Label = Label.Tokenize( _T( "." ) , iTok );
    }
    CFigureFrame *pFf = NULL;
    CFigureFrame *pBestFf = NULL;
    CFramesIterator* Iterator = pParamFrame->CreateFramesIterator( figure );
    if ( Iterator != NULL )
    {
      do
      {
        pFf = (CFigureFrame*) Iterator->Next( DEFAULT_LABEL );
        if ( pFf == NULL )
          break;
        else if ( pBestFf == NULL )
          pBestFf = pFf;
        else if ( pBestFf->GetSize() < pFf->GetSize() )
          pBestFf = pFf;
      } while ( pFf );
    }

    if ( pBestFf != NULL )
    {
      double dAngle;
      CDPoint cdCenterPoint;
      GetFigureCenterandAngle( pParamFrame , pBestFf , dAngle , cdCenterPoint ) ;
      pBestFf->SetLabel( Label );
      AddTemplateFigure( pBestFf , dAngle , cdCenterPoint );
    }
  }
  return -1;
}

void ObjectMatcherGadget::AddTemplateFigure( CFigureFrame* pFigureFrame , double dAngle , CDPoint cdCenter )
{
  if ( !m_TemplateCountourArr.GetSize() )
  {
    Contour tmpcntr;
    tmpcntr.Label = pFigureFrame->GetLabel();
    for ( int counter = 0; counter < pFigureFrame->GetCount(); counter++ )
    {
      CDPoint cdPoint = pFigureFrame->GetAt( counter );
      cv::Point pt;
      pt.x = int( cdPoint.x );
      pt.y = int( cdPoint.y );
      tmpcntr.CountourVector.push_back( pt );
    }

    tmpcntr.dAngle = dAngle;
    tmpcntr.dCenter = cdCenter;
    FindFigureHeightandWidth( tmpcntr );
    m_TemplateCountourArr.Add( tmpcntr );
  }
  else
  {
    int isz = (int) m_TemplateCountourArr.GetSize();
    BOOL bRelpaced = false;
    Contour tmpcntr;
    for ( int i = 0; i < isz ; i++ )
    {
      tmpcntr.Label = "";
      tmpcntr.CountourVector.clear();
      for ( int counter = 0; counter < pFigureFrame->GetCount(); counter++ )
      {
        CDPoint cdPoint = pFigureFrame->GetAt( counter );
        cv::Point pt;
        pt.x = int( cdPoint.x );
        pt.y = int( cdPoint.y );
        tmpcntr.CountourVector.push_back( pt );
      }
      tmpcntr.Label = pFigureFrame->GetLabel();

      if ( m_TemplateCountourArr.GetAt( i ).Label == pFigureFrame->GetLabel() )
      {
        tmpcntr.dAngle = dAngle;
        tmpcntr.dCenter = cdCenter;
        FindFigureHeightandWidth( tmpcntr );
        m_TemplateCountourArr.SetAt( i , tmpcntr );
        bRelpaced = true;
        break;
      }
    }
    if ( !bRelpaced )
    {
      tmpcntr.dAngle = dAngle;
      tmpcntr.dCenter = cdCenter;
      FindFigureHeightandWidth( tmpcntr );
      m_TemplateCountourArr.Add( tmpcntr );
    }
  }
}

void  ObjectMatcherGadget::SaveTemplatesToFile( FXString sPath )
{
  if ( !m_TemplateCountourArr.GetSize() )
    return;

  //FXString sPath = "c:\\ObjectMatcher.bin";
  FILE * pFile;
  //fopen_s( &pFile,( LPCTSTR )sPath, "w" ) ;
  errno_t Err = _tfopen_s( &pFile , sPath , "w" ) ;
  CFigureFrame* pFigureFrame = NULL;
  FXString CurLabel( "" );
  if ( pFile )
  {
    for ( int i = 0; i < m_TemplateCountourArr.GetSize(); i++ )
    {
      if ( m_TemplateCountourArr.GetAt( i ).Label != CurLabel )
      {
        CurLabel = m_TemplateCountourArr.GetAt( i ).Label;
        double dAngle = m_TemplateCountourArr.GetAt( i ).dAngle;
        double dx = m_TemplateCountourArr.GetAt( i ).dCenter.x;
        double dy = m_TemplateCountourArr.GetAt( i ).dCenter.y;
        FXString sDel;
        sDel.Format( "\\\\template_%s_%lf_%lf_%lf\n" , CurLabel , dAngle , dx , dy );
        fputs( sDel , pFile );
      }
      FXString fxPoints;
      for ( std::vector<cv::Point>::iterator it = m_TemplateCountourArr.GetAt( i ).CountourVector.begin() ; it != m_TemplateCountourArr.GetAt( i ).CountourVector.end(); ++it )
      {
        fxPoints.Format( "%d,%d\n" , it->x , it->y );
        fputs( fxPoints , pFile );
      }
    }
    fclose( pFile );
  }
}


void  ObjectMatcherGadget::LoadTemplatesFromFile( FXString sPath )
{
  FILE * pFile;
  //fopen_s( &pFile,( LPCTSTR )sPath, "r" ) ;
  errno_t Err = _tfopen_s( &pFile , sPath , "r" ) ;
  CFigureFrame* pFigureFrame = NULL;
  FXString CurLabel( "" );
  double dCurAngle;
  CDPoint cdCurCenter;
  if ( pFile )
  {
    char buf[ 256 ];
    FXString fxstr;
    while ( fgets( buf , 256 , pFile ) )
    {
      fxstr.Format( buf );
      if ( fxstr.Find( "template_" ) > -1 )
      {
        FXString Label;
        double dAngle;
        CDPoint cdCenter;
        GetParamFromString( fxstr , Label , dAngle , cdCenter );
        if ( pFigureFrame )
        {
          pFigureFrame->SetLabel( CurLabel );
          AddTemplateFigure( pFigureFrame , dCurAngle , cdCurCenter );
          FR_RELEASE_DEL( pFigureFrame ) ;
        }

        //         int iTok = 0 ;
        //         FXString Label = fxstr.Tokenize(_T("_"),iTok);	
        //         if(iTok != -1)
        //           Label = fxstr.Tokenize(_T("_"),iTok); 
        //         if(Label.Find(".")>-1)
        //         {
        //           iTok = 0;
        //           Label = Label.Tokenize(_T("."),iTok); 
        //         }
        if ( Label.GetLength() > 3 )
        {
          CurLabel = Label;
          dCurAngle = dAngle;
          cdCurCenter = cdCenter;
        }

        pFigureFrame = CFigureFrame::Create() ;
      }
      else if ( CurLabel.GetLength() > 0 && fxstr.GetLength() > 1 )
      {
        int ix , iy;
        sscanf( (LPCTSTR) buf , "%d,%d" , &ix , &iy );
        if ( pFigureFrame )
        {
          pFigureFrame->Add( CDPoint( double( ix ) , double( iy ) ) ) ;
        }
      }
    }
    if ( pFigureFrame )
    {
      pFigureFrame->SetLabel( CurLabel );
      AddTemplateFigure( pFigureFrame , dCurAngle , cdCurCenter );
      FR_RELEASE_DEL( pFigureFrame ) ;
    }
    fclose( pFile );
  }
}

void ObjectMatcherGadget::FindFigureHeightandWidth( Contour &SourceCountour )
{
  SourceCountour.dCenter.y -= 10;
  CDPoint cd1 , cd2 , cd3 , cd4;
  double dSlope = tan( SourceCountour.dAngle * M_PI / 180 );
  dSlope *= -1;//Inverse Axis
  double dInter = SourceCountour.dCenter.y - dSlope * SourceCountour.dCenter.x;

  double dSlopePer = -1 / dSlope;
  double dInterPer = SourceCountour.dCenter.y - dSlopePer * SourceCountour.dCenter.x;

  double dPrevLongRadMaxDelta_Abs = 0;
  double dPrevLongRadMinDelta_Abs = 0;
  double dPrevShortRadMaxDelta_Abs = 0;
  double dPrevShortRadMinDelta_Abs = 0;

  //   double dPrevWidthMinDelta = 0;
  //   double dPrevHeightMaxDelta = 0;
  //   double dPrevHeightMinDelta = 0;
  for ( std::vector<cv::Point>::iterator it = SourceCountour.CountourVector.begin() ; it != SourceCountour.CountourVector.end(); ++it )
  {
    double dContourX = it->x;
    double dContourY = it->y;
    double dFitLineY = dSlope * dContourX + dInter;
    double dPerFitLineY = dSlopePer * dContourX + dInterPer;

    double dDelta = dContourY - dFitLineY;
    double dDeltaPer = dContourY - dPerFitLineY;

    if ( dPrevLongRadMaxDelta_Abs == 0 )
      dPrevLongRadMaxDelta_Abs = dDeltaPer;

    if ( dPrevLongRadMinDelta_Abs == 0 )
      dPrevLongRadMinDelta_Abs = dDeltaPer;

    if ( dPrevShortRadMaxDelta_Abs == 0 )
      dPrevShortRadMaxDelta_Abs = dDelta;

    if ( dPrevShortRadMinDelta_Abs == 0 )
      dPrevShortRadMinDelta_Abs = dDelta;


    //Long Radius - Absolute
    if ( (dPrevLongRadMaxDelta_Abs) < (dDeltaPer) )
    {
      cd3.x = dContourX , cd3.y = dContourY;
      dPrevLongRadMaxDelta_Abs = dDeltaPer;
    }

    if ( (dPrevLongRadMinDelta_Abs) > (dDeltaPer) )
    {
      cd4.x = dContourX , cd4.y = dContourY;
      dPrevLongRadMinDelta_Abs = dDeltaPer;
    }

    //Short Radius - Absolute
    if ( (dPrevShortRadMaxDelta_Abs) < (dDelta) )
    {
      cd1.x = dContourX , cd1.y = dContourY;
      dPrevShortRadMaxDelta_Abs = dDelta;
    }
    if ( (dPrevShortRadMinDelta_Abs) > (dDelta) )
    {
      cd2.x = dContourX , cd2.y = dContourY;
      dPrevShortRadMinDelta_Abs = dDelta;
    }


    /*
    if (dPrevWidthMinDelta == 0)
      dPrevWidthMinDelta = dDelta;

    if (dPrevHeightMaxDelta == 0)
      dPrevHeightMaxDelta = dDelta;
    if (dPrevHeightMinDelta == 0)
      dPrevHeightMinDelta = dDelta;

    if(abs(dDelta) < abs(dPrevWidthMinDelta))
    {
      cd2 = cd1;
      cd1.x = dContourX, cd1.y = dContourY;
      dPrevWidthMinDelta = dDelta;
    }

    if((dPrevHeightMaxDelta) < (dDelta))
    {
      cd3.x = dContourX, cd3.y = dContourY;
      dPrevHeightMaxDelta = dDelta;
    }

    if((dPrevHeightMinDelta) > (dDelta))
    {
      cd4.x = dContourX, cd4.y = dContourY;
      dPrevHeightMinDelta = dDelta;
    }
  }

  double dwidth  = sqrt((cd1.x - cd2.x) * (cd1.x - cd2.x) + (cd1.y - cd2.y) * (cd1.y - cd2.y));
  double dHeight = sqrt((cd3.x - cd4.x) * (cd3.x - cd4.x) + (cd3.y - cd4.y) * (cd3.y - cd4.y));
  */
  }
  double dwidth = GetDistance( cd1 , cd2 );
  double dHeight = GetDistance( cd3 , cd4 );


  SourceCountour.dWidth = dwidth;
  SourceCountour.dHeight = dHeight;
}


void ObjectMatcherGadget::GetFigureCenterandAngle( const CDataFrame* pDataFrame , CFigureFrame * pFf , double &dAngle , CDPoint &cdCenterPoint )
{
  int iIndex;
  CFramesIterator* TextIterator = pDataFrame->CreateFramesIterator( text );
  if ( TextIterator != NULL )
  {
    CTextFrame* TextFrame = (CTextFrame*) TextIterator->Next();
    while ( TextFrame )
    {
      FXString Data;

      FXString fxText = TextFrame->GetString();
      if ( fxText.Find( "Spot" ) > -1 )
      {
        FXSIZE iTok = 0;

        Data = fxText.Tokenize( ":" , iTok );
        if ( iTok != -1 )
          Data = fxText.Tokenize( ":" , iTok );


        double d1 , d2 , d3 , d4 , d5 , d6 , d7 , d8;
        int iNItems = sscanf( (LPCTSTR) Data , "%d %lf %lf %lf %lf %lf %lf %lf %lf %lf" , &iIndex , &d1 , &d2 , &d3 , &d4 , &d5 , &d6 , &d7 , &d8 , &dAngle ) ;
        FXString fxptr;
        fxptr.Format( "_%d]" , iIndex );
        FXString  Label = pFf->GetLabel();
        if ( Label.Find( fxptr ) > -1 )
        {
          TextFrame = (CTextFrame*) TextIterator->Next();
          if ( TextFrame )
          {
            FXPropertyKit * pProp = TextFrame->Attributes();
            if ( !pProp->IsEmpty() )
            {
              pProp->GetDouble( "x" , cdCenterPoint.x ) && pProp->GetDouble( "y" , cdCenterPoint.y );
            }
          }
        }
      }
      TextFrame = (CTextFrame*) TextIterator->Next();
    }
  }
}
void ObjectMatcherGadget::GetParamFromString( FXString fxstr , FXString &fxLabel , double &dAngle , CDPoint &cdCenter )
{
  FXSIZE iTok = 0;
  FXString s1 = fxstr.Tokenize( "_" , iTok );
  if ( iTok != -1 )
    fxLabel = fxstr.Tokenize( "_" , iTok );
  if ( iTok != -1 )
    dAngle = atof( fxstr.Tokenize( "_" , iTok ) );
  if ( iTok != -1 )
    cdCenter.x = atof( fxstr.Tokenize( "_" , iTok ) );
  if ( iTok != -1 )
    cdCenter.y = atof( fxstr.Tokenize( "_" , iTok ) );
}

double ObjectMatcherGadget::GetDistance( CDPoint &pt1 , CDPoint &pt2 )
{
  return sqrt( (pt1.x - pt2.x) * (pt1.x - pt2.x) + (pt1.y - pt2.y) * (pt1.y - pt2.y) );
}