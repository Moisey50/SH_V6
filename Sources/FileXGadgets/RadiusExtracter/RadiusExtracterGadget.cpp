// UserExampleGadget.h.h : Implementation of the UserExampleGadget class


#include "StdAfx.h"
#include "RadiusExtracterGadget.h"

//USER_FILTER_RUNTIME_GADGET(RadiusExtracterGadget,"RadiusExtracter");	//	Mandatory
//strcpy_s(TVDB400_PLUGIN_NAME,fName);

 IMPLEMENT_RUNTIME_GADGET_EX(RadiusExtracterGadget, CFilterGadget, "Matchers", TVDB400_PLUGIN_NAME);
//const char* UserExampleGadget::pList = "AA1; AA2; AA3; BB1; BB2; BB3; CC3";	//	Example

RadiusExtracterGadget::RadiusExtracterGadget()
{
	//	Example


	//	Mandatory
  m_pOutput = new COutputConnector(vframe * figure);
	init();
}

CDataFrame* RadiusExtracterGadget::DoProcessing(const CDataFrame* pDataFrame) 
{

  int iMinimalFigLength  = 20;
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame(DEFAULT_LABEL);

  if (!VideoFrame  || !VideoFrame->lpBMIH )
    return NULL;
 
  CContainerFrame * pResult ;
  pResult = CContainerFrame::Create() ;
  pResult->ChangeId(pDataFrame->GetId());
  pResult->SetTime(pDataFrame->GetTime());
  pResult->SetLabel( "RadiusExtracterResults" ) ;

 

  CFigureFrame *pFf = NULL;
  FXString fstr;
  //CContainerFrame* resVal;
  //resVal = CContainerFrame::Create();
  CFramesIterator* Iterator = pDataFrame->CreateFramesIterator(figure);
  if (Iterator!=NULL)
  {
    do 
    { 
      pFf = (CFigureFrame*)Iterator->Next(DEFAULT_LABEL);	
      if(pFf==NULL)
        break;
      if (iMinimalFigLength <= pFf->GetSize())
      {
        Contour SourceCountour;
        double dMatchCoeff = 1000;
        for(int counter = 0; counter < pFf->GetCount(); counter++)
        {
          CDPoint cdPoint = pFf->GetAt(counter);
          SourceCountour.CountourVector.Add(cdPoint);
        }
        double dAngle;
        CDPoint cdCenterPoint;
        GetFigureCenterandAngle(pDataFrame,pFf,dAngle,cdCenterPoint) ;

        SourceCountour.dAngle =  dAngle;
        SourceCountour.dCenter = cdCenterPoint;
        FindFigureHeightandWidth(SourceCountour);

        cdCenterPoint.y-=10;
        
        CFigureFrame * pAbsolutePt = NULL ;
        CFigureFrame * pRelativePt = NULL;
        
        if(pAbsolutePt == NULL)
        {
          cmplx pt( SourceCountour.Up_Abs.x, SourceCountour.Up_Abs.y);
          FXString fxlabel =  pFf->GetLabel();
          fxlabel += ": Absolute";
          const char * pLabel = fxlabel;
          pAbsolutePt = CreatePtFrame(pt,VideoFrame->GetTime(),"0xff0000" , pLabel );

          pAbsolutePt->Add(CDPoint(SourceCountour.Right_Abs.x,SourceCountour.Right_Abs.y));
          pAbsolutePt->Add(CDPoint(SourceCountour.Down_Abs.x,SourceCountour.Down_Abs.y));
          pAbsolutePt->Add(CDPoint(SourceCountour.Left_Abs.x,SourceCountour.Left_Abs.y));
          pAbsolutePt->Add(CDPoint(SourceCountour.Up_Abs.x,SourceCountour.Up_Abs.y));

        }

        if(pRelativePt == NULL)
        {
          cmplx pt( SourceCountour.Up_Rel.x, SourceCountour.Up_Rel.y);
          FXString fxlabel =  pFf->GetLabel();
          fxlabel += ": Relative";
          const char * pLabel = fxlabel;
          pRelativePt = CreatePtFrame(pt,VideoFrame->GetTime(),"0x0000ff" , pLabel );

          pRelativePt->Add(CDPoint(SourceCountour.Right_Rel.x,SourceCountour.Right_Rel.y)); 
          pRelativePt->Add(CDPoint(SourceCountour.Down_Rel.x,SourceCountour.Down_Rel.y));
          pRelativePt->Add(CDPoint(SourceCountour.Left_Rel.x,SourceCountour.Left_Rel.y));
          pRelativePt->Add(CDPoint(SourceCountour.Up_Rel.x,SourceCountour.Up_Rel.y));
        }

        if(pRelativePt)
          pResult->AddFrame(pRelativePt);
        if (pAbsolutePt) 
          pResult->AddFrame(pAbsolutePt);
      }
    } while(pFf != NULL);

  }
//  if(pRelativePt)
//     pResult->AddFrame(pRelativePt);
//  if (pAbsolutePt) 
//    pResult->AddFrame(pAbsolutePt);
// 

 pResult->AddFrame(VideoFrame);
 m_pOutput->Put(pResult); 
 //return pResult;
 CVideoFrame* retV = NULL;
 return retV;
}

void RadiusExtracterGadget::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame) 
{
  pParamFrame->Release( pParamFrame );
};

void RadiusExtracterGadget::PropertiesRegistration() 
{

};


void RadiusExtracterGadget::ConnectorsRegistration() 
{
  addInputConnector( transparent, "InputName1");
  addOutputConnector( createComplexDataType(3, rectangle, text, vframe) , "OutputName1");
};




void RadiusExtracterGadget::GetFigureCenterandAngle(const CDataFrame* pDataFrame, CFigureFrame * pFf, double &dAngle, CDPoint &cdCenterPoint)
{
  int iIndex;
  CFramesIterator* TextIterator = pDataFrame->CreateFramesIterator(text);
  if (TextIterator!=NULL)
  {
    CTextFrame* TextFrame = (CTextFrame*)TextIterator->Next();
    while (TextFrame)
    {
      FXString Data;

      FXString fxText = TextFrame->GetString();
      if(fxText.Find("Spot")>-1)
      {
        FXSIZE iTok = 0;

        Data = fxText.Tokenize(":",iTok); 
        if(iTok!=-1)
          Data = fxText.Tokenize(":",iTok); 


        double d1,d2,d3,d4,d5,d6,d7,d8;
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
}
void RadiusExtracterGadget::FindFigureHeightandWidth(Contour &SourceCountour)
{
  SourceCountour.dCenter.y-=10;
  CDPoint UpRel,UpAbs,DownRel,DownAbs,RightRel,RightAbs,LeftRel,LeftAbs;
  double dSlope = tan(SourceCountour.dAngle * M_PI / 180);
  dSlope *= -1;//Inverse Axis
  double dInter = SourceCountour.dCenter.y - dSlope * SourceCountour.dCenter.x;

  double dSlopePer = -1/dSlope;
  double dInterPer = SourceCountour.dCenter.y - dSlopePer * SourceCountour.dCenter.x;

  double dPrevLongRadMinDelta_Rel = 0;
  double dPrevLongRadMaxDelta_Rel = 0;//

  double dPrevShortRadMinDelta_Rel = 0; 
  double dPrevShortRadMaxDelta_Rel = 0;// 
 
  double dPrevLongRadMaxDelta_Abs = 0;
  double dPrevLongRadMinDelta_Abs = 0;

  double dPrevShortRadMaxDelta_Abs = 0;
  double dPrevShortRadMinDelta_Abs = 0;

  int iprevSignLongRad  = 0;
  int iprevSignShortRad = 0;

//   FILE* fw;
//   fopen_s( &fw, (LPCTSTR)"d:\\contur.csv", "w" );
//   if(fw)
//     fclose(fw);
  FXArray <CDPoint> ExtrPointsArrLong;
  FXArray <CDPoint> ExtrPointsArrShort;

  for (int icntr = 0 ; icntr < SourceCountour.CountourVector.GetSize(); icntr++)
  {

    double dContourX = SourceCountour.CountourVector.GetAt(icntr).x;
    double dContourY = SourceCountour.CountourVector.GetAt(icntr).y;

//     fopen_s( &fw, (LPCTSTR)"d:\\contur.csv", "a" );
//     if ( fw )
//     {
//       CString sout;
//       sout.Format("%6.2lf,%6.2lf,\n",dContourX,dContourY);
//       fputs( (LPCTSTR)sout, fw ) ;
//       fclose(fw);
//     } 

    double dFitLineY = dSlope * dContourX + dInter;
    double dPerFitLineY  =  dSlopePer * dContourX + dInterPer;

    double dDelta = dContourY - dFitLineY;
    double dDeltaPer = dContourY - dPerFitLineY;


    int icurSignLongRad  = 0;
    int icurSignShortRad = 0;

     if(dPrevLongRadMinDelta_Rel == 0)
       dPrevLongRadMinDelta_Rel = dDelta;

     if(dPrevShortRadMinDelta_Rel == 0)
       dPrevShortRadMinDelta_Rel = dDeltaPer;


     if(dPrevLongRadMaxDelta_Rel == 0)
       dPrevLongRadMaxDelta_Rel = dDelta;

     if(dPrevShortRadMaxDelta_Rel == 0)
       dPrevShortRadMaxDelta_Rel = dDeltaPer;


     if(dPrevLongRadMaxDelta_Abs == 0)
         dPrevLongRadMaxDelta_Abs = dDeltaPer;

     if(dPrevLongRadMinDelta_Abs == 0)
       dPrevLongRadMinDelta_Abs = dDeltaPer;

     if(dPrevShortRadMaxDelta_Abs == 0)
       dPrevShortRadMaxDelta_Abs = dDelta;

     if(dPrevShortRadMinDelta_Abs == 0)
       dPrevShortRadMinDelta_Abs = dDelta;

      
     if(dDelta > 0)
       icurSignLongRad = 1;
     else
       icurSignLongRad = -1;

     if(dDeltaPer > 0)
       icurSignShortRad = 1;
     else
       icurSignShortRad = -1;
     
     
     if(iprevSignLongRad == 0)
     {
       if(dDelta > 0)
        iprevSignLongRad = 1;
       else
        iprevSignLongRad = -1;
     }

     if(iprevSignShortRad == 0)
     {
       if(dDeltaPer > 0)
         iprevSignShortRad = 1;
       else
         iprevSignShortRad = -1;
     }

/*   
    //Long Radius - Relative
    if(abs(dDelta) < abs(dPrevLongRadMinDelta_Rel) && dContourX < SourceCountour.dCenter.x)
    {
      //DownRel = UpRel;
      UpRel.x = dContourX, UpRel.y = dContourY;
      dPrevLongRadMinDelta_Rel = dDelta;
    }

    if(abs(dDelta) < abs(dPrevLongRadMaxDelta_Rel) && dContourX > SourceCountour.dCenter.x)
    {
      DownRel.x = dContourX, DownRel.y = dContourY;
      dPrevLongRadMaxDelta_Rel = dDelta;
    }

    //Short Radius - Relative
    if(abs(dDeltaPer) < abs(dPrevShortRadMinDelta_Rel) && dContourX < SourceCountour.dCenter.x)
    {

      LeftRel.x = dContourX, LeftRel.y = dContourY;
      dPrevShortRadMinDelta_Rel = dDeltaPer;
    }

    if(abs(dDeltaPer) < abs(dPrevShortRadMaxDelta_Rel) && dContourX > SourceCountour.dCenter.x)
    {
    RightRel.x = dContourX, RightRel.y = dContourY;
    dPrevShortRadMaxDelta_Rel = dDeltaPer;
    }
    */ 

     if(icurSignLongRad != iprevSignLongRad)
       ExtrPointsArrLong.Add(CDPoint(dContourX,dContourY));

     if(icurSignShortRad != iprevSignShortRad)
       ExtrPointsArrShort.Add(CDPoint(dContourX,dContourY));

     iprevSignLongRad   = icurSignLongRad;
     iprevSignShortRad  = icurSignShortRad;



     //Long Radius - Absolute
     if((dPrevLongRadMaxDelta_Abs) < (dDeltaPer))
     {
       UpAbs.x = dContourX, UpAbs.y = dContourY;
      dPrevLongRadMaxDelta_Abs = dDeltaPer; 
    } 

    if((dPrevLongRadMinDelta_Abs) > (dDeltaPer))
    {
      DownAbs.x = dContourX, DownAbs.y = dContourY;
      dPrevLongRadMinDelta_Abs = dDeltaPer; 
    } 

    //Short Radius - Absolute
    if((dPrevShortRadMaxDelta_Abs) < (dDelta))
    {
      RightAbs.x = dContourX, RightAbs.y = dContourY;
      dPrevShortRadMaxDelta_Abs = dDelta;
    }
    if((dPrevShortRadMinDelta_Abs) > (dDelta))
    {
      LeftAbs.x = dContourX, LeftAbs.y = dContourY;
      dPrevShortRadMinDelta_Abs = dDelta;
    }
  }

   GetFarestPoints(ExtrPointsArrLong,UpRel,DownRel);
   GetFarestPoints(ExtrPointsArrShort,RightRel,LeftRel);


  if (UpRel.y > DownRel.y)
  {
    SourceCountour.Up_Rel = UpRel;
    SourceCountour.Down_Rel = DownRel;
  }
  else
  {
    SourceCountour.Up_Rel = DownRel;
    SourceCountour.Down_Rel = UpRel;
  }

  if (UpAbs.y > DownAbs.y)
  {
    SourceCountour.Up_Abs = UpAbs;
    SourceCountour.Down_Abs = DownAbs;
  }
  else
  {
    SourceCountour.Up_Abs = DownAbs;
    SourceCountour.Down_Abs = UpAbs;
  }

  if (RightRel.x > LeftRel.x)
  {
    SourceCountour.Left_Rel = LeftRel;
    SourceCountour.Right_Rel = RightRel;
  }
  else
  {
    SourceCountour.Left_Rel = RightRel;
    SourceCountour.Right_Rel = LeftRel;
  }


  if (RightAbs.x > LeftAbs.x)
  {
    SourceCountour.Left_Abs = LeftAbs;
    SourceCountour.Right_Abs = RightAbs;
  }
  else
  {
    SourceCountour.Left_Abs = RightAbs;
    SourceCountour.Right_Abs = LeftAbs;
  }

 SourceCountour.dWidthRel  =  GetDistance(RightRel,LeftRel);
 SourceCountour.dHeightRel =  GetDistance(UpRel,DownRel);

 SourceCountour.dWidthAbs  =  GetDistance(RightAbs,LeftAbs);
 SourceCountour.dHeightAbs =  GetDistance(UpAbs,DownAbs);
  
//   FILE* fcsv;
//   fopen_s( &fcsv, (LPCTSTR)"d:\\coord.csv", "wt" );
//   if ( fcsv )
//   { 
//     CString ResString;
// 
//     ResString.Format("RightRel,%6.2lf,%6.2lf\n,",RightRel.x,RightRel.y);
//     fputs( (LPCTSTR)ResString, fcsv ) ;
// 
//     ResString.Format("LeftRel,%6.2lf,%6.2lf\n,",LeftRel.x,LeftRel.y);
//     fputs( (LPCTSTR)ResString, fcsv ) ;
// 
//     ResString.Format("RightAbs,%6.2lf,%6.2lf\n,",RightAbs.x,RightAbs.y);
//     fputs( (LPCTSTR)ResString, fcsv ) ;
// 
//     ResString.Format("LeftAbs,%6.2lf,%6.2lf\n,",LeftAbs.x,LeftAbs.y);
//     fputs( (LPCTSTR)ResString, fcsv ) ;
// 
// 
// 
// 
//     ResString.Format("DowntRel,%6.2lf,%6.2lf\n,",DownRel.x,DownRel.y);
//     fputs( (LPCTSTR)ResString, fcsv ) ;
// 
//     ResString.Format("UpRel,%6.2lf,%6.2lf\n,",UpRel.x,UpRel.y);
//     fputs( (LPCTSTR)ResString, fcsv ) ;
// 
//     ResString.Format("DownAbs,%6.2lf,%6.2lf\n,",DownAbs.x,DownAbs.y);
//     fputs( (LPCTSTR)ResString, fcsv ) ;
// 
//     ResString.Format("UpAbs,%6.2lf,%6.2lf\n,",UpAbs.x,UpAbs.y);
//     fputs( (LPCTSTR)ResString, fcsv ) ;
// 
// 
//     fclose( fcsv );
//   }

}

void RadiusExtracterGadget::GetFarestPoints(FXArray <CDPoint> &ExPtArr, CDPoint &pt1, CDPoint &pt2)
{
   CDPoint tmp1,tmp2;
   double dMaxDist = -1;
   int isz = (int) ExPtArr.GetSize();
   for(int i = 0; i < isz;i++ )
   { 
     tmp1 = ExPtArr.GetAt(i);
      for (int j = i+1 ; j < isz ;j++)
      {
        tmp2 = ExPtArr.GetAt(j);
        double dDist = GetDistance(tmp1,tmp2);
        if (dDist > dMaxDist)
        {
          dMaxDist = dDist;
          pt1 = tmp1;
          pt2 = tmp2;
        }
      }
   }
}

double RadiusExtracterGadget::GetDistance(CDPoint &pt1, CDPoint &pt2)
{
  return sqrt((pt1.x - pt2.x) * (pt1.x - pt2.x) + (pt1.y - pt2.y) * (pt1.y - pt2.y));
}