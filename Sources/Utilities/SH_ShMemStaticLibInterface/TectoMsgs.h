#ifndef TECTO_MSG_INC
#define TECTO_MSG_INC

#include <atltypes.h>
//#pragma warning(disable:4996)

#define IP_MAX_PARAMS 7
#define MAX_MSG_SIZE_MB 4

extern "C" char _BufForTextOut[ 4096 ] ;

enum TectoMsgId
{
  TM_Dummy = 0 ,
  TM_NoWaiting = TM_Dummy ,
  TM_Init = 1 ,
  TM_Terminate ,
  TM_Calibration ,
  TM_SetFLower ,
  TM_GetParameterList ,
  TM_AnalyzeFlower ,

  TM_OK = 1000 ,
  TM_ERROR ,
  TM_FileOpenError ,
  TM_CalibSetupError ,
  TM_CalibError ,
  TM_SetFlowerError ,
  TM_TrayEmpty ,
  TM_NotInRange ,
  TM_ClippedByTray ,
  TM_AnalyzeTechnicalError ,

  TM_Simu_Init = 2000 ,
  TM_Simu_Terminate ,
  TM_Simu_Calibration ,
  TM_Simu_SetFLower ,
  TM_Simu_GetParameterList ,
  TM_Simu_AnalyzeFlower ,

  TM_GetSimulationImage = 3000 ,
  TM_SimuImageArrived 
};

struct CalibContent
{
  int m_iNFiles ;
  int m_CamPositions[ 10 ] ;
  char m_CalibFileNames[ 10 ][ 256 ] ;
};

struct RoiPoints
{
  CPoint topleft;
  CPoint topright;
  CPoint bottomleft;
  CPoint bottomright;
};

struct FlowerSetContent
{
  char m_FlowerName[ 128 ] ;
  int  m_iCamCount ;
  int  m_CamPositions[ 10 ] ;
  char m_CalibFileNames[ 10 ][ 256 ] ;
  RoiPoints m_ROI[ 10 ] ;
  char m_ConfigFileName[256] ;
};

struct ParameterListContent
{
  char m_FlowerName[ 128 ] ; // in
  int  m_iParamCount ;       // out
  char m_Parameters[ IP_MAX_PARAMS + 1 ][ 100 ] ; // out
  char m_Diagnostics[ 1024 ] ;
};

struct AnalyzeFlowerContent
{
  int  m_iTicket ;
  char m_FlowerName[ 128 ] ;
  int  m_iImageCount ; // correspondent to camera count in FlowerSet
  float m_Results[ IP_MAX_PARAMS ] ;
  BYTE m_Images[1] ;
};

struct SimulationImageContent
{
  char m_FlowerName[ 256 ] ;
  int  m_iNImages ; // return value
  BYTE m_Images[ 1 ] ; // series of images with pointers inside this structure
};

class TectoMsg
{
public:
  static TectoMsg * CreateMsg( TectoMsgId Id , DWORD dwDataLen ) ;
  TectoMsg * Copy() ;
  BYTE * GetData() { return m_MsgData ; } ;
  DWORD GetMsgLen()
  {
    return sizeof( m_Id ) + sizeof( m_MsgDataLen ) + m_MsgDataLen ;
  } ;
  TectoMsgId m_Id ;
  DWORD     m_MsgDataLen ;
  BYTE      m_MsgData[ 1 ] ;
};

//#pragma warning(default:4996)

#endif // TECTO_MSG_INC
