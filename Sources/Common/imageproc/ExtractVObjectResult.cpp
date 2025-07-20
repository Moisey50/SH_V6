
#include "stdafx.h"
#include "imageproc/ExtractVObjectResult.h"

bool ExtractStatistics( FXString& Data , FXSIZE& iPos ,
  int& iNSpots , int& iMaxIntens , int& iMinIntens )
{
  FXString Token = Data.Tokenize( "\n" , iPos );
  if ( iPos > 0 )
  {
    FXString Statistics = Data.Left( iPos );
    int iNSpotsPos = ( int ) Statistics.Find( "Spots=" );
    int iMaxPos = ( int ) Statistics.Find( "Max=" );
    int iMinPos = ( int ) Statistics.Find( "Min=" );
    if ( ( iNSpotsPos >= 0 ) || ( iMaxPos >= 0 ) || ( iMinPos >= 0 ) )
    {
      CHAR * p = Statistics.GetBuffer();
      iNSpots = ( iNSpotsPos >= 0 ) ?
        atoi( p + iNSpotsPos + 6 ) : 0;
      iMaxIntens = ( iMaxPos >= 0 ) ?
        ROUND( atof( p + iMaxPos + 4 ) ) : 0;
      iMinIntens = ( iMinPos >= 0 ) ?
        ROUND( atof( p + iMinPos + 4 ) ) : 0;
      return true;
    }
  }
  return false;  // not proper format 
}

int ExtractDataAboutSpots( const CTextFrame * pSrcTextFrame ,
  SpotArray& Result , FXString& SpotName ,
  int& iNSpots , int& iMaxIntens , int& iMinIntens )
{
  FXString Label = pSrcTextFrame->GetLabel();
  if ( Label.Find( "Data_Spot:" ) < 0 )
    return 0;
  else // extract name
  {
    SpotName = Label.Mid( 10 );
  }

  FXString Data = pSrcTextFrame->GetString();
  FXSIZE iPos = 0;
  if ( ExtractStatistics( Data , iPos , iNSpots , iMaxIntens , iMinIntens ) )
  {
    FXString Name , Token;
    while ( !( Token = Data.Tokenize( "\n" , iPos ) ).IsEmpty() )
    {
      if ( Token.Find( "//" ) >= 0 ) // comment or Caption
        continue;
      CColorSpot Spot;
      if ( Spot.FromString( Token , &Name ) )
      {
//         if ( Spot.m_dBlobWidth < )
//         {
//         }
        Result.Add( Spot );
      }
    };
    return ( int ) Result.size();
  }
  return 0;
}
int ExtractDataAboutSpots( const CTextFrame * pSrcTextFrame ,
  SpotVector& Result , FXString& SpotName ,
  int& iNSpots , int& iMaxIntens , int& iMinIntens )
{
  FXString Label = pSrcTextFrame->GetLabel();
  if ( Label.Find( "Data_Spot:" ) < 0 )
    return 0;
  else // extract name
    SpotName = Label.Mid( 10 );

  FXString Data = pSrcTextFrame->GetString();
  FXSIZE iPos = 0;
  if ( ExtractStatistics( Data , iPos , iNSpots , iMaxIntens , iMinIntens ) )
  {
    FXString Name , Token;
    while ( !( Token = Data.Tokenize( "\n" , iPos ) ).IsEmpty() )
    {
      if ( Token.Find( "//" ) >= 0 ) // comment or Caption
        continue;
      CColorSpot Spot;
      if ( Spot.FromString( Token , &Name ) )
        Result.push_back( Spot );
    };
    return ( int ) Result.size();
  }
  return 0;
}

int ExtractDataAboutSpots( const CDataFrame * pSrcFrame , SpotArray& Result )
{
  CFramesIterator* Iterator = pSrcFrame->CreateFramesIterator( text );
  if ( Iterator )
  {
    int iNSpots = 0;
    int iMaxIntens = 0;
    int iMinIntens = 0;
    FXString SpotName;
    //     SpotArray SpotResults;
    CTextFrame * pTextFrame = NULL;
    while ( pTextFrame = ( CTextFrame * ) Iterator->Next( NULL ) )
    {
      ExtractDataAboutSpots( pTextFrame ,
        Result , SpotName ,
        iNSpots , iMaxIntens , iMinIntens );
    }
  }
  return ( int ) Result.GetCount();
}

int ExtractDataAboutSpots( const CDataFrame * pSrcFrame , SpotVector& Result )
{
  CFramesIterator* Iterator = pSrcFrame->CreateFramesIterator( text );
  if ( Iterator )
  {
    int iNSpots = 0;
    int iMaxIntens = 0;
    int iMinIntens = 0;
    FXString SpotName;
    //     SpotArray SpotResults;
    CTextFrame * pTextFrame = NULL;
    while ( pTextFrame = ( CTextFrame * ) Iterator->Next( NULL ) )
    {
      ExtractDataAboutSpots( pTextFrame ,
        Result , SpotName ,
        iNSpots , iMaxIntens , iMinIntens );
    }
  }
  return ( int ) Result.size();
}

int ExtractDataAboutROIs( const CDataFrame * pSrcFrame ,
  NamedCDRects& ROIs )
{
  int iBefore = (int)ROIs.size() ;
  CFramesIterator * pIt = pSrcFrame->CreateFramesIterator( rectangle );
  if ( pIt )
  {
    CDataFrame * pFrame = pIt->Next();
    while ( pFrame )
    {
      FXString Label = pFrame->GetLabel();
      if ( Label.Find( "ROI:" ) == 0 ) 
      {
        CRectFrame * pRectFrame = pFrame->GetRectFrame() ;
        if ( pRectFrame )
        {
          CRect * pRect = ( CRect* ) ( ( LPRECT ) pRectFrame ) ;
          NamedCDRect NewROI( ( LPCTSTR ) Label.Mid( 4 ) ,
            ( DWORD ) ( ROIs.size() ) , pRect ) ;
          ROIs.emplace( ROIs.begin() , NewROI ) ;
        }
      }
      pFrame = pIt->Next() ;
    }
  }
  return (int)ROIs.size() - iBefore ;
}

int ExtractDataAboutSpots( const CDataFrame * pSrcFrame ,
  SpotVectors& MeasurementResults , NamedCmplxVectors& Profiles , 
  NamedCDRects * pROIs , LPCTSTR pSpotName )
{
  double dROIWidth = 0. ;
  if ( pROIs )
    ExtractDataAboutROIs( pSrcFrame , *pROIs ) ;

  CFramesIterator * pIt = pSrcFrame->CreateFramesIterator( transparent );
  if ( pIt )
  {
    CDataFrame * pFrame = pIt->Next();
    FXString LastSpotName , LastConturName , LastProfileName;
    int iNSpots = 0;
    int iMaxIntens = 0;
    int iMinIntens = 0;
    SpotVector Spots;
    FXString SpotName;
    while ( pFrame )
    {
      FXString Label = pFrame->GetLabel();
      switch ( pFrame->GetDataType() )
      {
      case text:
        {
          if ( MeasurementResults.empty() || !MeasurementResults.front().empty() )
            MeasurementResults.emplace( MeasurementResults.begin() );

          if ( ExtractDataAboutSpots( pFrame->GetTextFrame() ,
            MeasurementResults.at( 0 ) , SpotName ,
            iNSpots , iMaxIntens , iMinIntens ) )
          {
            LastSpotName = SpotName.Left( SpotName.Find( '_' ) );
          }
        }
        break;
      case figure:
        {
          CFigureFrame * pFigFrame = pFrame->GetFigureFrame();
          DWORD dwLength = ( DWORD ) pFigFrame->size();
          if ( dwLength > 1 )
          {
            if ( Label.Find( "Profile" ) >= 0 )
            {
              NamedCmplxVector NewProfile;
              NewProfile.m_ObjectName = ( LPCTSTR ) Label;
              NewProfile.m_ObjectIndex = atoi( ( ( LPCTSTR ) Label ) + Label.Find( '_' ) + 1 );
              Profiles.emplace( Profiles.begin() , NewProfile );
              Profiles.at( 0 ).m_Data.resize( dwLength );
              memcpy( Profiles.at( 0 ).m_Data.data() , pFigFrame->GetData() , dwLength * sizeof( cmplx ) );
            }
            else if ( Label.Find( "Contur" ) >= 0 )
            {
              if ( Label.Find( LastSpotName ) >= 0 )
              {
                int iIndex = atoi( ( ( LPCTSTR ) Label ) + Label.Find( '_' ) + 1 );
                if ( ( int ) MeasurementResults.at( 0 ).size() > iIndex
                  && MeasurementResults.at( 0 ).at( iIndex ).m_iIndex == iIndex )
                {
                  MeasurementResults.at( 0 ).at( iIndex ).m_Contur.SetSize( dwLength );
                  memcpy( MeasurementResults.at( 0 ).at( iIndex ).m_Contur.GetData() ,
                    pFigFrame->GetData() , dwLength * sizeof( cmplx ) );
                }
              }
            }
          }
        }
        break;
      }

      pFrame = pIt->Next();
    };

  }
  if ( MeasurementResults.size() && ( MeasurementResults.at( 0 ).size() == 0 ) )
    MeasurementResults.erase( MeasurementResults.begin() );
  return ( int ) MeasurementResults.size();
}
