#include "StdAfx.h"
#include "clserlynx.h"
#include "CameraMDC2048.h"


//#define NORMAL  0
//#define WINDOW  1
//#define BINNING 2

CCLSerLynx::CCLSerLynx(void)
{
}

CCLSerLynx::~CCLSerLynx(void)
{
}

int CCLSerLynx::WriteASCII(char* ascii, char* responce)
{
	CWinApp *pApp = AfxGetApp();
	int nRet = -57; 

	char send[MAX_PATH];
	ZeroMemory(send,MAX_PATH);
	ULONG ulSize = 1;
	strcpy(send,ascii);
	ulSize = (ULONG)strlen(send);
// Commands 'gcs' and 'gce' need delay to allow camera to snap minimum 2 frames to 
//	calculate speed and exposure. 
//	Recomended values are:
//	1500 ms for 'gcs'
//  1000 ms for 'gce'

	nRet = Write(send, &ulSize); 
// Some commands have too much data to transmit from camera to computer, 
//	so it is recomended to delay read of response.
//  Recomended value for delat is 500 ms.
//  Commands are:
//		stu
//		lff
//		lfu
//		gws
//		slt
//		slh

	if(nRet == 0)	
	{
		char* rsp = responce;
		int len;
		do{
			ulSize = 128;
			nRet = Read(rsp,&ulSize);
			rsp+=ulSize;
			if(nRet < 0)
				break;
			if((rsp - responce) >= 2048-128)
				break;
		}while(ulSize == 128);
		len = (int)(rsp-responce);
		SkipEscSequence(responce,len);
	}
	return nRet;
}

// Response from camera includes escape sequence that contains '\0' char.
// We recomends to skip escape sequence before parsing response
void CCLSerLynx::SkipEscSequence(char *pcBuffer,int& ind)
{
	char* ptr = pcBuffer;
	int nd = 0;
	WORD* wrd;
	for(int i = 0; i < ind; i++)
	{
		wrd = (WORD*)(pcBuffer+i);
		if(*wrd == 0x5b1b)
		{
			i+=6;
		}
		ptr[nd] = pcBuffer[i];
		nd++;
	}
	for(i = nd; i <ind; i++)
	{
		ptr[i] = 0;
	}
}

int CCLSerLynx::CheckConnectedCamera()
{
	int nRet;

	char rsp[2048];
	ZeroMemory(rsp,sizeof(rsp));

	char cmd[MAX_PATH];
	ZeroMemory(cmd,MAX_PATH);

	sprintf(cmd,"gmn\r\n");

	nRet = WriteASCII(cmd,rsp);

	m_CamID = 0xFFFFFFFF;
	m_CamName = _T("Unknown");

	if(strstr(rsp,"IPX-1M48-LC") != NULL)
	{
		m_CamID = LYNX_ID;
		m_CamName = _T("IPX-1M48-LC");
	}
	else if(strstr(rsp,"IPX-1M48-L") != NULL)
	{
		m_CamID = LYNX_ID;
		m_CamName = _T("IPX-1M48-L");
	}
	else if(strstr(rsp,"IPX-2M30-LC") != NULL)
	{
		m_CamID = LYNX_ID;
		m_CamName = _T("IPX-2M30-LC");
	}
	else if(strstr(rsp,"IPX-2M30-L") != NULL)
	{
		m_CamID = LYNX_ID;
		m_CamName = _T("IPX-2M30-L");
	}
	else if(strstr(rsp,"IPX-2M30H-LC") != NULL)
	{
		m_CamID = LYNX_ID;
		m_CamName = _T("IPX-2M30H-LC");
	}
	else if(strstr(rsp,"IPX-2M30H-L") != NULL)
	{
		m_CamID = LYNX_ID;
		m_CamName = _T("IPX-2M30H-L");
	}
	else if(strstr(rsp,"IPX-4M15-LC") != NULL)
	{
		m_CamID = LYNX_ID;
		m_CamName = _T("IPX-4M15-LC");
	}
	else if(strstr(rsp,"IPX-4M15-L") != NULL)
	{
		m_CamID = LYNX_ID;
		m_CamName = _T("IPX-4M15-L");
	}
	else if(strstr(rsp,"IPX-11M5-LC") != NULL)
	{
		m_CamID = LYNX_ID;
		m_CamName = _T("IPX-11M5-LC");
	}
	else if(strstr(rsp,"IPX-11M5-L") != NULL)
	{
		m_CamID = LYNX_ID;
		m_CamName = _T("IPX-11M5-L");
	}
	else if(strstr(rsp,"IPX-VGA210-LC") != NULL)
	{
		m_CamID = LYNX_ID;
		m_CamName = _T("IPX-VGA210-LC");
	}
	else if(strstr(rsp,"IPX-VGA210-L") != NULL)
	{
		m_CamID = LYNX_ID;
		m_CamName = _T("IPX-VGA210-L");
	}
	else if(strstr(rsp,"IPX-VGA90-LC") != NULL)
	{
		m_CamID = LYNX_ID;
		m_CamName = _T("IPX-VGA90-LC");
	}
	else if(strstr(rsp,"IPX-VGA90-L") != NULL)
	{
		m_CamID = LYNX_ID;
		m_CamName = _T("IPX-VGA90-L");
	}
	else if(strstr(rsp,"IPX-VGA120-LC") != NULL)
	{
		m_CamID = LYNX_ID;
		m_CamName = _T("IPX-VGA120-LC");
	}
	else if(strstr(rsp,"IPX-VGA120-L") != NULL)
	{
		m_CamID = LYNX_ID;
		m_CamName = _T("IPX-VGA120-L");
	}

  /*int ret = -1;
  char cmd1[100];
  char rsp1[2048];
  ZeroMemory(rsp1,sizeof(rsp1));
  sprintf(cmd1,"ssm on\r\n");
  WriteASCII(cmd1,rsp1);
  if(strstr(rsp1,"Error") == NULL)
  {
    ret = 0;
  }
  else
  {
    ret = -1;
    return ret;
  }*/


	return (int)m_CamID;
}


int CCLSerLynx::SetShutter(int nShut,char * cError)
{
	int ret = -1;
	char cmd[100];
	char rsp[2048];
	ZeroMemory(rsp,sizeof(rsp));
	sprintf(cmd,"sst %d\r\n",nShut);
	WriteASCII(cmd,rsp);
	if(strstr(rsp,"Error") == NULL)
		ret = 0;
  else
    strcpy(cError,rsp);
	return ret;
}
int CCLSerLynx::GetShutter(int& nShut)
{
	int ret = -1;
	char cmd[100];
	char rsp[2048];
	char szNum[20];
	ZeroMemory(rsp,sizeof(rsp));
	ZeroMemory(szNum,sizeof(szNum));
	sprintf(cmd, "gst\r\n");
	ret = WriteASCII(cmd,rsp);
	if(strstr(rsp,"Error") == NULL)
		ret = 0;
	else
		return -1;

	int ind=0;
	int cnt = 0;
	for(int i = 0; i < 2048;i++)
	{
//	There is some '\0' characters (not more then two in a row)
//  may occur in the camera response. Just skip them.
		if(rsp[i] == 0)
		{
			cnt++;
			if(cnt > 3)
			{
// If there more then 3 '\0' characters in a row - end of response reached.
				break;
			}
		}
		else
		{
			cnt = 0;
		}
// Response may contain digits or 'off' word
// If there is digit character in response - accumulate it in szNum.
		if(isdigit(rsp[i]) != 0)
		{
			szNum[ind] = rsp[i];
			ind++;
		}
	}
// Convert number from response
	if(strlen(szNum) > 0)
	{
		nShut = atoi(szNum);
		ret = 0;
	}
// Or check response for word 'off'
	else if(strstr(rsp,"off\r\n") == NULL)
	{
		nShut = -1;
		ret = 0;
	}
	return ret;
}
int CCLSerLynx::SetGain1(float fGain)
{
	int ret = -1;
	char cmd[100];
	char rsp[2048];
	ZeroMemory(rsp,sizeof(rsp));
	sprintf(cmd,"sag 1 %.2f\r\n",fGain);
	WriteASCII(cmd,rsp);
	if(strstr(rsp,"Error") == NULL)
	{
		ret = 0;
	}
	return ret;
}
int CCLSerLynx::GetGain1(float& fGain)
{
	int ret = -1;
	char cmd[100];
	char rsp[2048];
	ZeroMemory(rsp,sizeof(rsp));
	sprintf(cmd,"gag 1\r\n");
	WriteASCII(cmd,rsp);
	if(strstr(rsp,"Error") == NULL)
	{
		ret = 0;
	}
	else
	{
		return ret;
	}
	char* ind = strstr(rsp,"dB");
	if(ind != NULL)
	{
		*ind = '\0';
		ind = strrchr(rsp,'\n');
		if(ind != NULL)
		{
			ind++;
			fGain = (float)atof(ind);
		}
	}
	return ret;
}
int CCLSerLynx::SetShutterEnable(bool bEnable)
{
	int ret = -1;
	char cmd[100];
	char rsp[2048];
	ZeroMemory(rsp,sizeof(rsp));
	if(!bEnable)
	{
		sprintf(cmd,"sst off\r\n");
		WriteASCII(cmd,rsp);
		if(strstr(rsp,"Error") == NULL)
		{
			ret = 0;
		}
		else
		{
			ret = -1;
			return ret;
		}
	}
	else
	{
		ret = 0;
	}
	return ret;
}
int CCLSerLynx::GetShutterEnable(bool& bEnable)
{
	int ret = -1;
	char cmd[100];
	char rsp[2048];
	ZeroMemory(rsp,sizeof(rsp));
	sprintf(cmd,"gst\r\n");
	WriteASCII(cmd,rsp);
	if(strstr(rsp,"Error") == NULL)
	{
		ret = 0;
	}
	else
	{
		return ret;
	}
	if(strstr(rsp,"off\r\n") == NULL)
	{
		bEnable = true;
		ret = 0;
	}
	else
	{
		bEnable = false;
		ret = 0;
	}
	return ret;
}
int CCLSerLynx::ReadReg(BYTE Address,BYTE *Reg)
{
  int ret = -1;
  char cmd[100];
  char rsp[2048];
  ZeroMemory(rsp,sizeof(rsp));

  sprintf(cmd,"peek %d \r\n", Address);
  WriteASCII(cmd,rsp);
  if(strstr(rsp,"Error") == NULL)
  {
    ret = 0;
  }
  else
  {
    return ret;
  }
  char* ind = strstr(rsp,"dB");
  if(ind != NULL)
  {
    *ind = '\0';
    ind = strrchr(rsp,'\n');
    if(ind != NULL)
    {
      ind++;
      *Reg = (BYTE)atoi(ind);
    }
  }
  return ret;
}
int CCLSerLynx::WriteReg(BYTE Address,BYTE  Reg)
{
  int ret = -1;
  char cmd[100];
  char rsp[2048];
  ZeroMemory(rsp,sizeof(rsp));
  sprintf(cmd,"poke %d %d \r\n",Address,Reg);
  WriteASCII(cmd,rsp);
  if(strstr(rsp,"Error") == NULL)
  {
    ret = 0;
  }
  return ret;
}

int CCLSerLynx::GetDualMode(int& nDualMode)
{
  int ret = -1;
  char cmd[100];
  char rsp[2048];
  ZeroMemory(rsp,sizeof(rsp));
  sprintf(cmd,"gdm\r\n");
  WriteASCII(cmd,rsp);
  if(strstr(rsp,"Error") == NULL)
  {
    ret = 0;
  }
  else
  {
    return ret;
  }
  if(strstr(rsp,"off\r\n") == NULL)
  {
    nDualMode = 1;
    ret = 0;
  }
  else
  {
    nDualMode = 0;
    ret = 0;
  }
  return ret;
}


int CCLSerLynx::GetHorizMode(int& nHorizMode)
{
  int ret = -1;
  char cmd[100];
  char rsp[2048];
  ZeroMemory(rsp,sizeof(rsp));
  sprintf(cmd,"ghm\r\n");
  WriteASCII(cmd,rsp);
  if(strstr(rsp,"Error") == NULL)
  {
    ret = 0;
  }
  else
  {
    return ret;
  }
  if(strstr(rsp,"n\r\n") != NULL)
  {
    nHorizMode = NORMAL;
    ret = 0;
  }
  else if(strstr(rsp,"w\r\n") != NULL)
  {
    nHorizMode = WINDOW;
    ret = 0;
  }
  else 
  {
    nHorizMode = BINNING;
    ret = 0;
  }
  return ret;

}
int CCLSerLynx::GetVertMode(int& nVertMode)
{
  int ret = -1;
  char cmd[100];
  char rsp[2048];
  ZeroMemory(rsp,sizeof(rsp));
  sprintf(cmd,"gvm\r\n");
  WriteASCII(cmd,rsp);
  if(strstr(rsp,"Error") == NULL)
  {
    ret = 0;
  }
  else
  {
    return ret;
  }
  if(strstr(rsp,"n\r\n")!= NULL)
  {
    nVertMode = NORMAL;
    ret = 0;
  }
  else if(strstr(rsp,"w\r\n") != NULL)
  {
    nVertMode = WINDOW;
    ret = 0;
  }
  else 
  {
    nVertMode = BINNING;
    ret = 0;
  }
  return ret;

}


int CCLSerLynx::GetTestMode(int& nTestMode)
{
  int ret = -1;
  char cmd[100];
  char rsp[2048];
  ZeroMemory(rsp,sizeof(rsp));
  sprintf(cmd,"gtm\r\n");
  WriteASCII(cmd,rsp);
  if(strstr(rsp,"Error") == NULL)
  {
    ret = 0;
  }
  else
  {
    return ret;
  }
  if(strstr(rsp,"off\r\n") == NULL)
  {
    nTestMode = 1;
    ret = 0;
  }
  else
  {
    nTestMode = 0;
    ret = 0;
  }
  return ret;

}
int CCLSerLynx::SetDualMode(int nDualMode)
{
  int ret = -1;
  char cmd[100];
  char rsp[2048];
  ZeroMemory(rsp,sizeof(rsp));
  if(!nDualMode)
  {
    sprintf(cmd,"sdm off\r\n");
    WriteASCII(cmd,rsp);
    if(strstr(rsp,"Error") == NULL)
    {
      ret = 0;
    }
    else
    {
      ret = -1;
      return ret;
    }
  }
  else
  {
    ret = 0;
  }
  return ret;
}


int CCLSerLynx::SetHorizMode(int nHorizMode)
{
  int ret = -1;
  char cmd[100];
  char rsp[2048];
  ZeroMemory(rsp,sizeof(rsp));
  switch(nHorizMode)
  {
  case NORMAL:
        {
          sprintf(cmd,"shm n\r\n");
          WriteASCII(cmd,rsp);
          if(strstr(rsp,"Error") == NULL)
          {
            ret = 0;
          }
          else
          {
            ret = -1;
            return ret;
          }
          break;
        }
         
  case WINDOW:
        {
          sprintf(cmd,"shm w\r\n");
          WriteASCII(cmd,rsp);
          if(strstr(rsp,"Error") == NULL)
          {
            ret = 0;
          }
          else
          {
            ret = -1;
            return ret;
          }
         break;
        }
        
  case BINNING:
        {
          sprintf(cmd,"shm b\r\n");
          WriteASCII(cmd,rsp);
          if(strstr(rsp,"Error") == NULL)
          {
            ret = 0;
          }
          else
          {
            ret = -1;
            return ret;
          }
          break;
        }

  }
 
  ret = 0;
 
  return ret;

}
int CCLSerLynx::SetVertMode(int nVertMode)
{
  int ret = -1;
  char cmd[100];
  char rsp[2048];
  ZeroMemory(rsp,sizeof(rsp));
  switch(nVertMode)
  {
  case NORMAL:
    {
      sprintf(cmd,"svm n\r\n");
      WriteASCII(cmd,rsp);
      if(strstr(rsp,"Error") == NULL)
      {
        ret = 0;
      }
      else
      {
        ret = -1;
        return ret;
      }
      break;
    }
  case WINDOW:
    {
      sprintf(cmd,"svm w\r\n");
      WriteASCII(cmd,rsp);
      if(strstr(rsp,"Error") == NULL)
      {
        ret = 0;
      }
      else
      {
        ret = -1;
        return ret;
      }
      break;
    }
  case BINNING:
    {
      sprintf(cmd,"svm b\r\n");
      WriteASCII(cmd,rsp);
      if(strstr(rsp,"Error") == NULL)
      {
        ret = 0;
      }
      else
      {
        ret = -1;
        return ret;
      }
      break;
    }
  }
  ret = 0;
  return ret;

}


int CCLSerLynx::SetTestMode(int nTestMode)
{
  int ret = -1;
  char cmd[100];
  char rsp[2048];
  ZeroMemory(rsp,sizeof(rsp));
  if(!nTestMode)
  {
    sprintf(cmd,"stm off\r\n");
    WriteASCII(cmd,rsp);
    if(strstr(rsp,"Error") == NULL)
    {
      ret = 0;
    }
    else
    {
      ret = -1;
      return ret;
    }
  }
  else
  {
    ret = 0;
  }
  return ret;

}

int CCLSerLynx::SetBitDepth(int nBitDepth)
{
  int ret = -1;
  char cmd[100];
  char rsp[2048];
  ZeroMemory(rsp,sizeof(rsp));
  sprintf(cmd,"sbd %d\r\n",nBitDepth);
  WriteASCII(cmd,rsp);
  if(strstr(rsp,"Error") == NULL)
  {
    ret = 0;
  }
  return ret;
}
int CCLSerLynx::SetFrameRate(int nFrameRate)
{
  int ret = -1;
  char cmd[100];
  char rsp[2048];
  ZeroMemory(rsp,sizeof(rsp));
  sprintf(cmd,"sfr %d\r\n",nFrameRate);
  WriteASCII(cmd,rsp);
  if(strstr(rsp,"Error") == NULL)
  {
    ret = 0;
  }
  return ret;
}
int CCLSerLynx::GetBitDepth(int& nBitDepth)
{
  int ret = -1;
  char cmd[100];
  char rsp[2048];
  char szNum[20];
  ZeroMemory(rsp,sizeof(rsp));
  ZeroMemory(szNum,sizeof(szNum));
  sprintf(cmd,"gbd \r\n");
  WriteASCII(cmd,rsp);
  if(strstr(rsp,"Error") == NULL)
    ret = 0;
  else
    return -1;

  int ind = 0;
  int cnt = 0;
  for(int i = 0; i < 2048;i++)
  {
    //	There is some '\0' characters (not more then two in a row)
    //  may occur in the camera response. Just skip them.
    if(rsp[i] == 0)
    {
      cnt++;
      if(cnt > 3)
      {
        // If there more then 3 '\0' characters in a row - end of response reached.
        break;
      }
    }
    else
    {
      cnt = 0;
    }
    // Response may contain digits or 'off' word
    // If there is digit character in response - accumulate it in szNum.
    if(isdigit(rsp[i]) != 0)
    {
      szNum[ind] = rsp[i];
      ind++;
    }
  }
  // Convert number from response
  if(strlen(szNum) > 0)
  {
    nBitDepth = atoi(szNum);
    ret = 0;
  }
  return ret;
}

int CCLSerLynx::GetFrameRate(int& nFrameRate)
{
  int ret = -1;
  char cmd[100];
  char rsp[2048];
  char szNum[20];
  ZeroMemory(rsp,sizeof(rsp));
  ZeroMemory(szNum,sizeof(szNum));
  sprintf(cmd,"gfr \r\n");
  WriteASCII(cmd,rsp);
  if(strstr(rsp,"Error") == NULL)
    ret = 0;
  else
    return -1;

  int ind = 0;
  int cnt = 0;
  for(int i = 0; i < 2048;i++)
  {
    //	There is some '\0' characters (not more then two in a row)
    //  may occur in the camera response. Just skip them.
    if(rsp[i] == 0)
    {
      cnt++;
      if(cnt > 3)
      {
        // If there more then 3 '\0' characters in a row - end of response reached.
        break;
      }
    }
    else
    {
      cnt = 0;
    }
    // Response may contain digits or 'off' word
    // If there is digit character in response - accumulate it in szNum.
    if(isdigit(rsp[i]) != 0)
    {
      szNum[ind] = rsp[i];
      ind++;
    }
  }
  // Convert number from response
  if(strlen(szNum) > 0)
  {
    nFrameRate = atoi(szNum);
    ret = 0;
  }
  return ret;
}
int CCLSerLynx::GetCamSpeed(double& nSpeed)
{
  int ret = -1;
  char cmd[100];
  char rsp[2048];
  char szNum[20];
  ZeroMemory(rsp,sizeof(rsp));
  ZeroMemory(szNum,sizeof(szNum));
  sprintf(cmd,"gcs \r\n");
  WriteASCII(cmd,rsp);
  if(strstr(rsp,"Error") == NULL)
    ret = 0;
  else
    return -1;

  int ind = 0;
  int cnt = 0;
  for(int i = 0; i < 2048;i++)
  {
    //	There is some '\0' characters (not more then two in a row)
    //  may occur in the camera response. Just skip them.
    if(rsp[i] == 0)
    {
      cnt++;
      if(cnt > 3)
      {
        // If there more then 3 '\0' characters in a row - end of response reached.
        break;
      }
    }
    else
    {
      cnt = 0;
    }
    // Response may contain digits or 'off' word
    // If there is digit character in response - accumulate it in szNum.
    if(isdigit(rsp[i]) != 0 || rsp[i]==46)
    {
      szNum[ind] = rsp[i];
      ind++;
    }
  }
  // Convert number from response
  if(strlen(szNum) > 0)
  {
    nSpeed = atof(szNum);
    ret = 0;
  }
  return ret;
}
int CCLSerLynx::SetStrobePos(int nStrobPos)
{
  int ret = -1;
  char cmd[100];
  char rsp[2048];
  ZeroMemory(rsp,sizeof(rsp));
  if(!nStrobPos)
  {
    sprintf(cmd,"ssp off\r\n");
    WriteASCII(cmd,rsp);
    if(strstr(rsp,"Error") == NULL)
    {
      ret = 0;
    }
    else
    {
      ret = -1;
      return ret;
    }
  }
  else
  {
    sprintf(cmd,"ssp %d\r\n",nStrobPos);
    WriteASCII(cmd,rsp);
    if(strstr(rsp,"Error") == NULL)
    {
      ret = 0;
    }
    else
    {
      ret = -1;
      return ret;
    }  
  }
  return ret;

}
int CCLSerLynx::GetStrobePos(int& nStrobPos)
{
  int ret = -1;
  char cmd[100];
  char rsp[2048];
  char szNum[20];
  ZeroMemory(rsp,sizeof(rsp));
  ZeroMemory(szNum,sizeof(szNum));
  sprintf(cmd,"gsp \r\n");
  WriteASCII(cmd,rsp);
  if(strstr(rsp,"Error") == NULL)
    ret = 0;
  else
    return -1;
  if(strstr(rsp,"off\r\n") == NULL)
  {
    nStrobPos = 0;
    ret = 0;
    return 0;
  }

  int ind = 0;
  int cnt = 0;
  for(int i = 0; i < 2048;i++)
  {
    //	There is some '\0' characters (not more then two in a row)
    //  may occur in the camera response. Just skip them.
    if(rsp[i] == 0)
    {
      cnt++;
      if(cnt > 3)
      {
        // If there more then 3 '\0' characters in a row - end of response reached.
        break;
      }
    }
    else
    {
      cnt = 0;
    }
    // Response may contain digits or 'off' word
    // If there is digit character in response - accumulate it in szNum.
    if(isdigit(rsp[i]) != 0)
    {
      szNum[ind] = rsp[i];
      ind++;
    }
  }
  // Convert number from response
  if(strlen(szNum) > 0)
  {
    nStrobPos = atoi(szNum);
    ret = 0;
  }
  return ret;
}
int CCLSerLynx::SetExtTrigger(int nTrigger)
{
  int ret = -1;
  char cmd[100];
  char rsp[2048];
  ZeroMemory(rsp,sizeof(rsp));
  if(!nTrigger)
  {
    sprintf(cmd,"str off\r\n");
    WriteASCII(cmd,rsp);
    if(strstr(rsp,"Error") == NULL)
    {
      ret = 0;
    }
    else
    {
      ret = -1;
      return ret;
    }
  }
  else
  {
    sprintf(cmd,"str et s\r\n");
    WriteASCII(cmd,rsp);
    if(strstr(rsp,"Error") == NULL)
    {
      ret = 0;
    }
    else
    {
      ret = -1;
      return ret;
    } 
  }
  return ret;

}
int CCLSerLynx::GetExtTrigger(int& nTrigger)
{
  int ret = -1;
  char cmd[100];
  char rsp[2048];
  ZeroMemory(rsp,sizeof(rsp));
  sprintf(cmd,"gtr \r\n");
  WriteASCII(cmd,rsp);
  if(strstr(rsp,"Error") == NULL)
  {
    ret = 0;
  }
  else
  {
    return ret;
  }
  if(strstr(rsp,"off\r\n") == NULL)
  {
    nTrigger = 1;
    ret = 0;
  }
  else
  {
    nTrigger = 0;
    ret = 0;
  }
   return ret;
}

int CCLSerLynx::GetPreExp(int& nPreExp)
{
  int ret = -1;
  char cmd[100];
  char rsp[2048];
  char szNum[20];
  ZeroMemory(rsp,sizeof(rsp));
  ZeroMemory(szNum,sizeof(szNum));
  sprintf(cmd,"gpe \r\n");
  WriteASCII(cmd,rsp);
  if(strstr(rsp,"Error") == NULL)
    ret = 0;
  else
    return -1;

  int ind = 0;
  int cnt = 0;
  for(int i = 0; i < 2048;i++)
  {
    //	There is some '\0' characters (not more then two in a row)
    //  may occur in the camera response. Just skip them.
    if(rsp[i] == 0)
    {
      cnt++;
      if(cnt > 3)
      {
        // If there more then 3 '\0' characters in a row - end of response reached.
        break;
      }
    }
    else
    {
      cnt = 0;
    }
    // Response may contain digits or 'off' word
    // If there is digit character in response - accumulate it in szNum.
    if(isdigit(rsp[i]) != 0)
    {
      szNum[ind] = rsp[i];
      ind++;
    }
  }
  // Convert number from response
  if(strlen(szNum) > 0)
  {
    nPreExp = atoi(szNum);
    ret = 0;
  }
  return ret;
}
int CCLSerLynx::SetPreExp(int nPreExp)
{
  int ret = -1;
  char cmd[100];
  char rsp[2048];
  ZeroMemory(rsp,sizeof(rsp));
  sprintf(cmd,"spe %d\r\n",nPreExp);
  WriteASCII(cmd,rsp);
  if(strstr(rsp,"Error") == NULL)
  {
    ret = 0;
  }
  return ret;
}
int CCLSerLynx::GetTrigDuration(int& nTrigDuration)
{
  int ret = -1;
  char cmd[100];
  char rsp[2048];
  char szNum[20];
  ZeroMemory(rsp,sizeof(rsp));
  ZeroMemory(szNum,sizeof(szNum));
  sprintf(cmd,"gtd \r\n");
  WriteASCII(cmd,rsp);
  if(strstr(rsp,"Error") == NULL)
    ret = 0;
  else
    return -1;

  int ind = 0;
  int cnt = 0;
  for(int i = 0; i < 2048;i++)
  {
    //	There is some '\0' characters (not more then two in a row)
    //  may occur in the camera response. Just skip them.
    if(rsp[i] == 0)
    {
      cnt++;
      if(cnt > 3)
      {
        // If there more then 3 '\0' characters in a row - end of response reached.
        break;
      }
    }
    else
    {
      cnt = 0;
    }
    // Response may contain digits or 'off' word
    // If there is digit character in response - accumulate it in szNum.
    if(isdigit(rsp[i]) != 0)
    {
      szNum[ind] = rsp[i];
      ind++;
    }
  }
  // Convert number from response
  if(strlen(szNum) > 0)
  {
    nTrigDuration = atoi(szNum);
    ret = 0;
  }
  return ret;
}
int CCLSerLynx::SetTrigDuration(int nTrigDuration)
{
  int ret = -1;
  char cmd[100];
  char rsp[2048];
  ZeroMemory(rsp,sizeof(rsp));
  sprintf(cmd,"std %d\r\n",nTrigDuration);
  WriteASCII(cmd,rsp);
  if(strstr(rsp,"Error") == NULL)
  {
    ret = 0;
  }
  return ret;
}
int CCLSerLynx::GetAnalogOffset(int &iTap,int& nAnalogOffset)
{
  int ret = -1;
//  char cmd[100];
//  char rsp[2048];
//  char szNum[20];
//  ZeroMemory(rsp,sizeof(rsp));
//  ZeroMemory(szNum,sizeof(szNum));
//  sprintf(cmd,"gao \r\n");
//  WriteASCII(cmd,rsp);
//  if(strstr(rsp,"Error") == NULL)
//    ret = 0;
//  else
//    return -1;
//
//  int ind = 0;
//  int cnt = 0;
//  for(int i = 0; i < 2048;i++)
//  {
//    //	There is some '\0' characters (not more then two in a row)
//    //  may occur in the camera response. Just skip them.
//    if(rsp[i] == 0)
//    {
//      cnt++;
//      if(cnt > 3)
//      {
//        // If there more then 3 '\0' characters in a row - end of response reached.
//        break;
//      }
//    }
//    else
//    {
//      cnt = 0;
//    }
//    // Response may contain digits or 'off' word
//    // If there is digit character in response - accumulate it in szNum.
//    if(isdigit(rsp[i]) != 0)
//    {
//      szNum[ind] = rsp[i];
//      ind++;
//    }
//  }
//  // Convert number from response
//  if(strlen(szNum) > 0)
//  {
//    nTrigDuration = atoi(szNum);
//    ret = 0;
//  }
  return ret;
}
int CCLSerLynx::SetAnalogOffset(int iTap,int nAnalogOffset)
{
  int ret = -1;
  char cmd[100];
  char rsp[2048];
  ZeroMemory(rsp,sizeof(rsp));
  sprintf(cmd,"sao %d %d\r\n",iTap,nAnalogOffset);
  WriteASCII(cmd,rsp);
  if(strstr(rsp,"Error") == NULL)
  {
    ret = 0;
  }
  return ret;
}

//int CCLSerLynx::GetExp(int& nExposure)
//{
//  int ret = -1;
//  char cmd[100];
//  char rsp[2048];
//  char szNum[20];
//  ZeroMemory(rsp,sizeof(rsp));
//  ZeroMemory(szNum,sizeof(szNum));
//  sprintf(cmd,"gce \r\n");
//  WriteASCII(cmd,rsp);
//  if(strstr(rsp,"Error") == NULL)
//    ret = 0;
//  else
//    return -1;
//
//  int ind = 0;
//  int cnt = 0;
//  for(int i = 0; i < 2048;i++)
//  {
//    //	There is some '\0' characters (not more then two in a row)
//    //  may occur in the camera response. Just skip them.
//    if(rsp[i] == 0)
//    {
//      cnt++;
//      if(cnt > 3)
//      {
//        // If there more then 3 '\0' characters in a row - end of response reached.
//        break;
//      }
//    }
//    else
//    {
//      cnt = 0;
//    }
//    // Response may contain digits or 'off' word
//    // If there is digit character in response - accumulate it in szNum.
//    if(isdigit(rsp[i]) != 0)
//    {
//      szNum[ind] = rsp[i];
//      ind++;
//    }
//  }
//  // Convert number from response
//  if(strlen(szNum) > 0)
//  {
//    nExposure = atoi(szNum);
//    ret = 0;
//  }
//  return ret;
//}





