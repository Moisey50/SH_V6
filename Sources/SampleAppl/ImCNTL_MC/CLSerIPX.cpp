#include "StdAfx.h"
#include "clseripx.h"
#include "CameraMDC2048.h"


CCLSerIPX::CCLSerIPX(void)
{
	m_cFirstOp = CLINK_OP_CHAR_1;
	m_cReadOp = CLINK_READ_OP_CHAR;
	m_cWriteOp = CLINK_WRITE_OP_CHAR;
}

CCLSerIPX::~CCLSerIPX(void)
{
}

int CCLSerIPX::ReadReg(BYTE byteAddress, BYTE *pByteData)
{
	ULONG ulSize = 1;
	int nRet;
	nRet = Write((char *)&m_cFirstOp, &ulSize); 
	if(nRet == 0)
	{
		nRet = Write((char *)&byteAddress, &ulSize);
		if(nRet == 0)
		{
			nRet = Write((char *)&m_cReadOp, &ulSize); 
			if(nRet == 0)
				nRet = Write((char *)&m_cReadOp, &ulSize);
				if(nRet == 0)
					nRet = Read((char *)pByteData, &ulSize);
		}
	}
	return nRet;
}

int CCLSerIPX::WriteReg(BYTE byteAddress, BYTE byteData, DWORD dwDelay)
{
	int nRet = -57; 
	ULONG ulSize = 1;
	nRet = Write((char *)&m_cFirstOp, &ulSize); 
	if(nRet == 0)
	{
		nRet = Write((char *)&byteAddress, &ulSize);
		if(nRet == 0)
		{
			nRet = Write((char *)&m_cWriteOp, &ulSize); 
			if(nRet == 0)
			{
				nRet = Write((char *)&byteData, &ulSize);
				if(dwDelay)
					Sleep(dwDelay);
			}
		}
	}
	return nRet;
}

int CCLSerIPX::CheckConnectedCamera()
{
	m_CamName.Empty();
	BYTE camID[4];
	int nRet;
	FillMemory(camID,sizeof(camID),0xFF);

	for (int i = 0; i<4;i++)
	{
		nRet = ReadReg(0xb0|i,camID+i);
		if(nRet)
		{
			return nRet;
		}
	}

			nRet= ReadReg(CAMERA_CONTROL1,&m_CamCntl1);
			if(nRet < 0)
				AfxMessageBox("Can't read register of Camera Control 1 ");

			nRet = ReadReg(CAMERA_CONTROL2,&m_CamCntl2);
			if(nRet < 0)
				AfxMessageBox("Can't read register of Camera Control 2");

			nRet = ReadReg(EXTERNAL_TRIGGER,&m_ExtTrigger);
			if(nRet < 0)
				AfxMessageBox("Can't read register of External Trigger ");

			nRet = ReadReg(STROBE_POSITION_HIGH,&m_StrobePosHigh);
			if(nRet < 0)
				AfxMessageBox("Can't read register of Strobe Position Hight");

			nRet = ReadReg(VERT_WIN_TOP_LOW,&m_VWinTopLow);
			if(nRet < 0)
				AfxMessageBox("Can't read register of Vertical Window Top Lines Low");

			nRet = ReadReg(VERT_WIN_TOP_HIGH,&m_VWinTopHigh);
			if(nRet < 0)
				AfxMessageBox("Can't read register of Vertical Window Top Lines High");

			nRet = ReadReg(VERT_WIN_BOTTOM_LOW,&m_VWinBottomLow);
			if(nRet < 0)
				AfxMessageBox("Can't read register of Vertical Window Bottom Lines Low");

			nRet = ReadReg(VERT_WIN_BOTTOM_HIGH,&m_VWinBottomHigh);
			if(nRet < 0)
				AfxMessageBox("Can't read register of Vertical Window Bottom Lines High");

			nRet = ReadReg(SHUT_TIME_LOW,&m_ShutTimeLow);
			if(nRet < 0)
				AfxMessageBox("Can't read register of Shutter Time Low");

			nRet = ReadReg(SHUT_TIME_HIGH,&m_ShutTimeHigh);
			if(nRet < 0)
				AfxMessageBox("Can't read register of Shutter Time High");

			nRet = ReadReg(LONG_INTEG_TIME_LOW,&m_LongIntTimeLow);
			if(nRet < 0)
				AfxMessageBox("Can't read register of Long Integration Time Low");

			nRet = ReadReg(LONG_INTEG_TIME_HIGH,&m_LongIntTimeHigh);
			if(nRet < 0)
				AfxMessageBox("Can't read register of Long Integration Time High");

			nRet = ReadReg(STROBE_POSITION_LOW,&m_StrobePosLow);
			if(nRet < 0)
				AfxMessageBox("Can't read register of Strobe Position Hight");

			nRet = ReadReg(HORIZ_WIN_SIZE_HIGH,&m_HWinSizeHigh);
			if(nRet < 0)
				AfxMessageBox("Can't read register of Horizontal Window Size High");
																				 
			nRet = ReadReg(HORIZ_WIN_SIZE_LEFT_LOW,&m_HWinSizeLeftLow);
			if(nRet < 0)
				AfxMessageBox("Can't read register of Horizontal Window Size Left Low");

			nRet = ReadReg(HORIZ_WIN_SIZE_RIGHT_LOW,&m_HWinSizeRightLow);
			if(nRet < 0)
				AfxMessageBox("Can't read register of Horizontal Window Size Right Low");

			m_CamCntl1 = m_CamCntl2 = 0;
			
			m_CamCntl1 |= SHUTTER_ENABLE | VERT_BINNING_ENABLE ;
//			m_CamCntl1 &= (~DUAL_OUTPUT_ENABLE) ;
				
			nRet = WriteReg(CAMERA_CONTROL1,m_CamCntl1,10); 
			if(nRet < 0)
			AfxMessageBox("Can't write to Camera Control 1");

			m_CamCntl2 |= HORIZ_BINNING_ENABLE ;//| EXT_TRIGGER_ENABLE;
			WriteReg(CAMERA_CONTROL2,m_CamCntl2,10 );//| HORIZ_BINNING_ENABLE))
			 if(nRet < 0)
				AfxMessageBox("Can't write to Camera Control 2");

//			m_CamCntl1 &= (~DUAL_OUTPUT_ENABLE);
//			if(!WriteRegister(CAMERA_CONTROL1,(INT8)m_CamCntl1))// & (~DUAL_OUTPUT_ENABLE)))
//				AfxMessageBox("Can't disable dual output");


	
	m_CamID = MAKELONG(MAKEWORD(camID[3],camID[2]),MAKEWORD(camID[1],camID[0]));
	switch(m_CamID)
	{
	case 0x00200001:
		m_CamName = _T("MDC1004");
		break;
	case 0x00200002:
		m_CamName = _T("MDC1004C");
		break;
	case 0x00230001:
		m_CamName = _T("MDC1600");
		break;
	case 0x00230002:
		m_CamName = _T("MDC1600C");
		break;
	case 0x00210001:
		m_CamName = _T("MDC1920");
		break;
	case 0x00210002:
		m_CamName = _T("MDC1920C");
		break;
	case 0x00220001:
		m_CamName = _T("MDC2048");
		break;
	case 0x00220002:
		m_CamName = _T("MDC2048C");
		break;
	case 0x00250001:
		m_CamName = _T("MDC4000");
		break;
	case 0x00250002:
		m_CamName = _T("MDC4000C");
		break;
	case 0x00400001:
		m_CamName = _T("IPX1M48");
		break;
	case 0x00400002:
		m_CamName = _T("IPX1M48C");
		break;
	case 0x00430001:
		m_CamName = _T("IPX2M30");
		break;
	case 0x00430002:
		m_CamName = _T("IPX2M30C");
		break;
	case 0x00410001:
		m_CamName = _T("IPX2M30H");
		break;
	case 0x00410002:
		m_CamName = _T("IPX2M30HC");
		break;
	case 0x00420001:
		m_CamName = _T("IPX4M15");
		break;
	case 0x00420002:
		m_CamName = _T("IPX4M15C");
		break;
	case 0x00440001:
		m_CamName = _T("IPXVGA210");
		break;
	case 0x00440002:
		m_CamName = _T("IPXVGA210C");
		break;
	case 0x00440003:
		m_CamName = _T("IPXVGA120");
		break;
	case 0x00440004:
		m_CamName = _T("IPXVGA120C");
		break;
	case 0x00440005:
		m_CamName = _T("IPXVGA90");
		break;
	case 0x00440006:
		m_CamName = _T("IPXVGA90C");
		break;
	case 0x00450001:
		m_CamName = _T("IPX11M5");
		break;
	case 0x00450002:
		m_CamName = _T("IPX11M5C");
		break;
	case 0x00220101:
		m_CamName = _T("Kinetta MDC2048");
		break;
	case 0x00220102:
		m_CamName = _T("Kinetta MDC2048C");
		break;
	case 0xFFFFFFFF:
		m_CamName.Empty();
		break;
	default:
		m_CamName = _T("Unknown");
	}
	return m_CamID;
}
int CCLSerIPX::SetShutterEnable(bool bEnable)
{
	int ret = -1;
	BYTE byte;
	ret = ReadReg(0x00,&byte);
	if(bEnable)
	{
		byte |= (1<<5);
	}
	else
	{
		byte &=~(1<<5);
	}
	ret = WriteReg(0x00,byte,10);
	return ret;
}
int CCLSerIPX::GetShutterEnable(bool& bEnable)
{
	int ret = -1;
	BYTE byte;
	ret = ReadReg(0x00,&byte);
	bEnable = ((byte & (1<<5)) != 0)? true : false ;
	return ret;
}
int CCLSerIPX::SetShutter(short nShut)
{
	int ret = -1;
	ret = WriteReg(0x08,LOBYTE(nShut),10);
	if(ret >= 0)
		ret = WriteReg(0x09,HIBYTE(nShut),10);
	return ret;
}
int CCLSerIPX::GetShutter(short& nShut)
{
	int ret = -1;
	BYTE loShut,hiShut;
	ret = ReadReg(0x08,&loShut);
	if(ret >= 0)
		ret = ReadReg(0x09,&hiShut);
	if(ret >= 0)
	{
		nShut = MAKEWORD(loShut,hiShut);
	}
	return ret;
}
int CCLSerIPX::SetGain1(float fGain)
{
	short nGain = (short)((fGain-6.)/(40.-6.)*1023);
	int ret = -1;
	ret = WriteReg(0x16,LOBYTE(nGain),10);
	if(ret >= 0)
		ret = WriteReg(0x17,HIBYTE(nGain),10);
	return ret;
}
int CCLSerIPX::GetGain1(float& fGain)
{
	short nGain ;
	int ret = -1;
	BYTE loVal,hiVal;
	ret = ReadReg(0x16,&loVal);
	if(ret >= 0)
		ret = ReadReg(0x17,&hiVal);
	if(ret >= 0)
	{
		nGain = MAKEWORD(loVal,hiVal);
		fGain = (float)(nGain*(40.-6.)/1023.+6.);
	}
	return ret;
}
