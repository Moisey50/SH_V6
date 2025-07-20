// AviFile.cpp: implementation of the CAviFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AviFile.h"
#include <files\AviVideoStream.h>
#include <files\AviTextStream.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

class CActivateCom  
{
public:
	CActivateCom()
	{
#if (_WIN32_WINNT >= 0x0400) || defined(_WIN32_DCOM)
		CoInitializeEx(NULL, COINIT_MULTITHREADED);
#else
		CoInitialize(NULL);
#endif
	}
	virtual ~CActivateCom() { CoUninitialize(); }
};

#define THIS_MODULENAME "TVAvi.AviFile"

#pragma comment(lib, "vfw32.lib")

#define VERBOSE_CASE(err)		\
	case err:					\
		return FXString(#err);	\

__forceinline BOOL _ensure_overwrite(LPCTSTR fileName)
{
	CFileStatus fs;
	if (CFile::GetStatus(fileName, fs))
	{
		TRY
		{
			CFile::Remove(fileName);
		}
		CATCH(CFileException, e)
		{
			return FALSE;
		}
		END_CATCH;
	}
	return TRUE;
}

__forceinline UINT _filemode2streammode(UINT mode)
{
	switch (mode)
	{
	case CAviFile::modeRead:
		return OF_READ;
	case CAviFile::modeOverwrite:
		return OF_CREATE;
	case CAviFile::modeAppend:
		return OF_WRITE;
	default:
		ASSERT(FALSE);
		return 0;
	}
}

FXString VerboseAviError(HRESULT error)
{
	switch (error)
	{
		VERBOSE_CASE(AVIERR_UNSUPPORTED);
		VERBOSE_CASE(AVIERR_BADFORMAT);
		VERBOSE_CASE(AVIERR_MEMORY);
		VERBOSE_CASE(AVIERR_INTERNAL);
		VERBOSE_CASE(AVIERR_BADFLAGS);
		VERBOSE_CASE(AVIERR_BADPARAM);
		VERBOSE_CASE(AVIERR_BADSIZE);
		VERBOSE_CASE(AVIERR_BADHANDLE);
		VERBOSE_CASE(AVIERR_FILEREAD);
		VERBOSE_CASE(AVIERR_FILEWRITE);
        VERBOSE_CASE(AVIERR_FILEOPEN);
		VERBOSE_CASE(AVIERR_COMPRESSOR);
		VERBOSE_CASE(AVIERR_NOCOMPRESSOR);
		VERBOSE_CASE(AVIERR_READONLY);
		VERBOSE_CASE(AVIERR_NODATA);
		VERBOSE_CASE(AVIERR_BUFFERTOOSMALL);
		VERBOSE_CASE(AVIERR_CANTCOMPRESS);
		VERBOSE_CASE(AVIERR_USERABORT);
		VERBOSE_CASE(REGDB_E_CLASSNOTREG);
		VERBOSE_CASE(CO_E_NOTINITIALIZED);
	default:
		{
			FXString err;
			err.Format("Unknown error %d (0x%x)", (UINT)error, (UINT)error);
			return err;
		}
	}
}

BOOL TraceSuccess(HRESULT hResult)
{
	if (SUCCEEDED(hResult))
		return TRUE;
	SENDERR_1("AVI File error: %s", VerboseAviError(hResult));
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FXString GetAviFileName(LPCTSTR fmt) 
{
    FXString retV=fmt;
    CTime t = CTime::GetCurrentTime();
    FXString tmpS;
    FXSIZE pos;
    while((pos=retV.Find('%'))>=0)
    {
        tmpS=retV.Mid(pos,2);
        if (tmpS.GetLength()==2)
        {
            retV.Delete(pos,2);
            if (tmpS=="%s")
                continue;
            tmpS=t.Format(tmpS);
            // remove forbidden chars
            tmpS.Replace( ':', '-');
            tmpS.Replace( '/', '.');
            tmpS.Replace( '\\', ' ');
            tmpS.Replace( '*', '+');
            tmpS.Replace( '|', 'I');
            tmpS.Replace( '?', ' ');
            tmpS.Replace( '>', ' ');
            tmpS.Replace( '<', ' ');
            tmpS.Replace( '"', '\'');

            retV.Insert(pos,tmpS);
        }
    }
    return retV;
}


CAviFile::CAviFile():
     m_RealFileName("")
    ,m_FileNameFormat("")
    ,m_pFile(NULL)
    ,m_LastAVIError(ERROR_SUCCESS)
    ,m_uMode(-1)
    ,m_VIDC_FOURCC(0)
    ,m_OpenErrorShown(false)
    ,m_CalcFrameRate(FALSE)
    ,m_OverwriteFrameRate(FALSE)
    ,m_FrameRate(25.0)
{
	::AVIFileInit();
}

CAviFile::~CAviFile()
{
	Close();
	::AVIFileExit();
}

void CAviFile::SetFrameRate(BOOL CalcFrameRate, BOOL OverwriteFrameRate, double FrameRate)
{
    m_CalcFrameRate=CalcFrameRate;
    m_OverwriteFrameRate=OverwriteFrameRate;
    m_FrameRate=FrameRate;
}

BOOL CAviFile::Open(LPCTSTR fileName, UINT uMode)
{
    m_FileNameFormat=fileName;
    FXString fName;
	if (IsOpen()) Close();
	if ((uMode == modeOverwrite) && !_ensure_overwrite(fileName))
		return FALSE;
	CActivateCom synchronize;
    if (strchr(fileName,'%'))
        fName=GetAviFileName(fileName);
    else
        fName=fileName;
	m_LastAVIError = ::AVIFileOpen(&m_pFile, fName, uMode, NULL);
	if (!SUCCEEDED(m_LastAVIError))
	{
		if (!m_OpenErrorShown)
			TraceSuccess(m_LastAVIError);
		Close();
		ErrorShown();
		return FALSE;
	}
	m_RealFileName = fName;
	m_uMode = uMode;
	m_LastAVIError = ::AVIFileInfo(m_pFile, &m_FileInfo, sizeof(m_FileInfo));
	if (!TraceSuccess(m_LastAVIError))
	{
		Close();
		ErrorShown();
		return FALSE;
	}
	return TRUE;
}

class CFps: public CArray<double,double>
{
};

void CAviFile::Close()
{
    if (!IsOpen()) return;
	CActivateCom synchronize;
	m_Lock.Lock();
	POSITION pos = m_Streams.GetStartPosition();
    CFps fpss;
	while (pos)
	{
		CAviStream* Stream;
		void* type;
		m_Streams.GetNextAssoc(pos, (void*&)Stream, type);
        double t=Stream->GetTimeLength();
        double f=Stream->GetLength();
        double fps=Stream->GetLength()/Stream->GetStreamTime();
        fpss.Add(fps);
        if(Stream)
        {
		    m_Streams.RemoveKey(Stream);
            Stream->Destroy();
        }
		delete Stream;
	}
	ZeroMemory(&m_FileInfo, sizeof(m_FileInfo));
	//m_RealFileName.Empty();
	if (m_pFile)
		::AVIFileRelease(m_pFile);
	m_pFile = NULL;
	m_uMode = -1;
    ResetErrors();
    if (m_CalcFrameRate)
        VERIFY(OverwriteFPS(&fpss));
	m_Lock.Unlock();
}

BOOL CAviFile::IsOpen()
{
	return (m_pFile != NULL);
}

LPCTSTR CAviFile::GetRealFileName()
{
	return m_RealFileName;
}

LPCTSTR CAviFile::GetFileNameFormat()
{
	return m_FileNameFormat;
}


int CAviFile::GetStreamCount()
{
	if (!m_pFile)
		return 0;
	return (int)m_FileInfo.dwStreams;
}

void* CAviFile::GetStream(long id)
{
	if (!m_pFile)
		return NULL;
	PAVISTREAM pStream;
	m_LastAVIError = ::AVIFileGetStream(m_pFile, &pStream, 0, id);
	if (!TraceSuccess(m_LastAVIError))
		return NULL;
	AVISTREAMINFO sInfo;
	m_LastAVIError = ::AVIStreamInfo(pStream, &sInfo, (LONG)sizeof(sInfo));
	if (!TraceSuccess(m_LastAVIError))
	{
		::AVIStreamRelease(pStream);
		return NULL;
	}
	CAviStream* Stream = NULL;
	switch (sInfo.fccType)
	{
	case streamtypeVIDEO:
		Stream = new CAviVideoStream(m_pFile, pStream, &sInfo, _filemode2streammode(m_uMode));
		break;
	case streamtypeTEXT:
		Stream = new CAviTextStream(m_pFile, pStream, &sInfo, _filemode2streammode(m_uMode));
		break;
	default:
//		Stream = new CAviStream(m_pFile, _filemode2streammode(m_uMode), pStream, &sInfo);
		break;
	}
	m_Lock.Lock();
	m_Streams.SetAt(Stream, (void*)(size_t)sInfo.fccType);
	m_Lock.Unlock();
	return Stream;
}

void* CAviFile::GetStream(DWORD type, long id)
{
	if (!m_pFile)
		return NULL;
	if ((id < 0) && (m_uMode == OF_READ))
		return NULL;
	CAviStream* Stream = NULL;
	switch (type)
	{
	case streamtypeVIDEO:
		if (id < 0)
			Stream = new CAviVideoStream(m_pFile, OF_CREATE, -1, m_VIDC_FOURCC);
		else
			Stream = new CAviVideoStream(m_pFile, _filemode2streammode(m_uMode), id, m_VIDC_FOURCC);
		break;
	case streamtypeTEXT:
		if (id < 0)
			Stream = new CAviTextStream(m_pFile, OF_CREATE, -1);
		else
			Stream = new CAviTextStream(m_pFile, _filemode2streammode(m_uMode), id);
		break;
	}
	if (!Stream || !Stream->IsValid())
	{
		delete Stream;
		return NULL;
	}
    Stream->SetFrameRate(m_CalcFrameRate, m_OverwriteFrameRate,m_FrameRate);
	m_Lock.Lock();
	m_Streams.SetAt(Stream, (void*) (size_t) type);
	m_Lock.Unlock();
	return Stream;
}

DWORD CAviFile::GetStreamType(void* pStream)
{
	if (!m_pFile)
		return 0;
	DWORD result = 0;
	m_Lock.Lock();
	void* type;
	if (m_Streams.Lookup(pStream, type) && type)
		result = (DWORD)(size_t)type;
	m_Lock.Unlock();
	return result;
}

void CAviFile::SetVideoFourCC(DWORD fourCC)
{
    m_VIDC_FOURCC=fourCC;
}

DWORD CAviFile::GetVideoFourCC()
{
    return m_VIDC_FOURCC;
}

void CAviFile::ReleaseStream(void* pStream)
{
	m_Lock.Lock();
	void* dummy;
	if (m_Streams.Lookup(pStream, dummy))
	{
		m_Streams.RemoveKey(pStream);
		delete ((CAviStream*)pStream);
	}
	m_Lock.Unlock();
}

int CAviFile::GetPos(void* pStream)
{
	void* type;
	if (!m_Streams.Lookup(pStream, type))
		return -1;
	return ((CAviStream*)pStream)->GetCurFrameID();
}

BOOL CAviFile::SeekToFrame(void* pStream, int nFrame)
{
	void* type;
	if (!m_Streams.Lookup(pStream, type))
		return FALSE;
	return ((CAviStream*)pStream)->SeekTo(nFrame);
}

void CAviFile::SeekToBegin(void* pStream)
{
	if (pStream) SeekToFrame(pStream, -1);
}

void CAviFile::SeekToEnd(void* pStream)
{
    if (pStream) SeekToFrame(pStream, ((CAviStream*)pStream)->GetLength()-1);
}

BOOL CAviFile::IsEOF(void* pStream)
{
	void* type;
	if (!m_Streams.Lookup(pStream, type))
		return TRUE;
	return ((CAviStream*)pStream)->IsEOF();
}

int CAviFile::GetLength(void* pStream)
{
	void* type;
	if (!m_Streams.Lookup(pStream, type))
		return -1;
	return ((CAviStream*)pStream)->GetLength();
}

double CAviFile::GetTimeLength(void* pStream)
{
	void* type;
	if (!m_Streams.Lookup(pStream, type))
		return -1;
	return ((CAviStream*)pStream)->GetTimeLength();
}

void* CAviFile::Read(void* pStream, DWORD& ID)
{
	void* type;
	if (!m_Streams.Lookup(pStream, type))
		return NULL;
	if (!((CAviStream*)pStream)->SetNextFrame())
		return NULL;
    ID=((CAviStream*)pStream)->GetCurFrameID();
	return ((CAviStream*)pStream)->ReadFrame();
}

BOOL CAviFile::Write(void* pStream, void* pFrame, LPCTSTR Label)
{
	void* type;
	if (!m_Streams.Lookup(pStream, type))
		return NULL;
	return ((CAviStream*)pStream)->WriteFrame(pFrame, Label);
}

/// Kind of hucking
typedef struct tagRIFF
{
    FOURCC fourcc;
    unsigned  fileSize;
    FOURCC fileType;
}RIFF;

typedef struct tagLIST
{
    FOURCC LIST;
    unsigned listSize;
    FOURCC listType;
}LIST;

typedef struct _avimainheader {
    FOURCC fcc;
    DWORD  cb;
    DWORD  dwMicroSecPerFrame;
    DWORD  dwMaxBytesPerSec;
    DWORD  dwPaddingGranularity;
    DWORD  dwFlags;
    DWORD  dwTotalFrames;
    DWORD  dwInitialFrames;
    DWORD  dwStreams;
    DWORD  dwSuggestedBufferSize;
    DWORD  dwWidth;
    DWORD  dwHeight;
    DWORD  dwReserved[4];
} AVIMAINHEADER;

typedef struct _avistreamheader {
     FOURCC fcc;
     DWORD  cb;
     FOURCC fccType;
     FOURCC fccHandler;
     DWORD  dwFlags;
     WORD   wPriority;
     WORD   wLanguage;
     DWORD  dwInitialFrames;
     DWORD  dwScale;
     DWORD  dwRate;
     DWORD  dwStart;
     DWORD  dwLength;
     DWORD  dwSuggestedBufferSize;
     DWORD  dwQuality;
     DWORD  dwSampleSize;
     struct {
         short int left;
         short int top;
         short int right;
         short int bottom;
     }  rcFrame;
} AVISTREAMHEADER;

typedef struct _avistreamformat 
{
     FOURCC fcc;
     DWORD  cb;
     BITMAPINFOHEADER bmih;
}AVISTREAMFOMAT;

typedef struct _chunk
{
     FOURCC fcc;
     DWORD  cb;
}CHUNK;

__forceinline int readRIFF(CFile &fl, RIFF& riff)
{
    fl.Read(&riff,sizeof(RIFF));
    if (riff.fourcc!=mmioFOURCC('R','I','F','F')) return -1;
    if (riff.fileType!=mmioFOURCC('A','V','I',' ')) return -1;
    return (int)fl.GetPosition( );
}

__forceinline int readLIST(CFile &fl, LIST& list)
{
    fl.Read(&list,sizeof(LIST));
    if (list.LIST!=mmioFOURCC('L','I','S','T')) return -1;
    return (int)fl.GetPosition( );
}

__forceinline int readAVIMAINHEADER(CFile &fl, AVIMAINHEADER& aviMH)
{
    fl.Read(&aviMH,sizeof(AVIMAINHEADER));
    if (aviMH.fcc!=mmioFOURCC('a','v','i','h')) return -1;
    return (int)fl.GetPosition( );
}

__forceinline int readAVISTREAMHEADER(CFile &fl, AVISTREAMHEADER& aviSH)
{
    fl.Read(&aviSH,sizeof(AVISTREAMHEADER));
    if (aviSH.fcc!=mmioFOURCC('s','t','r','h')) return -1;
    return (int)fl.GetPosition( );
}

__forceinline int readBITMAPINFO(CFile &fl, AVISTREAMFOMAT& aviSF)
{
    fl.Read(&aviSF,sizeof(AVISTREAMFOMAT));
    if (aviSF.fcc!=mmioFOURCC('s','t','r','f')) return -1;
    if (aviSF.cb>aviSF.bmih.biSize) // palettized BMP, we don't need palette now
    { // just rewind
        fl.Seek(aviSF.cb-aviSF.bmih.biSize,CFile::current);
    }
    return (int)fl.GetPosition( );
}

__forceinline int skipSTRD(CFile &fl)
{
    int cp=(int)fl.GetPosition( );
    CHUNK chnk;
    fl.Read(&chnk,sizeof(chnk));
    if (chnk.fcc==mmioFOURCC('s','t','r','d'))
        fl.Seek((chnk.cb%2)?chnk.cb+1:chnk.cb,CFile::current); //seems offset must be even
    else
        fl.Seek(cp,CFile::begin);
    return (int)fl.GetPosition( );
}

__forceinline int readStreamName(CFile &fl, CString& name)
{
    int cp=(int)fl.GetPosition( );
    CHUNK chnk;
    fl.Read(&chnk,sizeof(chnk));
    if (chnk.fcc==mmioFOURCC('s','t','r','n'))
    {
        LPTSTR buf=name.GetBufferSetLength(chnk.cb);
        fl.Read(buf,chnk.cb);
        name.ReleaseBuffer();
    }
    else
        fl.Seek(cp,CFile::begin);
    return (int)fl.GetPosition( );
}

bool CAviFile::OverwriteFPS(CFps* fps)
{
    CFile fl;
    ///test reading
    if (!fl.Open(m_RealFileName,CFile::modeReadWrite|CFile::modeNoTruncate)) return false;
    fl.SeekToBegin();
    RIFF riff;
    LIST list;
    AVIMAINHEADER   aviMH;
    AVISTREAMHEADER aviSH;
    AVISTREAMFOMAT  aviSF;
    CString sname;

    int off=readRIFF(fl, riff); if (off<0) return false;
    off=readLIST(fl,list);      if (off<0) return false;

    off=readAVIMAINHEADER(fl,aviMH); if (off<0) return false;
    for (int i=0; i<fps->GetSize(); i++)
    {
        /// Start reading stream info
        int aviSHoff;
        aviSHoff=off=readLIST(fl,list);  if (off<0) return false;
        off=readAVISTREAMHEADER(fl,aviSH); if (off<0) return false;
        aviSH.dwScale=10;
        aviSH.dwRate=(int)(fps->GetAt(i)*aviSH.dwScale);
        fl.Seek(aviSHoff,CFile::begin);
        fl.Write(&aviSH,sizeof(AVISTREAMHEADER));
        if (aviSH.fccType==mmioFOURCC('v','i','d','s'))
        {
            off=readBITMAPINFO(fl,aviSF);
        }
        else if (aviSH.fccType==mmioFOURCC('t','x','t','s'))
        {
            CHUNK chnk;
            fl.Read(&chnk,sizeof(chnk));
            off=(int)fl.GetPosition( );
            fl.Seek((chnk.cb%2)?chnk.cb+1:chnk.cb,CFile::current);
            off=(int)fl.GetPosition( );
        }
        else
            return false;
        off=skipSTRD(fl);       if (off<0) return false;
        off=readStreamName(fl,sname); if (off<0) return false;
    }
    fl.Close();
    return true;
}