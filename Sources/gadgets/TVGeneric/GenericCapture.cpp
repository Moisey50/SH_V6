// GenericCapture.cpp: implementation of the GenericCapture class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GenericCapture.h"
#include <gadgets\TextFrame.h>
#include <gadgets\QuantityFrame.h>
#include <gadgets\VideoFrame.h>
#include <gadgets\RectFrame.h>
#include <files\imgfiles.h>
#include <helpers\FXParser2.h>
#include <helpers/FramesHelper.h>


#define THIS_MODULENAME "GenericCapture"

IMPLEMENT_RUNTIME_GADGET_EX(GenericCapture, CCtrlGadget, LINEAGE_GENERIC, TVDB400_PLUGIN_NAME);

void __stdcall KnobCallback(UINT nID, int nCode, void* cbParam, void* pParam)
{
    ((GenericCapture*)cbParam)->OnCommand(nID, nCode);
}

void __stdcall GenericCaptureOnInput(CDataFrame* lpData, void* lpParam, CConnector* lpInput)
{
    ((GenericCapture*)lpParam)->OnInput(lpData, lpInput);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

GenericCapture::GenericCapture():
    m_DataType(text),
    m_NoID(FALSE),
    m_Manual(TRUE),
    m_FramesCounter(0),
    m_FrameRate(25),
    m_Replace(FALSE),
    m_SetEOS( FALSE ) ,
    m_SetRegistredFlag(FALSE),
    m_bIgnoreInputConnected( FALSE )
{
    SetMonitor(SET_INPLACERENDERERMONITOR);
    m_ButtonName= "Send";
    UseFileFilters(); 
    m_pOutput   = new COutputConnector( transparent );
    m_pInput    = new CInputConnector(nulltype,GenericCaptureOnInput,this);
}

void    GenericCapture::Attach(CWnd* pWnd)
{
  Detach();

  CCtrlGadget::Create();
  CCtrlGadget::Attach(pWnd);

  CRect rc;
  pWnd->GetClientRect(rc);
  m_Proxy.Create(pWnd);
  m_Button.Create(m_ButtonName,WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, rc, &m_Proxy, 13); 
  m_Proxy.Init(&m_Button,KnobCallback,this);
  //m_Button.EnableWindow(FALSE);
  UpdateBtnStatus();
  if ((m_pStatus) && (m_pStatus->GetStatus()==CExecutionStatus::RUN))
      OnStart();
}

void    GenericCapture::Detach()
{
	if (m_Button.GetSafeHwnd())
		m_Button.DestroyWindow();
	if (m_Proxy.GetSafeHwnd())
		m_Proxy.DestroyWindow();
}

void GenericCapture::ShutDown()
{
    //VERIFY(m_Lock.LockAndProcMsgs());

    CCtrlGadget::ShutDown();
	delete m_pOutput; m_pOutput=NULL;
    delete m_pInput;  m_pInput=NULL;
    FileFiltersDone();
    
    //m_Lock.Unlock();
}

void GenericCapture::OnStop() 
{ 
    CCaptureGadget::OnStop();
    m_FramesCounter=0; 
    if (IsValid())
        UpdateBtnStatus(); 
}

void GenericCapture::OnStart() 
{ 
    CCaptureGadget::OnStart();
    UpdateBtnStatus();
    if ( m_bSendOnStart )
    {
      CDataFrame* retV = PrepareNextFrame();
      PutFrame( GetOutputConnector( 0 ) , retV ) ;
    }
}

CDataFrame* GenericCapture::PrepareNextFrame()
{
    FXAutolock al(m_Lock);
    CDataFrame* retV=NULL;
    if ( m_NoID )
      CCaptureGadget::m_FrameCounter = -1 ;
    switch (m_DataType)
    {
    case nulltype:
        {
            CDataFrame* pDataFrame = CDataFrame::Create();
            pDataFrame->ChangeId((m_NoID)?0:m_FramesCounter);
            pDataFrame->SetTime(GetGraphTime() * 1.e-3 );
            retV=pDataFrame;
            break;
        }
    case logical:
        {
            CBooleanFrame* pDataFrame = CBooleanFrame::Create(m_Bool);
            pDataFrame->ChangeId((m_NoID)?0:m_FramesCounter);
            pDataFrame->SetTime(GetGraphTime() * 1.e-3 );
            retV=pDataFrame;
            break;
        }
    case rectangle:
        {
            CRectFrame* pDataFrame = CRectFrame::Create(m_Rect);
            pDataFrame->ChangeId((m_NoID)?0:m_FramesCounter);
            pDataFrame->SetTime(GetGraphTime() * 1.e-3 );
            retV=pDataFrame;
            break;
        }
    case text:
        {
            CTextFrame* pDataFrame = CTextFrame::Create(m_Message);

            pDataFrame->ChangeId((m_NoID)?0:m_FramesCounter);
            pDataFrame->SetTime(GetGraphTime() * 1.e-3 );
            retV=pDataFrame;
            break;
        }
    case quantity:
        {
            CQuantityFrame* pDataFrame = CQuantityFrame::Create(m_Quantity);
        
            pDataFrame->ChangeId((m_NoID)?0:m_FramesCounter);
            pDataFrame->SetTime(GetGraphTime() * 1.e-3 );
            retV=pDataFrame;
            break;
        }
    case vframe:
        {
            if (m_FL.GetSize()!=0)
            {
                pTVFrame frame=(pTVFrame)malloc(sizeof(TVFrame));
                frame->lpBMIH=loadDIB(m_FL.Get());
                if (!frame->lpBMIH)
                {
                    free(frame);
                    retV=NULL;
                }
                else
                {
                    frame->lpData=NULL;
                    CVideoFrame* vf=CVideoFrame::Create(frame);
                    vf->ChangeId((m_NoID)?0:m_FramesCounter);
                    vf->SetTime(GetGraphTime() * 1.e-3 );
                    retV=vf;
                }
                if (!m_FL.Next())
                    m_FL.SetFirst();
            }
            break;
        }
    case figure:
        {
            CFigureFrame* pDataFrame= CFigureFrame::Create(&m_Figure);
            pDataFrame->ChangeId((m_NoID)?0:m_FramesCounter);
            pDataFrame->SetTime(GetGraphTime() * 1.e-3 );
            retV=pDataFrame;
            break;
        }
    }
    if (retV)
    {

        if (m_Attributes.GetLength())
        {
            *retV->Attributes()=m_Attributes;
        }
        if (m_Label.GetLength())
        {
            retV->SetLabel(m_Label);
        }
        if (m_SetEOS)
        {
            retV->ChangeId( END_OF_STREAM_MARK) ;
        } 
        if (m_SetRegistredFlag)
        {
            retV->SetRegistered();
        } 
    }
    return retV;
}


CDataFrame* GenericCapture::GetNextFrame(double* StartTime)
{
    if ((m_Manual) || ( !m_bIgnoreInputConnected && IsInputConnected())) 
    {
        SetTicksIdle(1000);
        //Sleep(10);
        return NULL;
    }
    else
        SetTicksIdle(1000/m_FrameRate);
    CDataFrame* retV = PrepareNextFrame();
    m_FramesCounter++;
    return retV;
}

void GenericCapture::OnCommand(UINT nID, int nCode)
{
    if ((nID==13) && 
        (nCode==BN_CLICKED) && 
        ((m_pStatus->GetStatus()==CExecutionStatus::RUN) ||
        (m_pStatus->GetStatus()==CExecutionStatus::PAUSE))
       )
    {
        if (m_Manual)
        {
            CDataFrame* retV=PrepareNextFrame();
                if ((retV) && ((!m_pOutput) || (!m_pOutput->Put(retV))))
                    retV->RELEASE(retV);
            m_FramesCounter++;
        }
    }
}

void GenericCapture::OnInput(CDataFrame* lpData, CConnector* lpInput)
{
  if ( !lpData )
    return ;
    if (Tvdb400_IsEOS(lpData))
    {
        if (((!m_pOutput) || (!m_pOutput->Put(lpData))))
            lpData->RELEASE(lpData);
        return ;
    }
    else if (  lpData->GetDataType() == text )
    {
      FXString Label( lpData->GetLabel() ) ;
      if ( Label == _T("Control") )
      {
        CTextFrame * tf = lpData->GetTextFrame() ;
        if ( tf )
        {
          bool Invalidate = false ;
          ScanProperties( (LPCTSTR)tf->GetString() , Invalidate ) ;
          lpData->Release(lpData) ;
          return ;
        }
      }
    }
    if (m_Replace)
    {
      CBooleanFrame * pBoolean = lpData->GetBooleanFrame() ;
      if ( pBoolean && ((bool)*pBoolean == false) ) // nothing to send if false
      {
      }
      else
      {
        CDataFrame* retV=PrepareNextFrame();
        if ( !m_NoID )
        {
          retV->ChangeId( lpData->GetId() ) ;
          retV->SetTime( lpData->GetTime() ) ;
        }
        lpData->Release(lpData);
        if ((retV) && ((!m_pOutput) || (!m_pOutput->Put(retV))))
            retV->RELEASE(retV);
        m_FramesCounter++;
      }
    }
    else
    {
        if (m_NoID)
            lpData->ChangeId(0);
        if (m_Label.GetLength())
            lpData->SetLabel(m_Label);
        if (m_Attributes.GetLength())
            *(lpData->Attributes())=m_Attributes;
        if ((!m_pOutput) || (!m_pOutput->Put(lpData)))
            lpData->RELEASE(lpData);
    }
}

bool GenericCapture::ScanSettings(FXString& txt)
{
    txt.Format("template(EditBox(Message),"
      "ComboBox(DataType(nulltype(%d),logical(%d),rectangle(%d),text(%d),quantity(%d),vframe(%d),figure(%d))),"
      "ComboBox(NoSync(False(%d),True(%d))),ComboBox(Manual(False(%d),True(%d))),"
      "Spin(FrameRate,1,25),ComboBox(Replace(False(%d),True(%d))),"
      "EditBox(Attributes),EditBox(Label),"
      "ComboBox(SendOnStart(False(0),True(1))),"
      "ComboBox(SetEOS(False(%d),True(%d))),"
      "ComboBox(SetRegistered(False(%d),True(%d))),"
      "ComboBox(IgnoreInConnect(False(%d),True(%d))))" ,
     (int)nulltype,(int)logical,(int)rectangle,(int)text,(int)quantity,(int)vframe,(int)figure, 
     FALSE,   TRUE,                      
     FALSE,   TRUE, 
     FALSE,   TRUE, 
     FALSE,   TRUE,
     0,   1);
    return true;
}

bool GenericCapture::ScanProperties(LPCTSTR txt, bool& Invalidate)
{
    FXAutolock al(m_Lock);
    CCtrlGadget::ScanProperties(txt, Invalidate);
    FXPropertyKit pk(txt);
    pk.GetString("Message",m_Cmd);
    pk.GetString("Attributes",m_Attributes);
    pk.GetString("Label",m_Label);
    pk.GetInt("NoSync",(int&)m_NoID);   
    pk.GetInt( "SendOnStart" , ( int& ) m_bSendOnStart );
    pk.GetInt( "SetEOS" , ( int& ) m_SetEOS );
    pk.GetInt( "SetRegistered" , ( int& )m_SetRegistredFlag );
    pk.GetInt("Manual",(int&)m_Manual);
    pk.GetInt("Replace",(int&)m_Replace);
    pk.GetInt( "IgnoreInConnect"  , (int&)m_bIgnoreInputConnected ) ;
    if ( pk.GetInt("FrameRate",m_FrameRate) )
      SetTicksIdle(1000/m_FrameRate);
    pk.GetInt("DataType",(int&)m_DataType);

    if (IsCtrlReady())
    {
        m_Button.EnableWindow(
            ((m_pStatus->GetStatus()==CExecutionStatus::RUN) ||
             (m_pStatus->GetStatus()==CExecutionStatus::PAUSE))
            ?TRUE:FALSE);
        m_Button.ShowWindow((m_Manual)?SW_SHOW:SW_HIDE);
    }

    switch (m_DataType)
    {
        case nulltype:
        {
            break;
        }
        case logical:
        {
            m_Bool=false;
            m_Cmd.GetBool("Bool",m_Bool);
            break;
        }
        case rectangle:
        {
            FXString tmpS;
            m_Cmd.GetString("rect",tmpS);
            int i=sscanf(tmpS,"%d,%d,%d,%d",&m_Rect.left,&m_Rect.top,&m_Rect.right,&m_Rect.bottom);
            if (i!=4)
            {
                SENDERR_0("Wrong sintax of 'rect' command");
                m_Rect.left=m_Rect.top=m_Rect.right=m_Rect.bottom=-1;
            }
            break;
        }
        case vframe:
        {
            FXString Filename;
            m_Cmd.GetString("VidFrame",Filename);
            FXString fExt=FxGetFileExtension(Filename);
            if ((fExt.GetLength()==0) || (fExt=="*"))
                fExt=GetInputFileFilter();
            m_FL.Dir(Filename,fExt);
            break;
        }
        case quantity:
        {
            m_Quantity=0;
            m_Cmd.GetDouble("Quantity",m_Quantity);
            break;
        }
        case text:
        {
            FXString Text ;
            m_Cmd.GetString("TextData",Text);
            m_Message.Empty();
            ConvViewStringToBin( Text , m_Message ) ;
            break;
        }
        case figure:
        {
            m_Figure.RemoveAll();
            FXString tmpS;
            m_Cmd.GetString("Points",tmpS);
            while (tmpS.GetLength())
            {
                CDPoint dPoint;
                FXSIZE p=tmpS.Find(",");
                if (p>=0)
                {
                    FXString sd=tmpS.Left(p);
                    dPoint.x=atof(sd);
                    tmpS=tmpS.Mid(p+1);
                    p=tmpS.Find(",");
                    if (p>=0)
                    {
                        CString sd=tmpS.Left(p);
                        dPoint.y=atof(sd);
                        tmpS=tmpS.Mid(p+1);
                    }
                    else
                    {
                        dPoint.y=atof(tmpS);
                        tmpS.Empty();
                    }
                    m_Figure.Add(dPoint);
                    continue;
                }
                break;
            }
            break;
        }
    }
    return true;
}

bool GenericCapture::PrintProperties(FXString& txt)
{
    FXPropertyKit pk;
    CCtrlGadget::PrintProperties(txt);
    pk.WriteString("Message",m_Cmd);
    pk.WriteString("Attributes",m_Attributes);
    pk.WriteString("Label",m_Label);
    pk.WriteInt("NoSync",m_NoID);   
    pk.WriteInt( "SendOnStart" , m_bSendOnStart );
    pk.WriteInt( "SetEOS" , m_SetEOS );
    pk.WriteInt( "SetRegistered" , m_SetRegistredFlag );
    pk.WriteInt( "Manual" , m_Manual );
    pk.WriteInt("Replace",m_Replace);
    pk.WriteInt("FrameRate",m_FrameRate);
    pk.WriteInt("DataType",m_DataType);
    pk.WriteInt("IgnoreInConnect" , m_bIgnoreInputConnected ) ;
    txt+=pk;
    return true;
}


void GenericCapture::UpdateBtnStatus()
{
    FXAutolock al(m_Lock);
    if (IsCtrlReady())
    {
        m_Button.EnableWindow(
            ((m_pStatus->GetStatus()==CExecutionStatus::RUN) ||
            (m_pStatus->GetStatus()==CExecutionStatus::PAUSE))
            ?TRUE:FALSE);
        m_Button.ShowWindow((m_Manual)?SW_SHOW:SW_HIDE);
    }
}
