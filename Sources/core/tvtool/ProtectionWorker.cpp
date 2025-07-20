#include "stdafx.h"
#include "guardant\include\ProtectionWorker.h"
#include "guardant\include\grddongle.h"

#define GrdDC_LP       0x5e617fb9  // Licence public code
#define GrdDC_JUNK3    0xab674822 // Junk
#define GrdDC_JUNK4    0x12345622  // Junk
#define GrdDC_JUNK5    0xbca42912  // Junk
#define GrdDC_LR       0x7cf838f4  // Licence private read code

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

CProtectionWorker::CProtectionWorker() 
{
    SetThreadName("GRDWorker");
	iResult = 0;
}

int CProtectionWorker::GetResult()
{
	return iResult;
}

int CProtectionWorker::DoJob()
{
	//GUARDANT
	int ret = 0;
	char pExe[32];
	memset(pExe, 0, 32);
	DWORD ID;
	DWORD dwMode = GrdF_First;
	CGrdDongle grdDongle;
	grdDongle.Create(GrdDC_LP, GrdDC_LR, 0, 0);
	do
	{
		ret = grdDongle.Find(dwMode, &ID);
		dwMode = GrdF_Next;
		if(ret != 0)
		{
			grdDongle.Logout();
			iResult = ret;
			break;
		}
		ret = grdDongle.Login();
		if(ret != 0)
		{
			grdDongle.Logout();
			iResult = ret;
			break;
		}
		ret = grdDongle.PI_Read(0, 0, 32, pExe);
		if(ret != 0)
		{
			grdDongle.Logout();
			continue;
		}
		char    szAppPath[MAX_PATH] = "";
		CString strAppName;
		::GetModuleFileName((HINSTANCE)&__ImageBase, szAppPath, sizeof(szAppPath) - 1);
		strAppName = szAppPath;
		strAppName = strAppName.Right(strAppName.GetLength() - strAppName.ReverseFind('\\') - 1);
		for(int i = 0; i < min(32, strAppName.GetLength()); i++)
			ret += pExe[i] - strAppName[i];
		iResult = ret;
		if(abs(int(ret * 2700.5)) > 0)
			grdDongle.Logout();
		else 
			break;
	}
	while(true);

	return WR_CONTINUE;
}