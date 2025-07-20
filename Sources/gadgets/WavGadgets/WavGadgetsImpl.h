#ifndef WAV_GADGETS_IMPL
#define WAV_GADGETS_IMPL

#include <gadgets\gadbase.h>
#include <gadgets\WaveFrame.h>
#include "WaveFileCaptureSetupDlg.h"
#include "WaveSpeakerSetupDlg.h"


#define MAXNUMOFBUFFER 5
//#define BUFFERTIME     500  //Time of buffer in milliseconds
#define BUFFERTIME     50  //Time of buffer in milliseconds

#define SOUNDERROR_DONTOPEN (WAVERR_LASTERROR+0)
#define SOUNDERROR_NOFILE   (WAVERR_LASTERROR+1)

//#define SPEAKER_OUT  0
//#define SPEAKER_NONE 1
//#define SPEAKER_FILE 2

LPCTSTR GetMMErrorMessage(MMRESULT LastErrNo);

typedef struct _tagSBSetupData
{
    int           iMaxBufNum;
    DWORD         dwBufferSize;
    HWAVEIN       hWaveIn;
    WAVEFORMATEX  WaveFormat;
    GLOBALHANDLE* hInBuffer;
    GLOBALHANDLE* hWaveInHdr;
    LPWAVEHDR*    lpWaveInHdr;
    LPBYTE*       lpInBuffer;
    MMRESULT      LastErrNo;
    _tagSBSetupData():
    hInBuffer(NULL),
        hWaveInHdr(NULL),
        lpWaveInHdr(NULL),
        lpInBuffer(NULL),
        hWaveIn(NULL),
        LastErrNo(MMSYSERR_NOERROR)
    {};
}SBSetupData,*pSBSetupData;

class SBSetupHelper: public SBSetupData  
{
protected:
    ///
    void _releasebuffers()
    {
        if (hInBuffer)   delete [] hInBuffer;   hInBuffer = NULL;
        if (hWaveInHdr)  delete [] hWaveInHdr;  hWaveInHdr= NULL;
        if (lpWaveInHdr) delete [] lpWaveInHdr; lpWaveInHdr= NULL;
        if (lpInBuffer)  delete [] lpInBuffer;  lpInBuffer= NULL;
    }
    void _init(int MaxBufNum, int BufferTime)
    {
        _releasebuffers();
        iMaxBufNum=MaxBufNum;

        hInBuffer  = new GLOBALHANDLE[MaxBufNum];
        hWaveInHdr = new GLOBALHANDLE[MaxBufNum];
        lpWaveInHdr= new LPWAVEHDR[MaxBufNum];
        lpInBuffer = new LPBYTE[MaxBufNum];
        for(int i=0; i<iMaxBufNum; i++)
        {
            hWaveInHdr[i]  = NULL;
            hInBuffer[i]   = NULL;
            lpWaveInHdr[i] = NULL;
            lpInBuffer[i]  = NULL;
        }
        LastErrNo=MMSYSERR_NOERROR;
    }
public:
    SBSetupHelper(int MaxBufNum, int BufferLength)
    {
        _init(MaxBufNum,BufferLength);
    }
    ~SBSetupHelper()
    {
        _releasebuffers();    
    }
    LPCTSTR GetlastErrMes() { return GetMMErrorMessage(LastErrNo); };
};

class WaveCapture : public CCaptureGadget,
    public SBSetupHelper
{
    friend void CALLBACK SoundInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD dwParam2);
    friend class WaveCaptureGadgetSetupDlg;
protected:
    int         m_CurrentBufferNum;
    LPWAVEHDR   m_lpWaveHdr; // Filled up buffer pointer
    bool        m_IsOpen,m_IsStarted;
    MCIDEVICEID m_wMCIDeviceID;
    int         m_iNChannels ;
    HANDLE      m_hEvent;    // Signal that buffer ready
    FXLockObject m_Lock;
protected:
            WaveCapture();
    virtual CDataFrame* GetNextFrame(double* StartTime);
public:
    bool            Init();
    void            Close();
    void            Start();
    void            Stop();
    virtual void    ShutDown();
    virtual void    OnStart();
    virtual void    OnStop();
	virtual bool    PrintProperties(FXString& text);
	virtual bool    ScanProperties(LPCTSTR text, bool& Invalidate);
    virtual bool    ScanSettings(FXString& text);
private:
    void SendEof();
    DECLARE_RUNTIME_GADGET(WaveCapture);
};


class WavFileCapture : public CCaptureGadget,
    public SBSetupHelper
{
    friend class CWaveFileCaptureSetupDlg;
protected:
    bool        m_IsOpen,m_IsStarted,m_IsEof;
    BOOL		m_bLoop;
    CString     m_fName;
    HMMIO       m_hFile;
    int         m_CrntBuffer;
    LONG        m_BuffersRead;
    LONG        m_bytesRead;
    FXLockObject m_Lock;
protected:
    WavFileCapture();
    virtual CDataFrame* GetNextFrame(double* StartTime);
public:
    virtual void OnStart();
    virtual void OnStop();
    virtual void ShutDown();
    virtual bool PrintProperties(FXString& text);
    virtual bool ScanProperties(LPCTSTR text, bool& Invalidate);
    virtual bool    ScanSettings(FXString& text);
    void OpenWavFile();
private:
    void SendEof();
    DECLARE_RUNTIME_GADGET(WavFileCapture);
};


typedef struct _tagSBSpeakerSetup
{
    HWAVEOUT      hWaveOut;
    LPWAVEHDR    *lpWaveHdr;
    GLOBALHANDLE *hWaveInHdr;
    GLOBALHANDLE *hOutBuffer;
    _tagSBSpeakerSetup():
    hWaveOut(NULL),
        lpWaveHdr(NULL),
        hWaveInHdr(NULL),
        hOutBuffer(NULL)
    {
    }
}SBSpeakerSetup;

class WaveSpeaker : public CRenderGadget,
    public SBSpeakerSetup
{
    friend class WaveSpeakerSetupDlg;
protected:
    MCIDEVICEID m_wMCIDeviceID;
    int m_BufferPntr;
    int m_BuffersNum;
    int m_Mode;
    WaveSpeakerSetupDlg* m_pSetupDlg;
    //    FXLockObject m_Lock;
public:
    virtual void ShutDown();
protected:
    void copyWAVEHDR(LPWAVEHDR pwh);
    void CloseDevice();
    bool InitDevice(WAVEFORMATEX*  WaveFormat);
private:
    WaveSpeaker();
    virtual void Render(const CDataFrame* pDataFrame);
    //void ShowSetupDialog(CPoint& point);
    DECLARE_RUNTIME_GADGET(WaveSpeaker);
};

#endif //WAV_GADGETS_IMPL

