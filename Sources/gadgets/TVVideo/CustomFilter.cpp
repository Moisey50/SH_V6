// CustomFilter.h : Implementation of the CustomFilter class


#include "StdAfx.h"
#include "CustomFilter.h"
#include <Gadgets\VideoFrame.h>
#include <Gadgets\vftempl.h>

IMPLEMENT_RUNTIME_GADGET_EX(CustomFilter, CFilterGadget, "Video.convolution", TVDB400_PLUGIN_NAME);

CustomFilter::CustomFilter(void):
    m_Matrix(0,0,0,
             0,1,0,
             0,0,-1),
    m_Offset(128)
{
    m_MultyCoreAllowed=true;
    m_pInput = new CInputConnector(vframe);
    m_pOutput = new COutputConnector(vframe);
    Resume();
}

void CustomFilter::ShutDown()
{
    CFilterGadget::ShutDown();
	delete m_pInput;
	m_pInput = NULL;
	delete m_pOutput;
	m_pOutput = NULL;
}

CDataFrame* CustomFilter::DoProcessing(const CDataFrame* pDataFrame)
{
    const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame(DEFAULT_LABEL);
	PASSTHROUGH_NULLFRAME(VideoFrame, pDataFrame);
    CVideoFrame* retVal=CVideoFrame::Create(_customfilter(VideoFrame, m_Matrix, m_Offset));
        retVal->CopyAttributes(pDataFrame);;
    return retVal;
}

bool CustomFilter::ScanProperties(LPCTSTR text, bool& Invalidate)
{
    MatrixType box=m_Matrix.GetMatrixType();
    CustomMatrix cmatrix;

    memset(&cmatrix,0,sizeof(cmatrix));
    FXPropertyKit pk(text);
    
    pk.GetInt("Box",(int&)box);
    Invalidate=(box!=m_Matrix.GetMatrixType());
    bool bRes=true;
    switch (box)
    {
        case type3x3:
            for (int i=0; i<3; i++)
            {
                FXString inS;
                FXString iName; iName.Format("Matrix%d",i);
                if (pk.GetString(iName,inS))
                {
                bRes&=(sscanf(inS,"%lf,%lf,%lf",
                    &cmatrix.m_3x3[i][0],&cmatrix.m_3x3[i][1],&cmatrix.m_3x3[i][2])==3);
            }
                else
                {
                    cmatrix.m_3x3[i][0]=m_Matrix.m_Matrix.m_3x3[i][0];
                    cmatrix.m_3x3[i][1]=m_Matrix.m_Matrix.m_3x3[i][1];
                    cmatrix.m_3x3[i][2]=m_Matrix.m_Matrix.m_3x3[i][2];
                }
            }
            if (bRes)
                m_Matrix.SetMatrix( cmatrix.m_3x3[0][0],cmatrix.m_3x3[0][1],cmatrix.m_3x3[0][2],
                                cmatrix.m_3x3[1][0],cmatrix.m_3x3[1][1],cmatrix.m_3x3[1][2],
                                cmatrix.m_3x3[2][0],cmatrix.m_3x3[2][1],cmatrix.m_3x3[2][2]);
            else
                m_Matrix.SetMatrix(0,0,0,
                                   0,1,0,
                                   0,0,0);
            break;
        case type5x5:
            for (int i=0; i<5; i++)
            {
                FXString inS;
                FXString iName; iName.Format("Matrix%d",i);
                if (pk.GetString(iName,inS))
                bRes&=(sscanf(inS,"%lf,%lf,%lf,%lf,%lf",
                    &cmatrix.m_5x5[i][0],&cmatrix.m_5x5[i][1],&cmatrix.m_5x5[i][2],&cmatrix.m_5x5[i][3],&cmatrix.m_5x5[i][4])==5);
                else
                {
                    cmatrix.m_5x5[i][0]=m_Matrix.m_Matrix.m_5x5[i][0];
                    cmatrix.m_5x5[i][1]=m_Matrix.m_Matrix.m_5x5[i][1];
                    cmatrix.m_5x5[i][2]=m_Matrix.m_Matrix.m_5x5[i][2];
                    cmatrix.m_5x5[i][3]=m_Matrix.m_Matrix.m_5x5[i][3];
                    cmatrix.m_5x5[i][4]=m_Matrix.m_Matrix.m_5x5[i][4];
                }
            }
            if (bRes)
                m_Matrix.SetMatrix( cmatrix.m_5x5[0][0],cmatrix.m_5x5[0][1],cmatrix.m_5x5[0][2],cmatrix.m_5x5[0][3],cmatrix.m_5x5[0][4],
                                cmatrix.m_5x5[1][0],cmatrix.m_5x5[1][1],cmatrix.m_5x5[1][2],cmatrix.m_5x5[1][3],cmatrix.m_5x5[1][4],
                                cmatrix.m_5x5[2][0],cmatrix.m_5x5[2][1],cmatrix.m_5x5[2][2],cmatrix.m_5x5[2][3],cmatrix.m_5x5[2][4],
                                cmatrix.m_5x5[3][0],cmatrix.m_5x5[3][1],cmatrix.m_5x5[3][2],cmatrix.m_5x5[3][3],cmatrix.m_5x5[3][4],
                                cmatrix.m_5x5[4][0],cmatrix.m_5x5[4][1],cmatrix.m_5x5[4][2],cmatrix.m_5x5[4][3],cmatrix.m_5x5[4][4]);
            else
                m_Matrix.SetMatrix(0,0,0,0,0,
                                   0,0,0,0,0,
                                   0,0,1,0,0,
                                   0,0,0,0,0,
                                   0,0,0,0,0);
            break;
        case type7x7:
            for (int i=0; i<7; i++)
            {
                FXString inS;
                FXString iName; iName.Format("Matrix%d",i);
                if (pk.GetString(iName,inS))
                bRes&=(sscanf(inS,"%lf,%lf,%lf,%lf,%lf,%lf,%lf",
                    &cmatrix.m_7x7[i][0],&cmatrix.m_7x7[i][1],&cmatrix.m_7x7[i][2],&cmatrix.m_7x7[i][3],&cmatrix.m_7x7[i][4],&cmatrix.m_7x7[i][5],&cmatrix.m_7x7[i][6])==7);
                else
                {
                    cmatrix.m_7x7[i][0]=m_Matrix.m_Matrix.m_7x7[i][0];
                    cmatrix.m_7x7[i][1]=m_Matrix.m_Matrix.m_7x7[i][1];
                    cmatrix.m_7x7[i][2]=m_Matrix.m_Matrix.m_7x7[i][2];
                    cmatrix.m_7x7[i][3]=m_Matrix.m_Matrix.m_7x7[i][3];
                    cmatrix.m_7x7[i][4]=m_Matrix.m_Matrix.m_7x7[i][4];
                    cmatrix.m_7x7[i][5]=m_Matrix.m_Matrix.m_7x7[i][5];
                    cmatrix.m_7x7[i][6]=m_Matrix.m_Matrix.m_7x7[i][6];
                }
            }
            if (bRes)
                m_Matrix.SetMatrix( cmatrix.m_7x7[0][0],cmatrix.m_7x7[0][1],cmatrix.m_7x7[0][2],cmatrix.m_7x7[0][3],cmatrix.m_7x7[0][4],cmatrix.m_7x7[0][5],cmatrix.m_7x7[0][6],
                                cmatrix.m_7x7[1][0],cmatrix.m_7x7[1][1],cmatrix.m_7x7[1][2],cmatrix.m_7x7[1][3],cmatrix.m_7x7[1][4],cmatrix.m_7x7[1][5],cmatrix.m_7x7[1][6],
                                cmatrix.m_7x7[2][0],cmatrix.m_7x7[2][1],cmatrix.m_7x7[2][2],cmatrix.m_7x7[2][3],cmatrix.m_7x7[2][4],cmatrix.m_7x7[2][5],cmatrix.m_7x7[2][6],
                                cmatrix.m_7x7[3][0],cmatrix.m_7x7[3][1],cmatrix.m_7x7[3][2],cmatrix.m_7x7[3][3],cmatrix.m_7x7[3][4],cmatrix.m_7x7[3][5],cmatrix.m_7x7[3][6],
                                cmatrix.m_7x7[4][0],cmatrix.m_7x7[4][1],cmatrix.m_7x7[4][2],cmatrix.m_7x7[4][3],cmatrix.m_7x7[4][4],cmatrix.m_7x7[4][5],cmatrix.m_7x7[4][6],
                                cmatrix.m_7x7[5][0],cmatrix.m_7x7[5][1],cmatrix.m_7x7[5][2],cmatrix.m_7x7[5][3],cmatrix.m_7x7[5][4],cmatrix.m_7x7[5][5],cmatrix.m_7x7[5][6],
                                cmatrix.m_7x7[6][0],cmatrix.m_7x7[6][1],cmatrix.m_7x7[6][2],cmatrix.m_7x7[6][3],cmatrix.m_7x7[6][4],cmatrix.m_7x7[6][5],cmatrix.m_7x7[6][6]);
            else
                m_Matrix.SetMatrix(0,0,0,0,0,0,0,
                                   0,0,0,0,0,0,0,
                                   0,0,0,0,0,0,0,
                                   0,0,0,1,0,0,0,
                                   0,0,0,0,0,0,0,
                                   0,0,0,0,0,0,0,
                                   0,0,0,0,0,0,0);
            break;
    }
    pk.GetInt("Offset",m_Offset);
	return true;
}

bool CustomFilter::PrintProperties(FXString& text)
{
    FXPropertyKit pc;
    
    pc.WriteInt("Box",m_Matrix.GetMatrixType());
    
    switch (m_Matrix.GetMatrixType())
    {
    case type3x3:
        for (int i=0; i<3; i++)
        {
            CString iName; iName.Format("Matrix%d",i);
            CString outS;
            outS.Format("%f,%f,%f",
                         m_Matrix.Get(i,0),m_Matrix.Get(i,1),m_Matrix.Get(i,2));
            pc.WriteString(iName,outS);
        }
        break;
    case type5x5:
        for (int i=0; i<5; i++)
        {
            CString iName; iName.Format("Matrix%d",i);
            CString outS;
            outS.Format("%f,%f,%f,%f,%f",
                         m_Matrix.Get(i,0),m_Matrix.Get(i,1),m_Matrix.Get(i,2),m_Matrix.Get(i,3),m_Matrix.Get(i,4));
            pc.WriteString(iName,outS);
        }
        break;
    case type7x7:
        for (int i=0; i<7; i++)
        {
            CString iName; iName.Format("Matrix%d",i);
            CString outS;
            outS.Format("%f,%f,%f,%f,%f,%f,%f",
                         m_Matrix.Get(i,0),m_Matrix.Get(i,1),m_Matrix.Get(i,2),m_Matrix.Get(i,3),m_Matrix.Get(i,4),m_Matrix.Get(i,5),m_Matrix.Get(i,6));
            pc.WriteString(iName,outS);
        }
        break;
    }
    pc.WriteInt("Offset",m_Offset);
    text=pc;
    return true;
}

bool CustomFilter::ScanSettings(FXString& text)
{
    text.Format("template(ComboBox(Box(3x3(%d),5x5(%d),7x7(%d))),",type3x3,type5x5,type7x7);
    switch (m_Matrix.GetMatrixType())
    {
    case type3x3:
        for (int i=0; i<3; i++)
        {
            FXString iName; iName.Format("Matrix%d",i);
            text+="EditBox("+iName+"),";
        }
        break;
    case type5x5:
        for (int i=0; i<5; i++)
        {
            FXString iName; iName.Format("Matrix%d",i);
            text+="EditBox("+iName+"),";
        }
        break;
    case type7x7:
        for (int i=0; i<7; i++)
        {
            FXString iName; iName.Format("Matrix%d",i);
            text+="EditBox("+iName+"),";
        }
        break;
    }
    text+="Spin(Offset,-1000,1000)";
    text+=")";
    return true;
}

