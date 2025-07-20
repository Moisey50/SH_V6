#ifndef SHWRAPPER_INC
#define SHWRAPPER_INC
#pragma once

#ifndef SHWRAPPER_COMPILE

#define AFX_EXT_DLL __declspec(dllimport)
#ifndef _DEBUG
#pragma comment(lib,"shwrapper.lib")
#else
#pragma comment(lib,"shwrapperd.lib")
#endif
#else
#define AFX_EXT_DLL __declspec(dllexport)
#endif

#include <fxfc\fxfc.h>
// #include <files\futils.h>
#include <classes\DPoint.h>
#include <fxfc\LockObject.h>

typedef bool (FAR __stdcall *TextCallbackW)( const wchar_t * result, LPVOID lParam);
typedef bool (FAR __stdcall *TextCallbackA)(const char * result, LPVOID lParam);

enum CallBackDataType 
{
  DATA_NULL ,
  DATA_TEXT , 
  DATA_BOOL , 
  DATA_QUANTITY , 
  DATA_FIGURE , 
  DATA_IMAGE ,
  DATA_RECT ,

  DATA_UNKNOWN
};

enum GadgetWorkingMode
{
  GWM_REJECT = 0 ,
  GWM_PASS_THROW = 1 ,
  GWM_PROCESS = 2
};

union FrameData
{
  LPCSTR    pString ;
  bool      BoolVal ;
  double    Quantity ;
  RECT      Rectangle ;
  CDPointArray* pFigure ;
  BITMAPINFOHEADER* BmpInfo ;
};

class CallBackDataA 
{
public:
	const char * m_Label ;
	CallBackDataType m_ResultType ;
	double   m_dTime_ms ;
  DWORD    m_dwFrameId ;
	FrameData m_Par1 ;
	LPVOID   m_Par2 ;

	CallBackDataA( CallBackDataType Type = DATA_UNKNOWN , double dTime_ms = 0. , DWORD dwId = 0 ) 
  {
    memset( this , 0 , sizeof( *this ) ) ; 
    m_ResultType = Type ;
    m_dTime_ms = dTime_ms ;
    m_dwFrameId = dwId ;
  };
	CallBackDataA(const CallBackDataA& other); //eliminates multi-referencing to same memory of the class members;
	~CallBackDataA() { CheckAndDeleteAllocated() ; } ;
	CallBackDataA& operator=(const CallBackDataA& other); //eliminates multi-referencing to same memory of the class members;
  inline void CheckAndDeleteAllocated()
  {
    if ( m_Label ) 
      delete m_Label ;
    //     if ( m_ResultType == DATA_TEXT )
    //     {
    //       if ( m_Par1.pString )
    //         delete[] m_Par1.pString ;
    //     }
    // now figure data is in using directly in received figure frame, no allocation
    //     else if ( m_ResultType == DATA_FIGURE )
    //     {
    //       if ( m_Par1.pFigure )
    //         delete m_Par1.pFigure ;
    //     }
    // now vframe data is in using directly in received vframe, no allocation
    //     else if ( m_ResultType == DATA_IMAGE )
    //     {
    //       if ( m_Par1.BmpInfo )
    //         delete m_Par1.BmpInfo ;
    //     }
  }
  inline void Zero() 
  { 
    CheckAndDeleteAllocated() ;
    memset( this , 0 , sizeof( *this ) ) ; 
  }
	void Reset( CallBackDataType Type = DATA_UNKNOWN ,
		double dTime = 0. , DWORD dwId = 0 )
	{
    Zero() ;
		m_ResultType = Type; 
		m_dTime_ms = dTime ;
    m_dwFrameId = dwId ;
	} ;
  inline void SetId( DWORD dwId ) { m_dwFrameId = dwId ; }
  inline int GetId() { return m_dwFrameId ; }
  inline void SetImageData( LPBYTE pImageData )
  {
    if ( m_ResultType != DATA_IMAGE )
      return ;
    m_Par2 = pImageData ;
  }
  inline LPBYTE GetImageData()
  {
    if ( m_ResultType != DATA_IMAGE )
      return NULL ;
    return (LPBYTE)m_Par2 ;
  }
};

class CallBackDataW 
{
public:
  const wchar_t * m_Label ;
  CallBackDataType m_ResultType ;
  double   m_dTime_ms ;
  DWORD    m_dwFrameId ;
  FrameData m_Par1 ;
  LPVOID  m_Par2 ;

  CallBackDataW( CallBackDataType Type = DATA_UNKNOWN , double dTime_ms = 0. , DWORD dwId = 0 ) 
  { 
    memset( this , 0 , sizeof( *this ) ) ; 
    m_ResultType = Type ;
    m_dTime_ms = dTime_ms ;
    m_dwFrameId = dwId ;
  };
  CallBackDataW(const CallBackDataW& other); //eliminates multi-referencing to same memory of the class members;
  ~CallBackDataW() { CheckAndDeleteAllocated() ; } ;
  CallBackDataW& operator=(const CallBackDataW& other); //eliminates multi-referencing to same memory of the class members;
  inline void CheckAndDeleteAllocated()
  {
    if ( m_Label ) 
      delete m_Label ;
    //     if ( m_ResultType == DATA_TEXT )
    //     {
    //       if ( m_Par1.pString )
    //         delete[] m_Par1.pString ;
    //     }
    // now figure data is in using directly in received figure frame, no allocation
    //     else if ( m_ResultType == DATA_FIGURE )
    //     {
    //       if ( m_Par1.pFigure )
    //         delete m_Par1.pFigure ;
    //     }
    // now vframe data is in using directly in received vframe, no allocation
    //     else if ( m_ResultType == DATA_IMAGE )
    //     {
    //       if ( m_Par1.BmpInfo )
    //         delete m_Par1.BmpInfo ;
    //     }
  }
  inline void Zero() 
  { 
    CheckAndDeleteAllocated() ;
    memset( this , 0 , sizeof( this) ) ; 
  }
  void Reset( CallBackDataType Type = DATA_UNKNOWN ,
    double dTime = 0. , DWORD dwId = 0 )
  {
    Zero() ;
    m_ResultType = Type; 
    m_dTime_ms = dTime ;
    m_dwFrameId = dwId ;
  } ;
  inline void SetId( DWORD dwId ) { m_dwFrameId = dwId ; }
  inline int GetId() { return m_dwFrameId ; }
  inline void SetImageData( LPBYTE pImageData )
  {
    if ( m_ResultType != DATA_IMAGE )
      return ;
    m_Par2 = pImageData ;
  }
  inline LPBYTE GetImageData()
  {
    if ( m_ResultType != DATA_IMAGE )
      return NULL ;
    return (LPBYTE)m_Par2 ;
  }
} ;

typedef bool (FAR __stdcall *DataCallbackA)( CallBackDataA& Data , LPVOID lParam ) ;
typedef bool (FAR __stdcall *DataCallbackW)( CallBackDataW& Data , LPVOID lParam ) ;

typedef void (FAR __stdcall  *PrintLogMsgFuncA)(
  int msgLevel, const char * src, int msgId, const char * msgText);
typedef void (FAR __stdcall  *PrintLogMsgFuncW)(
  int msgLevel, const wchar_t * src, int msgId, const wchar_t * msgText);




#include <string>
#include <set>
#include <map>
#include <list>
#include <vector>

using namespace std;

class CGadgetInfo
{
	size_t m_type;
	string m_className;
	string m_name;	

public:
	CGadgetInfo(const string& name="", const string& clssName="", size_t type = 0):m_name(name), m_className(clssName), m_type(type) {}

	size_t getType()const{return m_type;}
	string getClassName()const{return m_className;}
	string getName()const{return m_name;}

	bool operator<(const CGadgetInfo& gdgt)const
	{
		return m_name.compare(gdgt.m_name)<0;
	}
};

class CGadgetsDB
{
	set<CGadgetInfo> m_gdgts;
	map<size_t, list<const CGadgetInfo*> > m_gdgtsByType;
	map<string, list<const CGadgetInfo*> > m_gdgtsByClss;

	void _addToGadgetsByType( const CGadgetInfo& gdgt )
	{
		m_gdgtsByType[gdgt.getType()].push_back(&gdgt);
	}

	void _removeFromGadgetsByType( const CGadgetInfo& gdgt )
	{
		m_gdgtsByType[gdgt.getType()].remove(&gdgt);
	}

	void _addToGadgetsByClss( const CGadgetInfo& gdgt )
	{
		m_gdgtsByClss[gdgt.getClassName()].push_back(&gdgt);
	}

	void _removeFromGadgetsByClss( const CGadgetInfo& gdgt )
	{
		m_gdgtsByClss[gdgt.getClassName()].remove(&gdgt);
	}

	vector<CGadgetInfo> _listPtrs2VectVals(const list<const CGadgetInfo*>& gdgtsPtrs)const;
public:
	bool addGadget(const CGadgetInfo& gdgt);

	bool removeGadget(const string& name);

	void clean()
	{
		m_gdgtsByType.clear();
		m_gdgtsByClss.clear();
		m_gdgts.clear();
	}

	vector<CGadgetInfo> getGadgetsByType(size_t type)const
	{
		map<size_t, list<const CGadgetInfo*> >::const_iterator cit = m_gdgtsByType.find(type); 
		// we want to eliminate adding a new type if the required 'type' does not exist in the map
		// then using of the operator[] will imposable because the function is 'const';

		return _listPtrs2VectVals(cit->second);
	}

	vector<CGadgetInfo> getGadgetsByClassName(const string& clssName)const
	{
		map<string, list<const CGadgetInfo*> >::const_iterator cit = m_gdgtsByClss.find(clssName); 
		// we want to eliminate adding a new type if the required 'type' does not exist in the map
		// then using of the operator[] will imposable because the function is 'const';

		return _listPtrs2VectVals(cit->second);
	}

	map<size_t, list<const CGadgetInfo*> > getGadgetsByTypes()const
	{
		return m_gdgtsByType;
	}

	map<string, list<const CGadgetInfo*> > getGadgetsByClassNames()const
	{
		return m_gdgtsByClss;
	}
};


class CGabgetsDBFactory
{
	set<string> m_gadgetsNames;

public:
	CGabgetsDBFactory(const CStringArray& gadgets)
	{
		for(int i = 0; i < gadgets.GetSize(); i++)
			m_gadgetsNames.insert(string((LPCTSTR)(gadgets[i])));
	}

	CGadgetsDB& fillDB( CGadgetsDB& db, void *pBuilder );
};


// Main interface class for communication between SH and user application

class AFX_EXT_DLL ConnectGraph
{
  void *  m_pBuilder ;
  void *  m_pPluginLoader ;
  void *  m_CB ;
  void *  m_DataCB ;
  void *  m_pCorrespondence ;
  FXLockObject m_Lock ;
  PrintLogMsgFuncW m_pLogger ;
  bool    m_bRunBeforeInspect ;
  bool    m_bExternalConnect ;
  FXString m_GraphPath ;

  CGadgetsDB m_gdgtsInfoDb;

  void OperateGraph(bool toStart = false);

  void _setRenderRect( const RECT& newRC, LPCTSTR renderName, void *pBuilder);

  ConnectGraph(const ConnectGraph&); //eliminates multi-referencing to same memory of the class members;
  ConnectGraph& operator=(const ConnectGraph&); //eliminates multi-referencing to same memory of the class members;

public:
  ConnectGraph( const wchar_t * tvgName ) ;     // wide char version of constructor
  ConnectGraph( const char * tvgName = NULL ) ;
  ~ConnectGraph() ;
  bool Init(const wchar_t * tvgName , PrintLogMsgFuncW pLogger = NULL, bool toStart = true ); // Program or reprogram class with new graph
  bool Init(const char * tvgName , PrintLogMsgFuncA pLogger = NULL, bool toStart = true );
  bool Init( void * pBuilder , void * pLoader ) ;           // for old connection style support (if graph is loaded directly)
  bool SaveGraph( LPCTSTR pPath = NULL ) ; // if parameter is zero, loaded graph will be saved

  void StartGraph() {OperateGraph(true);}  // Execution control
  void StopGraph() {OperateGraph(false);}
  void PauseGraph();
  void StepFrwrdGraph();

  void a2wMessage( int msgLevel, const char * src, int msgId, const char * msgText ) ; // converter from char to wide char
  void Disconnect( int iTimeout = 500 ); // disconnect from graph
  void RemoveCallbacks() ; // call backs shutdown
  bool IsInitialized() { return m_pBuilder != NULL ; } ;
  void RunSetupDialog( const char * pGadgetName = NULL , CPoint * pPoint = NULL ) ; // setup dialog viewing
  void RunSetupDialog( const wchar_t * pGadgetName/* = NULL */, CPoint * pPoint = NULL ) ;
  void ViewGraph( CWnd * pHost ) ; // View graph over application. In current version saving is not realized
  void ViewSetup( CWnd * PHost ) ; // View all setups for all gadgets
  inline double GetTime() { return GetHRTickCount(); } ; // returns time in milliseconds from OS start; usual, resolution is 4.33 MHz, may be vary

  template<class T> void SetRenderRect( const RECT& newRC, T renderName ) // Not recommended for using: view output assignment 
  {
  	LPCTSTR name;
  	if(sizeof(T) == 2)
  	{
  		int len = wcslen( (const wchar_t*)renderName );
  		char * pResult = new char[len+1] ;
  		size_t ConvLen ;
  		if ( wcstombs_s( &ConvLen , pResult , len+1 , (const wchar_t*)renderName , len ) )
  			name = NULL ;
  		name = pResult ;
  	}
  	else
  		name = renderName;
  
  	_setRenderRect(newRC, name, m_pBuilder);
  
  	if(sizeof(T) == 2)
  		delete[] name;
  }

  bool ConnectRendererToWindow( // Window Assignment for renderer gadget (wide char version)
    CWnd * pWnd , const wchar_t * WindowCaption , 
    const wchar_t * RendererName  ) ;
  bool ConnectRendererToWindow( // Window Assignment for renderer gadget 
    CWnd * pWnd ,const char *  WindowCaption ,
    const char * RendererName  ) ;
  void ReconnectRenderersToWindows() ; // This function should be called after grpah view closing

  void * GetCB() { return m_CB ; } ;
  void * GetDataCB() { return m_DataCB ; } ;
  PrintLogMsgFuncW GetLoggerFunc() { return m_pLogger ;}
  void LockCBs() ; // callbacks data protection
  void UnlockCBs() ;  // and unprotection
  
  bool SetTextCallBack(       // Set text callback for pin. Only text frames will produce callback
    const wchar_t * PinName ,
    TextCallbackW oc,         // callback data (look TextCallbackW)
    LPVOID lParam ,           // usually pointer to host class
    bool bMulti = false );    // if bMulti if false, first TextFrame content will be delivered
                              // if bMulti is true, all text frames contents will be delivered with
                              // callback for every textframe (i.e. if container n text frames is arrived, 
                              // n callbacks will be issued
 
  bool SetTextCallBack(      // Set text callback for pin. Only text frames will produce callback
    const char * PinName , 
    TextCallbackA oc,        // callback data (look TextCallback)
    LPVOID lParam ,          // usually pointer to host class
    bool bMulti = false );   // if bMulti if false, first TextFrame content will be delivered
                             // if bMulti is true, all text frames contents will be delivered with
                             // callback for every textframe (i.e. if container n text frames is arrived,
                             // n callbacks will be issued

  bool SetDataCallBackW(      // The same with previous function, but for any data types. Not all data types currently processed.
    const  wchar_t * PinName , 
    DataCallbackW oc,        // look DataCallbackW definition 
    LPVOID lParam , 
    bool bMulti = false );

  bool SetDataCallBack(      // The same with previous function, 8 bits char version
    const char * PinName , 
    DataCallbackA oc,        // look DataCallback definition
    LPVOID lParam , 
    bool bMulti = false );

  bool SetDataCallBackEx(      // Newer version of set callback. As result callbacks providing time and frame id info.
    const  wchar_t * PinName , 
    DataCallbackW oc, 
    LPVOID lParam , 
    bool bMulti = false );

  bool SetDataCallBackEx(
    const char * PinName , 
    DataCallbackA oc, 
    LPVOID lParam , 
    bool bMulti = false );


  bool SendFrame( const char * PinName , // Send text frame to pin
    CDataFrame * Data ,
    const char * pLabel = NULL ) ;
  bool SendText( const char * PinName , // Send text frame to pin
    const char * Data ,
    const char * pLabel = NULL ) ;
  bool SendText( const wchar_t * PinName ,
    const wchar_t * Data ,
    const wchar_t * pLabel = NULL ) ;
  int SendText( const char * PinNames[] , // Send text frame to pins, 
    const char * Data ,                   // last pointer should be NULL
    const char * pLabel = NULL ) ;
//   bool SendText( const wchar_t * PinNames[] ,// Send text frame to pins, 
//     const wchar_t * Data ,                  // last pointer should be NULL
//     const wchar_t * pLabel = NULL ) ;
  bool SendBoolean( const char * PinName , // Send boolean frame to pin
    bool bValue , 
    const char * pLabel = NULL) ;
  bool SendBoolean( const wchar_t * PinName , 
    bool bValue , 
    const wchar_t * pLabel = NULL ) ;
  bool SendQuantity( const char * PinName , // Send integer as quantity frame to pin
    int iValue ,
    const char * pLabel = NULL ) ;
  bool SendQuantity( const wchar_t * PinName ,
    int iValue ,
    const wchar_t * pLabel = NULL ) ;
  bool SendQuantity( const char * PinName , // Send double as quantity frame to pin
    double dValue ,
    const char * pLabel = NULL ) ;
  bool SendQuantity( const wchar_t * PinName ,
    double dValue ,
    const wchar_t * pLabel = NULL ) ;
  bool SendFigure( const char * PinName ,   // Send figure frame to pin (set of coordinate pairs)
    CDPointArray& Figure , 
    const char * pLabel = NULL) ;
  bool SendFigure(const wchar_t * PinName , 
    CDPointArray& Figure , 
    const wchar_t * pLabel = NULL ) ;
  bool SendImage( const char * PinName ,    // Send video frame to pin
    void * pImage ,                         // For BW 8 or 16 bits images
    CSize Size , int iBits , const char * pLabel = NULL) ;
  bool SendImage( const wchar_t * PinName ,
    void * pImage ,
    CSize Size , int iBits , const wchar_t * pLabel = NULL ) ;
    
    // Print properties. Can be used, when 
    // Several properties are should be read 
    
  bool PrintProperties( const char * GadgetName ,
    char * pBuffer , // if zero, ulPropValLen will return necessary length
    ULONG& ulPropValLen ) ; // In-Out: In - buffer length, Out - real string length
  bool PrintProperties( const wchar_t * GadgetName ,
    wchar_t * pBuffer , // if zero, ulPropValLen will return necessary length
    ULONG& ulPropValLen ) ; // In-Out: In - buffer length, Out - real string length
  bool ConnectGraph::ScanProperties(
    const char * GadgetName , const char * pProperties ) ;
    // Set/Get text like property
  bool SetProperty( const wchar_t * GadgetName , 
    const wchar_t * PropertyName , 
    const wchar_t * PropertyValue ) ;
  bool GetProperty( const wchar_t * GadgetName ,
    const wchar_t * PropertyName , 
    wchar_t * PropertyValue , ULONG ulPropValLen ) ;
  bool SetProperty( const char * GadgetName , 
    const char * PropertyName , 
    const char * PropertyValue ) ;
  bool GetProperty( const char * GadgetName , 
    const char * PropertyName , 
    char * PropertyValue , 
    ULONG ulPropValLen ) ;

  // Set/Get integer property

  bool SetProperty( const wchar_t * GadgetName , 
    const wchar_t * PropertyName , 
    int PropertyValue ) ;
  bool GetProperty( const wchar_t * GadgetName ,
    const wchar_t * PropertyName , 
    int& PropertyValue ) ;
  bool SetProperty( const char * GadgetName , 
    const char * PropertyName , 
    int PropertyValue ) ;
  bool GetProperty( const char * GadgetName , 
    const char * PropertyName , 
    int& PropertyValue ) ;
 
  // Set/Get double property
  
  bool SetProperty( const wchar_t * GadgetName , 
    const wchar_t * PropertyName , 
    double PropertyValue ) ;
  bool GetProperty( const wchar_t * GadgetName ,
    const wchar_t * PropertyName , 
    double& PropertyValue ) ;
  bool SetProperty( const char * GadgetName , 
    const char * PropertyName , 
    double PropertyValue ) ;
  bool GetProperty( const char * GadgetName , 
    const char * PropertyName , 
    double& PropertyValue ) ;


    // Set/Get gadget working mode
  bool SetWorkingMode( const wchar_t * GadgetName , int iWorkingMode ) ; // 0 - reject input frame
  // 1 - pass through
  // 2 - process (default value)
  bool GetWorkingMode( const wchar_t * GadgetName , int& iWorkingMode ) ;
  bool SetWorkingMode( const char * GadgetName , int iWorkingMode ) ;
  bool GetWorkingMode( const char * GadgetName , int& iWorkingMode ) ;

  map<string, list<const CGadgetInfo*> > getGadgetsByClassNames();

  bool IsRunning() const;

  bool IsPaused() const;

  FXString m_EvaluationMsg ;
};


#endif
