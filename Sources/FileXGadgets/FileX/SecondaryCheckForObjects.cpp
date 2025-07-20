#include "stdafx.h"
#include "SecondaryCheckForObjects.h"
#include <imageproc/ImageProfile.h>


double CheckForVLineOnHBlackStrip( const CVideoFrame * pVF , 
  CRect& rcLinePos , // !!! right holds width and bottom holds height
  double& dLineWidth , double dNormThres , CContainerFrame * pMarking )
{
  CPoint Center( rcLinePos.left + rcLinePos.right/2 ,
    rcLinePos.top + rcLinePos.bottom/2 ) ;
  Profile VProfile( rcLinePos.bottom + 1 ) ;

  double dAverVProfile = calc_vprofile( pVF , &VProfile , &rcLinePos );

  double dMax = VProfile.m_dMaxValue;
  double dMin = VProfile.m_dMinValue;
  double dThres = ( dMin + dMax ) * dNormThres ;
  double * pProfData = VProfile.m_pProfData;
  cmplx UpperBodyEdge , LowerBodyEdge;
  int iYBegin = INT_MAX;
  int iYEnd = INT_MAX;

  while ( ( ( *pProfData > dThres ) || ( *( pProfData + 1 ) > dThres ) )
    && ( ( pProfData - VProfile.m_pProfData ) < VProfile.m_iProfLen ) )
    pProfData++;
  if ( ( pProfData - VProfile.m_pProfData ) < VProfile.m_iProfLen - 1 )
  {
    UpperBodyEdge._Val[ _RE ] = Center.x;
    UpperBodyEdge._Val[ _IM ] = iYBegin = rcLinePos.top + ( int ) ( pProfData - VProfile.m_pProfData );
    if ( pMarking )
      pMarking->AddFrame( CreatePtFrameEx( UpperBodyEdge , 0xff00ff , 3 ) );
    
    while ( ( ( *pProfData < dThres ) || ( *( pProfData + 1 ) < dThres ) )
      && ( ( pProfData - VProfile.m_pProfData ) < VProfile.m_iProfLen - 1 ) )
      pProfData++;

    if ( ( pProfData - VProfile.m_pProfData ) < VProfile.m_iProfLen - 1 )
    {
      LowerBodyEdge._Val[ _RE ] = Center.x;
      LowerBodyEdge._Val[ _IM ] = iYEnd = rcLinePos.top + ( int ) ( pProfData - VProfile.m_pProfData );
      if ( pMarking )
        pMarking->AddFrame( CreatePtFrameEx( LowerBodyEdge , 0xff00ff , 3 ) );

      int iXFrom = rcLinePos.left ;
      int iRange = rcLinePos.right ;
      int iXTo = iXFrom + iRange;

      CRect rcHorProf( iXFrom , iYBegin , iRange , iYEnd - iYBegin ) ;
      Profile HProfile( rcHorProf.right + 1 ) ;
      double dAverVProfile = calc_hprofile( pVF , &HProfile , &rcHorProf );
      dMax = HProfile.m_dMaxValue;
      dMin = HProfile.m_dMinValue;
      dThres = ( dMin + dMax ) * dNormThres ;

      double dLeftEdge = find_border_forw( HProfile.m_pProfData , HProfile.m_iProfLen , dThres ) ;
      if ( dLeftEdge != 0 )
      {
        double dRightEdge = find_border_back( HProfile.m_pProfData , HProfile.m_iProfLen , dThres ) ;
        if ( dRightEdge > 0. )
        {
          dLineWidth = dRightEdge - dLeftEdge ;
          double dLinePos = iXFrom + ( dLeftEdge + dRightEdge ) * 0.5 ;
          return dLinePos ;
        }
      }
    }
  }

  return 0;
}
