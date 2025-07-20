#ifndef _GADB400_BASE_INCLUDED
#define _GADB400_BASE_INCLUDED

#ifdef _DEBUG
#define GADBASE_DLL_NAME "gadbased.dll"
#define GADBASE_LIB_NAME "gadbased.lib"
#else
#define GADBASE_DLL_NAME "gadbase.dll"
#define GADBASE_LIB_NAME "gadbase.lib"
#endif

#ifndef GADBASE_DLL
#define FX_EXT_GADGET __declspec(dllimport)
#pragma comment(lib, GADBASE_LIB_NAME)
#else
#define FX_EXT_GADGET __declspec(dllexport)
#endif

#include <fxfc\fxfc.h>
#include <fxfc\FXLoggerToFile.h>
#include <afxtempl.h>
#include <helpers/LockingFIFO.h>

void FX_EXT_GADGET attachgadbaseDLL();

#define NOSYNC_FRAME        (0)     // if frameID==0 - it means, that no sync for frame is requied

#define END_OF_STREAM_MARK	(-1)	// Every capture gadget must send an additional dataframe (EOS)
// with this time-stamp on stop command. By default EOS data is not
// to be processed by other gadgets. EOS is used for synchronization,
// clean up and/or reset purposes.

#define DEFAULT_LABEL	NULL	// in this case type conversion/search of data packet with DEFAULT_LABEL succeeds, no matter how it is actually labelled
// define DEFAULT_LABEL			// in this case type conversion/search of data packet will succeed only if packet has empty label (which is given by default)

#define CONNECTORS_BACK_COMPATIBILITY

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data types formalism. Version 1.1
//
// The following is added to the version 0.1:
//
// 1. transparent - special data type. No connection restrictions during editing. Instead requires run-time
//     compatibility check
// 2. implementation
//	   transparent = 0
//     for input connectors transparent is being substituted by nulltype at construction time
//     for output connectors compatibility is checked of DataFrame being passed vs input connector type
//
//              - - -
//
// Data types formalism. Version 0.1
//
// 1. nulltype - no data but id ("time-stamp"). Compatible with any data type
// 2. elementary types (vframe etc). Compatible with same type and nulltype
// 3. composite types (e.g. vframes + clusters). Compatible with corresponding elementary types and nulltype
//
// Compatibility check algorithm
//
// 1. nulltype = 1
// 2. elementary type = primes (2, 3, 5, 7, 11 ...)
// 3. composite type = product of corresponding elenentary types (e.g. 2 * 3, 2 * 3 * 5, etc)
// 4. compatibility criterion: (output_type % input_type == 0)
// 
// List of first 1000 primes can be found at http://primes.utm.edu/lists/small/1000.txt 
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum
{
  transparent = 0,	// unspecified format, needs run-time compatibility checking
  nulltype = 1,    // bare time-stamp (synchronization data)
  vframe = 2,	// video frame
  text = 3,	// text
  wave = 5,	// sound
  //     = 7,	// deleted control (notifications exchange, setup etc)
  quantity = 11,	// quantity (integer, double etc)
  logical = 13,	// logical value (boolean true/false)
  rectangle = 17,	// rectangle
  figure = 19,   // figure - set of points connected with segments
  metafile = 23,   // windows metafile drawing
  userdata = 29,	// specific user-defined data
  arraytype = 31,	// array
} basicdatatype;

typedef unsigned datatype;

__forceinline FXString Tvdb400_TypeToStr(datatype type)
{
  FXString retV;
  if (type == transparent)
    retV = "transparent";
  else if (type == nulltype)
    retV = "nulltype";
  else
  {
    if (type%vframe == 0)
      retV += "vframe";
    if (type%text == 0)
    {
      retV += (retV.GetLength()) ? ":" : ""; retV += "text";
    }
    if (type%wave == 0)
    {
      retV += (retV.GetLength()) ? ":" : ""; retV += "wave";
    }
    if (type%quantity == 0)
    {
      retV += (retV.GetLength()) ? ":" : ""; retV += "quantity";
    }
    if (type%logical == 0)
    {
      retV += (retV.GetLength()) ? ":" : ""; retV += "logical";
    }
    if (type % rectangle == 0)
    {
      retV += (retV.GetLength()) ? ":" : ""; retV += "rectangle";
    }
    if (type%figure == 0)
    {
      retV += (retV.GetLength()) ? ":" : ""; retV += "figure";
    }
    if (type%metafile == 0)
    {
      retV += (retV.GetLength()) ? ":" : ""; retV += "metafile";
    }
    if (type%userdata == 0)
    {
      retV += (retV.GetLength()) ? ":" : ""; retV += "userdata";
    }
    if (type%arraytype == 0)
    {
      retV += (retV.GetLength()) ? ":" : ""; retV += "arraytype";
    }
  }
  return retV;
}

bool FX_EXT_GADGET Tvdb400_TypesCompatible(datatype output_type, datatype input_type);

bool FX_EXT_GADGET Tvdb400_CheckSetLicense( LPCTSTR pAppName , LPCTSTR pDirName );
bool FX_EXT_GADGET Tvdb400_AddRemoveLicense( LPCTSTR pName , int i );
int  FX_EXT_GADGET Tvdb400_CheckLicense( LPCTSTR pName ); 

#define DEFAULT_GADGET_WIDTH		50
#define DEFAULT_GADGET_HEIGHT		80
#define GADGET_VIDEO_WIDTH	320
#define GADGET_VIDEO_HEIGHT	240


// Predefined lineages 
#define LINEAGE_DEBUG       _T("Debug")
#define LINEAGE_COMPLEX     _T("Complex")
#define LINEAGE_GENERIC     _T("Generic")
#define LINEAGE_CTRL        _T("Controls")
#define LINEAGE_DIAGNOSTIC  _T("Diagnostic")
#define LINEAGE_TEXT        _T("Text")
#define LINEAGE_VIDEO       _T("Video")
#define LINEAGE_TESTER      _T("TestGadgets")
#define LINEAGE_WMF         _T("Metafiles")
#define LINEAGE_RECTANGLES  _T("Data.Rectangles")
#define LINEAGE_LOGIC       _T("Boolean")
#define LINEAGE_COMM        _T("Communication")
#define LINEAGE_FILEX       _T("FileX_Specific")
#define LINEAGE_DEMO        _T("DemoGadgets")
#define LINEAGE_VIRTUAL		  _T("Virtual")

#define FLW_ADDITION        (28)

// Default type of renderer monitor
#define SET_INPLACERENDERERMONITOR _T("_inplace")

//
// CDataFrame
//
class CDataFrame;
class CVideoFrame;
class CTextFrame;
class CWaveFrame;
class CQuantityFrame;
class CBooleanFrame;
class CRectFrame;
class CFigureFrame;
class CMetafileFrame;
class CUserDataFrame;
class CFLWArchive;
class CArrayFrame;

class CFramesIterator
{
protected:
  CFramesIterator()
  {}
public:
  virtual ~CFramesIterator()
  {}
  virtual CDataFrame* Next(LPCTSTR label = DEFAULT_LABEL)
  {
    return NULL;
  }
  virtual CDataFrame* NextChild(LPCTSTR label = DEFAULT_LABEL)
  {
    return NULL;
  }
};

#ifdef _DEBUG
//#define _TRACE_DATAFRAMERELEASE
#endif

#ifdef _TRACE_DATAFRAMERELEASE
#define RELEASE(Frame)	Release(Frame, m_Name)
#else
#define RELEASE(Frame)	Release(Frame)
#endif

class FX_EXT_GADGET CDataFrame
{
  friend class CDataChain ;
protected:
  datatype        m_DataType;   // data type
private:
  long            m_cUsers;	// access counter
  FXLockObject    m_Lock; // synchronization object
  FXPropertyKit   m_Attributes;
protected:  
    // !!!Don't change order of next 4 variables (important for serialization)
  bool            m_Registered; // delivery with guarantee
  DWORD           m_ID;		  // id for synchronization (usualy frame # from start)
  double          m_Time;   // Graph Time (i.e. from graph start and pause accounting) 
  double          m_AbsTime;// Abs time from computer reset in ms
  FXString        m_Label;	  // any additional information.
  //  potential use - subclassifying frames of same data type
protected:
  CDataFrame(datatype dt = nulltype);
  virtual ~CDataFrame();
public:
  datatype GetDataType() const;
  int     AddRef(int nRefs = 1);
#ifdef _TRACE_DATAFRAMERELEASE
  virtual bool Release(CDataFrame* Frame, LPCTSTR Name = "");
#else
  virtual bool Release(CDataFrame* Frame = NULL);
#endif
  // #ifdef _DEBUG
  long GetUserCnt() const
  {
    return m_cUsers;
  };
  // #endif
  DWORD   GetId() const
  {
    return m_ID;
  }
  void    ChangeId(DWORD id)
  {
    m_ID = id;
  }
  double  GetTime() const
  {
    return m_Time;
  }
  void    SetTime(double id)
  {
    m_Time = id;
  }
  double  GetAbsTime() const
  {
    return m_AbsTime;
  }
  void    SetAbsTime(double dT)
  {
    m_AbsTime = dT;
  }
  LPCTSTR GetLabel() const
  {
    return m_Label;
  }
  void    SetLabel(LPCTSTR label)
  {
    m_Label = label;
  }
  const FXPropertyKit* Attributes() const
  {
    return &m_Attributes;
  }
  FXPropertyKit* Attributes()
  {
    return &m_Attributes;
  }
  bool    IsRegistered() const
  {
    return m_Registered;
  }
  void    SetRegistered()
  {
    m_Registered = true;
  }
  // Data access
  //  Subclass must overwrite corresponding function to return valid pointer to dataframe (itself)
  //  NULL is returned if datatype or label doesn't match. Default empty label query should
  //  match empty label of requested data frame. To get frame with "any" label, pass NULL as a parameter
  virtual const CDataFrame*     GetDataFrame(LPCTSTR label = DEFAULT_LABEL) const;
  virtual CDataFrame*     GetDataFrame(LPCTSTR label = DEFAULT_LABEL);
  virtual const CVideoFrame*    GetVideoFrame(LPCTSTR label = DEFAULT_LABEL) const
  {
    return NULL;
  };
  virtual CVideoFrame*    GetVideoFrame(LPCTSTR label = DEFAULT_LABEL)
  {
    return NULL;
  };
  virtual const CTextFrame*     GetTextFrame(LPCTSTR label = DEFAULT_LABEL) const
  {
    return NULL;
  };
  virtual CTextFrame*     GetTextFrame(LPCTSTR label = DEFAULT_LABEL)
  {
    return NULL;
  };
  virtual const CWaveFrame*     GetWaveFrame(LPCTSTR label = DEFAULT_LABEL) const
  {
    return NULL;
  };
  virtual CWaveFrame*     GetWaveFrame(LPCTSTR label = DEFAULT_LABEL)
  {
    return NULL;
  };
  virtual const CQuantityFrame* GetQuantityFrame(LPCTSTR label = DEFAULT_LABEL) const
  {
    return NULL;
  };
  virtual CQuantityFrame* GetQuantityFrame(LPCTSTR label = DEFAULT_LABEL)
  {
    return NULL;
  };
  virtual const CBooleanFrame*  GetBooleanFrame(LPCTSTR label = DEFAULT_LABEL) const
  {
    return NULL;
  };
  virtual CBooleanFrame*  GetBooleanFrame(LPCTSTR label = DEFAULT_LABEL)
  {
    return NULL;
  };
  virtual const CRectFrame*     GetRectFrame(LPCTSTR label = DEFAULT_LABEL) const
  {
    return NULL;
  };
  virtual CRectFrame*     GetRectFrame(LPCTSTR label = DEFAULT_LABEL)
  {
    return NULL;
  };
  virtual const CFigureFrame*   GetFigureFrame(LPCTSTR label = DEFAULT_LABEL) const
  {
    return NULL;
  };
  virtual CFigureFrame*   GetFigureFrame(LPCTSTR label = DEFAULT_LABEL)
  {
    return NULL;
  };
  virtual const CMetafileFrame* GetMetafileFrame(LPCTSTR label = DEFAULT_LABEL) const
  {
    return NULL;
  };
  virtual CMetafileFrame* GetMetafileFrame(LPCTSTR label = DEFAULT_LABEL)
  {
    return NULL;
  };
  virtual const CArrayFrame*	  GetArrayFrame(LPCTSTR label = DEFAULT_LABEL) const
  {
    return NULL;
  };
  virtual CArrayFrame*	  GetArrayFrame(LPCTSTR label = DEFAULT_LABEL)
  {
    return NULL;
  };
  virtual const CUserDataFrame* GetUserDataFrame(LPCTSTR uType, LPCTSTR label = DEFAULT_LABEL) const
  {
    return NULL;
  };
  virtual CUserDataFrame* GetUserDataFrame(LPCTSTR uType, LPCTSTR label = DEFAULT_LABEL)
  {
    return NULL;
  };
  // Specific data access for container
  virtual CFramesIterator* CreateFramesIterator(datatype type) const
  {
    return NULL;
  };
  // Container copying
  virtual CDataFrame* Copy() const; // Make copy of whole data
  virtual void        CopyAttributes(const CDataFrame* src, bool bCopyAbsTime = true); // Make copy of attributes from src frame
  virtual CDataFrame* CopyContainer()
  {
    return NULL;
  } // Allocate new container and copy pointers 
  virtual bool        IsContainer() const
  {
    return false;
  }
  // Virtual data access and drawing functions 
  virtual bool        Draw(CDC * dc, RECT rc)
  {
    return false;
  }
  static  CDataFrame* Create(datatype dt =/*transparent*/nulltype);
  static	CDataFrame* CreateFrom(void* pData, UINT cData);


  // Serialization
#pragma pack(push,1)
  typedef struct _tagDestSerDataFrame
  {
    bool        m_bRegistered;
    DWORD       m_ID;
    double      m_dTime;
    double      m_dAbsTime;
    datatype    m_DataType;
    char        m_LabelAndAttrib[1] ;
  } DestSerDataFrame;
#pragma pack(pop)

  virtual BOOL Serialize( LPBYTE* ppData , FXSIZE* cbData ) const;
  virtual BOOL SerializeCommon( LPBYTE pBufferOrigin , FXSIZE& CurrentWriteIndex , FXSIZE BufLen ) const;

  virtual BOOL Serialize( LPBYTE pBufferOrigin , FXSIZE& CurrentWriteIndex , FXSIZE BufLen ) const
  {
    return SerializeCommon( pBufferOrigin , CurrentWriteIndex , BufLen ) ;
  }

  virtual FXSIZE GetSerializeLength( FXSIZE& uiLabelLen ,
    FXSIZE * pAttribLen = NULL ) const ;
  virtual BOOL Restore( LPBYTE lpData , FXSIZE cbData );
  virtual void ToLogString(FXString& Output);
};

class FX_EXT_GADGET CUserDataFrame : public CDataFrame
{
  typedef CUserDataFrame * (FAR __stdcall *UserFrameCreateFunc)();
  class CUserDataType
  {
  public:

    CUserDataType(LPCTSTR Name = NULL, UserFrameCreateFunc CreateFunc = NULL)
    {
      m_DataName = Name;
      m_llId = 0;
      m_CreateFunc = CreateFunc;
    }

    long long    m_llId;
    FXString     m_DataName;
    UserFrameCreateFunc m_CreateFunc;
    CUserDataType& operator =  (const CUserDataType& Orig)
    {
      m_llId = Orig.m_llId;
      m_DataName = Orig.m_DataName;
      m_CreateFunc = Orig.m_CreateFunc;
      return *this;
    }
  };
protected:
  FXString m_UserDataName;
protected:
  CUserDataFrame(LPCTSTR name) : CDataFrame(userdata)
  {
    m_UserDataName = name;
  };
  virtual ~CUserDataFrame()
  {};
public:
  static   FXArray<CUserDataFrame::CUserDataType> m_UserTypes;
  static   FXLockObject           m_Lock;
  const FXString& GetUserType() const
  {
    return m_UserDataName;
  };
  CUserDataFrame* GetUserDataFrame(LPCTSTR uType, LPCTSTR label = DEFAULT_LABEL)
  {
    if ((!label || m_Label == label) && (!uType || m_UserDataName == uType))
      return this;
    return NULL;
  }
  const CUserDataFrame* GetUserDataFrame(LPCTSTR uType, LPCTSTR label = DEFAULT_LABEL) const
  {
    if ((!label || m_Label == label) && (!uType || m_UserDataName == uType))
      return this;
    return NULL;
  }
  virtual CDataFrame* Copy() const
  {
    LPBYTE lpData;
    FXSIZE cbData;
    if (Serialize(&lpData, &cbData))
    {
      CUserDataFrame* copy = CreateEmpty();
      BOOL result = copy->Restore(lpData, cbData);
      free(lpData);
      if (result)
        return copy;
      copy->Release(copy);
    }
    return NULL;
  }
  static bool RegisterCreateFunc(LPCTSTR DataName, UserFrameCreateFunc pFunc)
  {
    FXAutolock al(m_Lock);
    for (int i = 0; i < (int)m_UserTypes.GetCount(); i++)
    {
      if (m_UserTypes[i].m_DataName == DataName)
        return true;
    }

    CUserDataType NewType(DataName, pFunc);
    size_t iLen = strlen(DataName);
    if (iLen > sizeof(NewType.m_llId))
      iLen = sizeof(NewType.m_llId);
    memcpy(&NewType.m_llId, DataName, iLen);
    m_UserTypes.Add(NewType);
    return true;
  }
  static CUserDataFrame* CreateEmpty(long long Id = 0)
  {
    if (Id != 0)
    {
      FXAutolock al(m_Lock);
      for (int i = 0; i < (int)m_UserTypes.GetCount(); i++)
      {
        if (Id == m_UserTypes[i].m_llId)
          return m_UserTypes[i].m_CreateFunc();
      }
    }
    return NULL;
  }
  virtual BOOL Serialize(LPBYTE* ppData, FXSIZE* cbData) const
  {
    return FALSE;
  };
  virtual BOOL SerializeUserData(LPBYTE* ppData, FXSIZE* cbData) const
  {
    return FALSE;
  };
  virtual BOOL Restore(LPBYTE lpData, FXSIZE cbData)
  {
    return FALSE;
  };
  virtual BOOL RestoreUserData(LPBYTE lpData, FXSIZE cbData)
  {
    return FALSE;
  };
  virtual DWORD GetThisFrameDataLen() const
  {
    return 0;
  } // len of data
  virtual DWORD GetThisFrameDataLenLen() const
  {
    return 0;
  } //len of data len
};

bool FX_EXT_GADGET Tvdb400_FrameLabelMatch(const CDataFrame* pDataFrame, LPCTSTR label);
bool FX_EXT_GADGET Tvdb400_IsEOS(const CDataFrame* pDataFrame);
void FX_EXT_GADGET Tvdb400_SetEOS(CDataFrame* pDataFrame);
bool FX_EXT_GADGET Tvdb400_Serialize(const CDataFrame* pDataFrame, LPBYTE* ppData, FXSIZE* cbData);
FX_EXT_GADGET CDataFrame* Tvdb400_Restore(LPBYTE lpData, FXSIZE cbData);

//
// CWire
// CConnector
//  CInputConnector
//  COutPin
//   CDuplexConnector
//   COutputConnector



class FX_EXT_GADGET CConnector
{
protected:
  FXString m_Name;
  datatype m_DataType;		// type of data frames, which this connector accept
  FXLockPtrArray m_Wires;		// links to complementary connectors
  FXString m_Label;			// label for all data frames passing through the pin
  DWORD    m_FramesPassed;
  DWORD    m_FramesSkipped;
  int      m_iLogging;
  BOOL     m_bIsVisible ; // for hidden connectors on filter gadgets
                          // which should have input and output connectors
  static   FXLoggerToFile m_PinLogger;
public:
  CConnector(datatype dtype);
  virtual ~CConnector();
  virtual bool		Connect(CConnector* pConnector);
  virtual bool		Disconnect(CConnector* pConnector = NULL); // NULL = disconnect all
  virtual bool		Send(CDataFrame* pDataFrame)
  {
    return false;
  };
  static void ProcessPinLogMessage(void * pMsg, FXString& outp);
#ifdef CONNECTORS_BACK_COMPATIBILITY
  bool		IsConnected()
  {
    return (m_Wires.GetSizeLocked() > 0);
  };
#endif
  void		SetName(LPCTSTR str)
  {
    m_Name = str;
  }
  LPCTSTR		GetName()
  {
    return m_Name;
  };
  void		SetLabel(LPCTSTR label)
  {
    m_Label = label;
  }
  LPCTSTR		GetLabel()
  {
    return m_Label;
  }
  virtual int			GetComplementary(FXPtrArray& Connectors);
  virtual CConnector*	GetInputPin(datatype* dtype = NULL)
  {
    return NULL;
  };
  virtual CConnector*	GetOutputPin(datatype* dtype = NULL)
  {
    return NULL;
  };
  virtual bool		GetPinInfo(FXString& result);
  virtual int LogIfNecessary(const CDataFrame * pDataFrame);
  datatype			GetDataType()
  {
    return m_DataType;
  };
  DWORD       GetFramesPassed()
  {
    DWORD retV = m_FramesPassed; 
    m_FramesPassed = 0; 
    return retV;
  }
  DWORD       GetFramesSkipped()
  {
    DWORD retV = m_FramesSkipped;
    m_FramesSkipped = 0;
    return retV;
  }
  void SetLogMode(int iLogMode)
  {
    m_iLogging = iLogMode;
  }
  int GetLogMode() { return m_iLogging;  }
  void SetVisible( bool bSet ) { m_bIsVisible = bSet ; }
  bool IsVisible() { return m_bIsVisible ;  }
};

class CWire
{
private:
  CConnector* m_pConnector1;
  CConnector* m_pConnector2;
  FXLockObject m_Lock;
public:
  CWire(CConnector* pConnector1, CConnector* pConnector2);
  ~CWire();
  bool Push(CDataFrame* pDataFrame, CConnector* pSender, BOOL bTypeCheck);
  friend class CConnector;
};

typedef void (CALLBACK *FN_SENDINPUTDATA)(CDataFrame* lpData, void* lpParam, CConnector* lpInput);

class CInputConnector : public CConnector
{
protected:
  FXStaticQueue<CDataFrame*> m_FramesQueue;
  FXLockObject m_PinLock;
protected:
  // Direct data transfer
  FN_SENDINPUTDATA m_pFnSendInputData;
  void* m_pHostGadget;
  // Indirect data buffering
  HANDLE m_evClose;
  HANDLE m_evHasData;
public:
  FX_EXT_GADGET CInputConnector(datatype dtype = nulltype, FN_SENDINPUTDATA fnSendInputData = NULL, void* lpHostGadget = NULL);
  FX_EXT_GADGET virtual ~CInputConnector();
  bool FX_EXT_GADGET IsDirectConnector();
  bool FX_EXT_GADGET Send(CDataFrame* pDataFrame); // direct input interface: pin -> gadget
  bool FX_EXT_GADGET Get(CDataFrame*& pDataFrame); // indirect input interface: gadget <- pin
  void FX_EXT_GADGET Close();
  virtual CConnector* GetInputPin(datatype* dtype = NULL)
  {
    if (dtype) *dtype = m_DataType; return this;
  };
  int GetNFramesInQueue()
  {
    return m_FramesQueue.ItemsInQueue();
  }
  int GetQueueSize()
  {
    return m_FramesQueue.GetQueueSize();
  }

  DWORD SetQueueSize(DWORD dwNewLength)
  {
    if (!m_pFnSendInputData
      && dwNewLength >= 1 && dwNewLength <= 1000)
    {
      if (m_FramesQueue.ResizeQueue(dwNewLength))
        return dwNewLength;
    }
    return m_FramesQueue.GetQueueSize();
  }
private:
  void SetDataAvailable(bool bSet);
};

class CBufferedInputConnector : public CInputConnector
{
  friend DWORD WINAPI ConnectorWatchFunc(void* pThis);
  HANDLE m_WrkThread;
protected:
  FXStaticQueue<CDataFrame*> m_InputQueue;
private:
  DWORD   Watch();
  void    ClearQueue();
public:
  FX_EXT_GADGET      CBufferedInputConnector(datatype dtype = nulltype, FN_SENDINPUTDATA fnSendInputData = NULL, void* lpHostGadget = NULL);
  FX_EXT_GADGET virtual ~CBufferedInputConnector();
  int FX_EXT_GADGET  ItemsInQueue()
  {
    return m_InputQueue.ItemsInQueue();
  }
  bool FX_EXT_GADGET Peep(int offset, CDataFrame*& Object)
  {
    return m_InputQueue.Peep(offset, Object);
  }
  bool FX_EXT_GADGET GetQueueObject(CDataFrame*& Object)
  {
    return m_InputQueue.GetQueueObject(Object);
  }
};

typedef BOOL(FAR __stdcall *OutputCallback)(CDataFrame*& pDataFrame, FXString& idPin, void* pClient);
typedef void (FAR __stdcall *RenderCallBack)(const CDataFrame* pDataFrame, void* wParam);

class COutputConnector : public CConnector
{
  FXLockObject    m_Lock;
  OutputCallback  m_fnOCB;
  void*           m_pExtClient;
  FXString        m_ExtID;
  bool            m_EOSSent;
public:
  FX_EXT_GADGET COutputConnector(datatype dtype = nulltype);
  FX_EXT_GADGET virtual ~COutputConnector();
  virtual CConnector* GetOutputPin(datatype* dtype = NULL)
  {
    if (dtype) *dtype = m_DataType; return this;
  };
  void FX_EXT_GADGET SetCallback(LPCTSTR id, OutputCallback fn, void* pClient);
  virtual bool FX_EXT_GADGET Put(CDataFrame* pDataFrame);
  FXLockObject& GetLocker() { return m_Lock ; }
};

class CGadget;
class CDuplexConnector : public COutputConnector
{
  CGadget*			m_pHostGadget;		// hosting gadget
  datatype			m_InputType;		// data type for input pin
public:
  FX_EXT_GADGET CDuplexConnector(CGadget* Host, datatype outtype = transparent, datatype intype = transparent);
  FX_EXT_GADGET virtual ~CDuplexConnector();
  bool FX_EXT_GADGET Send(CDataFrame* pDataFrame);
  virtual CConnector* GetInputPin(datatype* dtype = NULL)
  {
    if (dtype) *dtype = m_InputType; return this;
  };
};

typedef struct tagEXECUTIONSTATUS
{
  volatile LONG    m_cRefs;
  HANDLE m_evStart, m_evPause, m_evStepFwd, m_evStop;
}EXECUTIONSTATUS, *LPEXECUTIONSTATUS;

class FX_EXT_GADGET CExecutionStatus
{
protected:
  FXString    m_Name;
  LPEXECUTIONSTATUS m_pStatus;
public:
  enum
  {
    STOP, PAUSE, RUN, EXIT
  };
protected:
  LONG    AddRef();
  CExecutionStatus(CExecutionStatus* Status = NULL, LPCTSTR Name = NULL);
  ~CExecutionStatus();
public:
  static CExecutionStatus*   Create(CExecutionStatus* Status = NULL, LPCTSTR Name = NULL);
  LONG                Release();

  UINT GetStatus();
  BOOL IsForwardTriggerOn();
  HANDLE GetStartHandle()
  {
    if (m_pStatus) return m_pStatus->m_evStart; return NULL;
  }
  HANDLE GetStopHandle()
  {
    if (m_pStatus) return m_pStatus->m_evStop; return NULL;
  }
  HANDLE GetPauseHandle()
  {
    if (m_pStatus) return m_pStatus->m_evPause; return NULL;
  }
  HANDLE GetStpFwdHandle()
  {
    if (m_pStatus) return m_pStatus->m_evStepFwd; return NULL;
  }
  void Pause();
  void Start();
  void StepFwd();
  void Stop();
protected:
  void Copy(LPEXECUTIONSTATUS Status);
};

//
// CRuntimeGadget
// FXWorker
//  CGadget
//   CCaptureGadget
//   CFilterGadget
//   CCtrlGadget
//      CRenderGadget
//   CCompositeGadget

class CGadget;

struct CRuntimeGadget
{
  LPCTSTR m_lpszClassName;
  LPCTSTR m_lpszLineage;
  LPCTSTR m_lpszPlugin;
  CGadget* (__stdcall *m_pfnCreateGadget)();
  CRuntimeGadget* m_pBaseGadget;
  FX_EXT_GADGET BOOL IsDerivedFrom(const CRuntimeGadget* pBaseClass) const;
};

typedef double(__stdcall *TVDB400_FNGETGRAPHTIME)();
typedef BOOL(CALLBACK *TVDB400_FNGETGADGETNAME)(void*, void*, FXString&);

enum ReadOnlyForStdSetupState
{
  RO_NoGrayed = 0 ,
  RO_Normal ,     // show grayed
  RO_Disabled ,   // Don't show grayed, could be changed
  RO_DoNotShow    // Items are not shown
};
class CSetupObject
{
public:
  CSetupObject()
  {
  };
  virtual     ~CSetupObject()
  {};
  virtual bool Show(CPoint point, LPCTSTR uid)
  {
    return false;
  }
  virtual void Delete()
  {
    delete this;
  }
  virtual      BOOL IsOn()
  {
    return FALSE;
  }
  virtual FXPropertyKit& GetSavedProperties()
  {
    static FXPropertyKit Dummy;
    return Dummy;
  }
  virtual void SetSavedProperties(FXString& Properties)
  {};
  virtual void Update() {} ;
};

#define STATUS_MODIFIED _T("modified")
#define STATUS_REDRAW   _T("redraw")

class CGadget : public FXWorker
{
#ifdef _DEBUG
  int m_ShtDnCls;
#endif
  TVDB400_FNGETGRAPHTIME	m_pFnGetGraphTime;
  TVDB400_FNGETGADGETNAME	m_pFnGetGadgetName;
  void* m_pHost;
protected:
  FXLockObject    m_PinsLock;
  CSetupObject *	m_SetupObject;
  FXPropertyKit   m_Status;
  BOOL            m_bModified;
  BOOL            m_IsAboutToShutDown;
  int             m_Mode; // { reject, transmit, process }
  bool            m_Invalid;
  CExecutionStatus* m_pStatus;
  LockingFIFO<FXString> * m_pModifiedUIDs ;
protected:
  FX_EXT_GADGET CGadget();
  FX_EXT_GADGET BOOL GetGadgetName(FXString& Name)
  {
    return (m_pFnGetGadgetName && m_pHost) ? m_pFnGetGadgetName(this, m_pHost, Name) : FALSE;
  }
public:
  enum
  {
    mode_reject, mode_transmit, mode_process
  };
  virtual FX_EXT_GADGET void ShutDown();
  virtual FX_EXT_GADGET BOOL IsAboutToShutDown();
  virtual int GetInputsCount() = 0;
  virtual int GetOutputsCount() = 0;
  virtual FX_EXT_GADGET  int GetDuplexCount();
  virtual CInputConnector*    GetInputConnector(int n) = 0;
  virtual COutputConnector*   GetOutputConnector(int n) = 0;
  virtual FX_EXT_GADGET CDuplexConnector* GetDuplexConnector(int n);
  virtual FX_EXT_GADGET void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pDataFrame);
  FX_EXT_GADGET BOOL IsKindOf(CRuntimeGadget* RuntimeGadget);
  virtual FX_EXT_GADGET FXPropertyKit& Status()
  {
    return m_Status;
  };
  virtual FX_EXT_GADGET BOOL IsSetupOn();
  virtual FX_EXT_GADGET bool PrintProperties(FXString& text);
  virtual FX_EXT_GADGET bool PrintProperties(FXString& text, LPCTSTR pPropName);
  virtual FX_EXT_GADGET bool ScanProperties(LPCTSTR text, bool& Invalidate);
  virtual FX_EXT_GADGET bool ScanSettings(FXString& text)
  {
    return false;
  }
  virtual FX_EXT_GADGET LPCTSTR GetAdditionalInfo()
  {
    return NULL ;
  }
  virtual FX_EXT_GADGET bool IsInvalid()
  {
    return m_Invalid;
  }
  virtual CRuntimeGadget* GetRuntimeGadget();

  virtual FX_EXT_GADGET bool IsComplex()
  {
    return false;
  }

  void SetMode(int mode)
  {
    m_Mode = mode;
  };
  void GetMode(int& mode)
  {
    mode = m_Mode;
  };

  ///*virtual */void InitExecutionStatus(CExecutionStatus* Status)  /*{}*/;

  virtual void /*CGadget::*/InitExecutionStatus( CExecutionStatus* Status )
  {
    bool Suspended = ( ::WaitForSingleObject( m_evResume , 0 ) != WAIT_OBJECT_0 );
    if ( !Suspended )
      Suspend();
    if ( m_pStatus != NULL )
      ::SetEvent( m_pStatus->GetStartHandle() );
    if ( m_pStatus != Status )
    {
      if ( m_pStatus )
        m_pStatus->Release();
      m_pStatus = CExecutionStatus::Create( Status );
    }
    Resume();
  }

  void SetGraphTimer(TVDB400_FNGETGRAPHTIME pFnGetGraphTime)
  {
    m_pFnGetGraphTime = pFnGetGraphTime;
  };
  void SetNamingFunction(TVDB400_FNGETGADGETNAME pFnGetGadgetName, void* host)
  {
    m_pFnGetGadgetName = pFnGetGadgetName; m_pHost = host;
  };
  void SetSetupObject(CSetupObject* pSetupObject)
  {
    if (m_SetupObject)
      m_SetupObject->Delete();
    m_SetupObject = pSetupObject;
  };
  CSetupObject* GetSetupObject()
  {
    return m_SetupObject;
  }
  FX_EXT_GADGET double GetGraphTime() // Time is micro seconds from OS start
  {
    return (m_pFnGetGraphTime) ? m_pFnGetGraphTime() : -1;
  };
  void SetModifiedUIDsPtr( LockingFIFO<FXString> * pPtr )
  {
    m_pModifiedUIDs = pPtr ;
  }

  virtual DWORD GetBorderColor() { return 1 ; }
  virtual DWORD GetBodyColor() { return 1 ; }
  virtual BOOL CanBeGrouped() { return FALSE; }
  virtual void SetGroupSelected( BOOL bSelected ) {}
  virtual BOOL GetGroupSelected() { return FALSE ; }

  static CRuntimeGadget FX_EXT_GADGET classCGadget;
};

#define RUNTIME_GADGET(gadget_class) ((CRuntimeGadget*)(&gadget_class::class##gadget_class))

#define DECLARE_RUNTIME_GADGET(gadget_class) \
public: \
  static CGadget* __stdcall CreateGadget(); \
  virtual CRuntimeGadget* GetRuntimeGadget(); \
  static CRuntimeGadget class##gadget_class; \

#define IMPLEMENT_RUNTIME_GADGET_EX(gadget_class, base_gadget_class, lineage, plugin) \
  CGadget* __stdcall gadget_class::CreateGadget() { return new gadget_class; } \
  CRuntimeGadget* gadget_class::GetRuntimeGadget() { return ((CRuntimeGadget*)(&gadget_class::class##gadget_class)); } \
  CRuntimeGadget gadget_class::class##gadget_class = \
{ #gadget_class , lineage, plugin, gadget_class::CreateGadget, /*NULL, NULL,*/ RUNTIME_GADGET(base_gadget_class) }; \


#define IMPLEMENT_RUNTIME_GADGET(gadget_class, base_gadget_class, lineage)	IMPLEMENT_RUNTIME_GADGET_EX(gadget_class, base_gadget_class, lineage, _T(""))

#define REGISTER_RUNTIME_GADGET(gadget_class, lpGraphBuilder) \
  (((IGraphbuilder*)lpGraphBuilder)->RegisterGadgetClass(RUNTIME_GADGET(gadget_class)));

#define UNREGISTER_RUNTIME_GADGET(gadget_class, lpGraphBuilder) \
  (((IGraphbuilder*)lpGraphBuilder)->UnregisterGadgetClass(RUNTIME_GADGET(gadget_class)));

class FX_EXT_GADGET CCaptureGadget : public CGadget
{
protected:
  DWORD m_FrameCounter;
  COutputConnector* m_pOutput;
  CCaptureGadget();
  volatile BOOL m_bRun;
public:
  virtual void ShutDown();
  virtual void OnStart();
  virtual void OnStop();
  virtual int GetInputsCount();
  virtual int GetOutputsCount();
  CInputConnector* GetInputConnector(int n);
  COutputConnector* GetOutputConnector(int n);
  BOOL    IsRun()
  {
    return m_bRun;
  }
  bool    IsValid()
  {
    return m_pStatus != NULL;
  }
private:
  virtual int DoJob();
  virtual CDataFrame* GetNextFrame(double* StartTime);
protected:
  DECLARE_RUNTIME_GADGET(CCaptureGadget);
};

class FX_EXT_GADGET CFilterGadget : public CGadget
{
public:
  enum OutputMode
  {
    modeAppend = 0x0, // output frame added to input container, if any, 
    // default mode
    modeReplace = 0x1  // output frame will be sent, input frame will be released
  };
protected:
  OutputMode        m_OutputMode;
  CInputConnector*  m_pInput;
  COutputConnector* m_pOutput;
  CFilterGadget();
public:
  virtual void    ShutDown();
  virtual int     GetInputsCount();
  virtual int     GetOutputsCount();
  CInputConnector* GetInputConnector(int n);
  COutputConnector* GetOutputConnector(int n);
  virtual void    SetOutputMode(OutputMode om);
  virtual OutputMode GetOutputMode();
  virtual bool PrintProperties(FXString& text);
  virtual bool ScanProperties(LPCTSTR text, bool& Invalidate);
  virtual void OnEOS();
private:
  virtual int DoJob();
  virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
  DECLARE_RUNTIME_GADGET(CFilterGadget);
};

class FX_EXT_GADGET CPortGadget : public CFilterGadget
{
public:
protected:
  CPortGadget();
public:
  virtual LPCTSTR GetInfoForView() { return NULL ; } ;
private:
  DECLARE_RUNTIME_GADGET( CPortGadget );
};

class FX_EXT_GADGET CGathererFilter :
  public CGadget
{
protected:
  FXArray<CInputConnector*, CInputConnector*> m_Inputs;
  FXLockObject      m_InputsLock;
  COutputConnector* m_pOutput;
  CGathererFilter(void);
public:
  virtual void    ShutDown();
  virtual int     GetInputsCount();
  virtual int     GetOutputsCount();
  CInputConnector* GetInputConnector(int n);
  COutputConnector* GetOutputConnector(int n);
private:
  virtual int DoJob();
  virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame, int pin);
  DECLARE_RUNTIME_GADGET(CGathererFilter);
};


class FX_EXT_GADGET CCtrlGadget : public CCaptureGadget
{
protected:
  HWND    m_hParentWnd;
  LPTSTR  m_Monitor; // output id; NULL = not defined
  CCtrlGadget();
public:
  virtual void ShutDown();
  virtual int GetInputsCount()
  {
    return 0;
  }
  virtual CInputConnector* GetInputConnector(int n)
  {
    return NULL;
  }
  virtual bool PrintProperties(FXString& text);
  virtual bool ScanProperties(LPCTSTR text, bool& Invalidate);
  virtual void Create();
  virtual void Attach(CWnd* View)
  {
    m_hParentWnd = View->m_hWnd;
  }
  virtual void Detach()
  {};
  virtual LPCTSTR GetMonitor();
  virtual void SetMonitor(LPCTSTR monitor);
  virtual CWnd*GetRenderWnd()
  {
    return NULL;
  }
  virtual bool SetCallBack(RenderCallBack rcb, void* cbData)
  {
    return false;
  }
  virtual void GetDefaultWndSize(RECT& rc)
  {
    rc.left = rc.top = 0; rc.right = DEFAULT_GADGET_WIDTH; rc.bottom = DEFAULT_GADGET_HEIGHT;
  }
  DECLARE_RUNTIME_GADGET(CCtrlGadget);
};

class FX_EXT_GADGET CRenderGadget : public CCtrlGadget
{
protected:
  CInputConnector* m_pInput;
  CRenderGadget();
public:
  virtual void ShutDown();
  virtual int GetInputsCount();
  virtual int GetOutputsCount();
  CInputConnector* GetInputConnector(int n);
  COutputConnector* GetOutputConnector(int n);
private:
  virtual int DoJob();
  virtual void Render(const CDataFrame* pDataFrame);
  virtual bool ReceiveEOS(const CDataFrame* pDataFrame);
  DECLARE_RUNTIME_GADGET(CRenderGadget);
};

class FX_EXT_GADGET CCollectorGadget : public CGadget
{
protected:
  HANDLE            m_evDataReady;
  COutputConnector* m_pOutput;
  FXPtrArray		  m_Inputs;
  int			      m_nInputs;
  FXLockObject	  m_Lock;
  CDataFrame**	  m_FramesBuffer;
  long              m_nInBuffer;
  long              m_nOffset;
public:
  CCollectorGadget(void);
  virtual void        ShutDown();
  virtual int			GetInputsCount();
  virtual CInputConnector*    GetInputConnector(int n);
  virtual int			GetOutputsCount();
  virtual COutputConnector*   GetOutputConnector(int n);
protected:
  int	 DoJob();
  void CreateInputs(int n, basicdatatype type = nulltype, bool bLock = true);
  void RemoveInputs();
  void Input(CDataFrame* pDataFrame, CConnector* lpInput);
  void PrepareToSend(int pos);
  virtual CDataFrame* DoProcessing(CDataFrame const*const* frames, int nmb);
  void ClearBuffers(void);
  bool FindInBuffers(DWORD ID, int& pos);
  int  GetInputID(CConnector* lpInput);
  bool IsComplete(int pos);
  bool FindComplete(int& pos);
  bool Add(CDataFrame* pDataFrame, CConnector* lpInput);
  CDataFrame** GetFrame(int offset, int pin)
  {
    return (m_FramesBuffer + offset * m_nInputs + pin);
  }
  static void __stdcall CCollectorGadget_SendData(
    CDataFrame* pDataFrame, void* lParam, CConnector* lpInput)
  {
    ((CCollectorGadget*)lParam)->Input(pDataFrame, lpInput);
  };
#ifdef _DEBUG
  void TraceBuffers();
#else
#define TraceBuffers()
#endif
  DECLARE_RUNTIME_GADGET(CCollectorGadget);
};

class FX_EXT_GADGET CTwoPinCollector : public CCollectorGadget
{
public:
  CTwoPinCollector()
  {
    m_pOutput = new COutputConnector(vframe);
    CreateInputs(2, vframe);
    Resume();
  }
  virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame1, const CDataFrame* pDataFrame2)
  {
    return NULL;
  }
  CDataFrame* DoProcessing(CDataFrame const*const* frames, int nmb)
  {
    ASSERT(nmb == 2);
    return DoProcessing(frames[0], frames[1]);
  }
  DECLARE_RUNTIME_GADGET(CTwoPinCollector);
};

class FX_EXT_GADGET CCollectorRenderer : public CRenderGadget
{
protected:
  FXPtrArray		  m_Inputs;
  int			      m_nInputs;
  FXLockObject	  m_Lock;
  CDataFrame**	  m_FramesBuffer;
  long              m_nInBuffer;
  long              m_nOffset;
  CCollectorRenderer();
public:
  virtual void ShutDown();
  int		GetInputsCount();
  CInputConnector* GetInputConnector(int n);
protected:
  void    CreateInputs(int n, basicdatatype type = nulltype);
  void    RemoveInputs();
  void    Input(CDataFrame* pDataFrame, CConnector* lpInput);
  void    PrepareToSend(int pos);
  virtual void Render(CDataFrame const*const* frames, int nmb);
  void    ClearBuffers(void);
  bool    FindInBuffers(DWORD ID, int& pos);
  int     GetInputID(CConnector* lpInput);
  bool    IsComplete(int pos);
  bool    Add(CDataFrame* pDataFrame, CConnector* lpInput);
  CDataFrame** GetFrame(int offset, int pin)
  {
    return (m_FramesBuffer + offset * m_nInputs + pin);
  }
  static void __stdcall CCollectorRenderer_SendData(CDataFrame* pDataFrame, void* lParam, CConnector* lpInput)
{
  ((CCollectorRenderer*) lParam)->Input( pDataFrame , lpInput );
};
  DECLARE_RUNTIME_GADGET(CCollectorRenderer);
};

class FX_EXT_GADGET CVirtualGadget : public CGadget
{
  FXPtrArray	m_Inputs;
  FXPtrArray	m_Outputs;
  FXPtrArray	m_Duplex;
  FXString    m_Params;
  FXString	m_ClassName;
protected:
  CVirtualGadget();
public:
  virtual ~CVirtualGadget();
  virtual void ShutDown();
  virtual int GetInputsCount();
  virtual int GetOutputsCount();
  virtual int GetDuplexCount();
  CInputConnector* GetInputConnector(int n);
  COutputConnector* GetOutputConnector(int n);
  CDuplexConnector* GetDuplexConnector(int n);
  virtual bool PrintProperties(FXString& text);
  virtual bool ScanProperties(LPCTSTR text, bool& Invalidate);
  virtual bool ScanSettings(FXString& text);
  void SetClassName(LPCTSTR clName)
  {
    m_ClassName = clName;
  };
  LPCTSTR GetClassName()
  {
    return LPCTSTR(m_ClassName);
  };
private:
  virtual int DoJob();
  static void CALLBACK VirtualGadgetInputFn(CDataFrame* Data, void* Host, CConnector* lpInput)
{
    if (Data) Data->RELEASE(Data);
}
  DECLARE_RUNTIME_GADGET(CVirtualGadget);
};

class IPluginLoader
{
public:
  enum
  {
    PL_SUCCESS,
    PL_PLUGINFOLDERNOTFOUND,
    PL_PLUGINNOTFOUND,
    PL_NOPLUGINENTRY,
  };
  virtual ~IPluginLoader()
  {}
  virtual void _stdcall RegisterPlugins(LPVOID pGraphBuilder, LPCTSTR PluginFolder = _T("Plugins")) = 0;
  virtual int _stdcall CheckPluginExist(FXString& Plugin, LPCTSTR PluginFolder = _T("Plugins")) = 0;
  virtual int _stdcall IncludePlugin(LPVOID pGraphBuilder, FXString& Plugin, LPCTSTR PluginFolder = _T("Plugins")) = 0;
};

#endif