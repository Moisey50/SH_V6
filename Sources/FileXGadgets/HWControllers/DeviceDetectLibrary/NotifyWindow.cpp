
#include "StdAfx.h"
#include "NotifyWindow.h"


using namespace DeviceDetectLibrary;

static const wchar_t WINDOW_CLASS[] = L"NotifyWindow";
static const wchar_t WINDOW_TITLE[] = L"winTitle";

static NotifyWindow *g_pNotifyWindow = NULL;

NotifyWindow::NotifyWindow(IDeviceChanged& deviceChanged)
  : m_hWnd(NULL)
  , m_toDestroy(false)
  , m_backgroundWorker(ThreadProc)
  , m_eventStarted()
  , m_deviceChanged(deviceChanged)
{
  if (g_pNotifyWindow != NULL)
  {
    throw std::runtime_error("Only one instance of the NotifyWindow can be created");
  }
  g_pNotifyWindow = this;
  m_backgroundWorker.Start(this);
  m_eventStarted.Wait(INFINITE);
}

NotifyWindow::~NotifyWindow()
{
  m_toDestroy = true;                   //1. Allows to break the ThreadProc loop
  PostMessage(m_hWnd, WM_CLOSE, 0, 0);  //2. Release the GetMessage(...) in the ThreadProc (should be before call the WaitForSingleObject)
  m_backgroundWorker.Wait(INFINITE);
  DestroyInstance();
  g_pNotifyWindow = NULL;
}

bool NotifyWindow::RegisterNotifyWndwClass(HINSTANCE hInstance)
{
  bool res = false;
  WNDCLASSEX wcex;
  ZeroMemory(&wcex, sizeof(wcex));

  int isPresent = GetClassInfoEx(hInstance, WINDOW_CLASS, &wcex);
  if(isPresent)
    res = wcex.lpfnWndProc == NotifyWindow::WndProc;
  else
  {
    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style			= CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc	= NotifyWindow::WndProc; //Window callback handler
    wcex.cbClsExtra		= 0;
    wcex.cbWndExtra		= 0;
    wcex.hInstance		= hInstance;
    wcex.hIcon			= NULL;
    wcex.hCursor		= NULL;
    wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW);
    wcex.lpszMenuName	= NULL;
    wcex.lpszClassName	= WINDOW_CLASS;
    wcex.hIconSm		= NULL;

    res = RegisterClassEx(&wcex) != 0;
  }
  return res;
}

bool NotifyWindow::InitInstance(HINSTANCE hInstance)
{
  m_hWnd = CreateWindow
    ( WINDOW_CLASS
    , WINDOW_TITLE
    , WS_ICONIC
    , 0, 0
    , CW_USEDEFAULT
    , 0
    , NULL, NULL
    , hInstance
    , NULL);

  if (!m_hWnd)
  {
    return false;
  }

  ShowWindow(m_hWnd, SW_HIDE);
  UpdateWindow(m_hWnd);

  return true;
}

int DeviceDetectLibrary::NotifyWindow::DestroyInstance()
{
  PostMessage(m_hWnd, WM_CLOSE, 0, 0);
  bool isClosed = CloseWindow(m_hWnd) != 0;
  bool isDestroyed = DestroyWindow(m_hWnd) != 0;
  if(isDestroyed && isClosed)
    m_hWnd = NULL;

  return isClosed && isDestroyed;
}

int DeviceDetectLibrary::NotifyWindow::UnregisterNotifyWndwClass( HINSTANCE hInstance )
{
  return UnregisterClass(WINDOW_CLASS, hInstance);
}

std::vector<std::wstring> DrivesFromMask(DWORD unitmask)
{
  std::vector<std::wstring> result;
  std::wstring path(L"?:\\");
  DWORD localUnitmask(unitmask);
  for (wchar_t i = L'A'; i <= L'Z'; ++i)
  {
    if (0x01 == (localUnitmask & 0x1))
    {
      path[0] = i;// + L'A';
      result.push_back(path);
    }
    localUnitmask >>= 1;
  }

  return result;
}

LRESULT CALLBACK NotifyWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
  case WM_PAINT:
    {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hWnd, &ps);
      EndPaint(hWnd, &ps);
    }
    break;
  case WM_DEVICECHANGE:
    {
      bool isArrived = DBT_DEVICEARRIVAL == wParam;

      if (isArrived || (wParam == DBT_DEVICEREMOVECOMPLETE))
      {
        try
        {
          DEV_BROADCAST_HDR *pHeader = reinterpret_cast<DEV_BROADCAST_HDR*>(lParam);
          switch (pHeader->dbch_devicetype)
          {
          case DBT_DEVTYP_VOLUME:
            {
              DEV_BROADCAST_VOLUME *pDevice = reinterpret_cast<DEV_BROADCAST_VOLUME*>(lParam);
              VolumeAddRemove(DrivesFromMask(pDevice->dbcv_unitmask), isArrived);
            }
            break;
          case DBT_DEVTYP_DEVICEINTERFACE:
            {
              DEV_BROADCAST_DEVICEINTERFACE *pDevice = reinterpret_cast<DEV_BROADCAST_DEVICEINTERFACE*>(lParam);
              DeviceInterfaceAddRemove(pDevice->dbcc_classguid, Utilities::StringUpper(pDevice->dbcc_name), isArrived);
            }
            break;
          default:
            break;
          }
        }
        catch(const std::runtime_error& ex)
        {
          ATLTRACE("\nNotifyWindow::WndProc exception caught: %s (type %s) " , ex.what() , typeid(ex).name() ) ;
        }
      }
    }
    break;
  case WM_DESTROY:
    {
      PostQuitMessage(0);
    }
    break;
  default:
    {
      return DefWindowProc(hWnd, message, wParam, lParam);
    }
  }
  return 0;
}

HWND NotifyWindow::GetHWND() const
{
  return m_hWnd;
}


void NotifyWindow::ThreadProc( void *pContext )
{
  NotifyWindow *This = static_cast<NotifyWindow*>(pContext);

  HINSTANCE hInstance = ::GetModuleHandle(NULL);

  int isRegistred = This->RegisterNotifyWndwClass(hInstance);

  if (isRegistred && This->InitInstance(hInstance))
  {        
    This->m_eventStarted.Set();

    // Main message loop:
    MSG msg;
    BOOL bRet(FALSE);

    while(!This->m_toDestroy
      && (bRet = GetMessage( &msg, This->m_hWnd, 0, 0 )) != FALSE
      && bRet != -1)
    {
      TranslateMessage(&msg); 
      DispatchMessage(&msg);
    }
  }
  int isDestroyed = This->DestroyInstance();
  int isUnregitred = This->UnregisterNotifyWndwClass(hInstance);
}


void NotifyWindow::VolumeAddRemove(const std::vector<std::wstring>& drives, bool isArrived )
{
  for (std::vector<std::wstring>::const_iterator ci = drives.begin(); ci != drives.end(); ++ci)
  {
    if (isArrived)
      g_pNotifyWindow->m_deviceChanged.VolumeArrival(*ci);
    else
      g_pNotifyWindow->m_deviceChanged.VolumeRemoved(*ci);
  }
}

void NotifyWindow::DeviceInterfaceAddRemove(const GUID& guid, const DeviceInfo::DeviceId& devId, bool isArrived)
{
  if (isArrived)
    g_pNotifyWindow->m_deviceChanged.InterfaceArrival(guid);
  else
    g_pNotifyWindow->m_deviceChanged.InterfaceRemoved(devId);
}


