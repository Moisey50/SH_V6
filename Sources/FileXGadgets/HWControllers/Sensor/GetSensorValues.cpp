// GetSensorValues.cpp : Read data from PCSensors temperature and/or humidity sensors 
// Based on Stuart Allman project - https://www.edn.com/using-the-hid-class-eases-the-job-of-writing-usb-device-drivers/
#include "stdafx.h"
#include <Windows.h> 
#include "setupapi.h"	
#include <iostream>
#include <stdio.h>
#include <conio.h>

/*A C native code we get from the OS*/
extern "C"
{
#include "hidsdi.h"
}

#pragma comment(lib, "hid.lib")
#pragma comment(lib, "Setupapi.lib")

#include "SensTypes.h"

double g_Temps[2];
double g_Calib[2] = { 1.0, 0.0 };
double g_Temps2[2];

static const BYTE CMD_TEMP_READ[] = { 0 , 0x01, 0x80, 0x33, 0x01, 0x00, 0x00, 0x00, 0x00 };
static const BYTE CMD_DATA_WRITE[] = { 0x00, 0x01, 0x81, 0x55, 0x01, 0x00, 0x00, 0x00, 0x00 };
static const BYTE CMD_GET_CALIBRATION[] = { 0 , 0x01, 0x82, 0x77, 0x01, 0x00, 0x00, 0x00, 0x00 };
static const BYTE CMD_GET_VERSION[] = { 0 , 0x01, 0x86, 0xff, 0x01, 0x00, 0x00, 0x00, 0x00 };
static const BYTE CMD_FLASH_ERASE[] = { 0x00, 0x01, 0x85, 0xdd, 0x01, 0x01, 0x00, 0x00, 0x00 };
static const BYTE CMD_FLASH_SAVE[] = { 0x00, 0x01, 0x85, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00 };

bool RequestAndReceive(HANDLE hPipe, BYTE * pCommand, DWORD dwReqestLen,
  DWORD& dwNRead, UCHAR * pAnswer, DWORD AnswerLen)
{
  BOOL RESULT = WriteFile(hPipe, pCommand, dwReqestLen, &dwNRead, NULL);
  if (RESULT)
  {
    Sleep(20);
    RESULT = ReadFile(hPipe, pAnswer, AnswerLen, &dwNRead, NULL);
  }
  return RESULT;
}

bool GetTypeAndVersion(HANDLE hPipe, UCHAR * pAnswer,
  DWORD dwAnswerBufLen, DWORD& dwNRead)
{
  memset(pAnswer, 0, dwAnswerBufLen);

  UCHAR Name[32];
  UCHAR * pDest = pAnswer;
  dwNRead = 0;
  do
  {
    DWORD dwNRead1 = 0;
    bool RESULT = RequestAndReceive(hPipe, (BYTE*)CMD_GET_VERSION, sizeof(CMD_GET_VERSION), dwNRead1,
      (UCHAR*)Name, sizeof(Name));
    if (!RESULT)
    {
      if (!dwNRead)
      {
        TRACE("\nCan't get version 1");
        return false;
      }
      break;
    }
    UCHAR * pSrc = &Name[(Name[0] == 0)];
    if (dwNRead) // some data was read previously
    {
      if (memcmp(pSrc, pAnswer, 6) == 0)
        break; // the same data was read before, stop reading
    }
    while ((*pSrc != 0) && (pSrc - Name < (int)dwNRead1) && ((pDest - pAnswer) < (int)dwAnswerBufLen))
      *(pDest++) = *(pSrc++);
    dwNRead += (DWORD)(pDest - pAnswer);
    if ((dwNRead == dwAnswerBufLen) || (pAnswer[dwNRead - 1] == ' '))
      break;
  } while (1);

  while (pAnswer[dwNRead - 1] == ' ')
    dwNRead--;
  if (dwNRead < dwAnswerBufLen)
    pAnswer[dwNRead] = 0;
  return true;
}

size_t EnumerateSensors( temper_type_t * pIds , Sensors& ForFilling )
{
  static GUID winGUID;                                /*Windows Globally Unique ID*/
  SP_DEVICE_INTERFACE_DATA deviceInfoData;            /*Holding information about the array of HID devices*/
  DWORD RequiredSize;							        /* size of device info data structure */
  PSP_DEVICE_INTERFACE_DETAIL_DATA deviceDetailData = NULL;  /*Holding the data within each device (PATH AND SIZE OF THE STRUCTURE)*/
  DWORD structureDataSize;                            /*The size of the structure*/
  bool firstInit = TRUE; /*A variable to indicate if it is the first iteration in order to set deviceDetailData*/
  HIDD_ATTRIBUTES Device_attr;    /*HID device attributes up on acquiring the right Pid and Vid*/
  BYTE inputBuffer[9] = {};/*Input buffer to hold the information from the PIPE*/

  PHIDP_PREPARSED_DATA hidParsedData; /* Pointer to preparsed data retrived from the device*/
  HIDP_CAPS capabilities; /*Device limitations and capabilities*/

  HidD_GetHidGuid(&winGUID); /*Obtain Windows GUID for HID Devices*/

  HDEVINFO hidDevInfo = SetupDiGetClassDevs(  /*Receiving an array of HID device's structure with the information about the devices */
    &winGUID, NULL, NULL, DIGCF_PRESENT | DIGCF_INTERFACEDEVICE /*Flags controlling what is included in the device information set built*/
  );
  deviceInfoData.cbSize = sizeof(deviceInfoData); /*Setting the size of THE ARRAY of structure that holds the device info data*/
  bool bSuccess = false;
  unsigned int index = 0;                             /*index to iterate over the devices array */
  size_t uInitialSize = ForFilling.size();
  BOOL RESULT = FALSE;
  vector<HIDD_ATTRIBUTES> AllUSBAttributes;
  vector<string> AllUSBPaths;

  do /*Looping over the HID Structure array and trying to open a pipe with the device */
  {
    RESULT = SetupDiEnumDeviceInterfaces( /*Acquiring handle for a specific HID device (indexed)*/
      hidDevInfo, 0, &winGUID, index, &deviceInfoData);

    if (!RESULT) // FINISHED ITERATING -> The Device was not found
      break;

    if (firstInit) /*At this point we want to evaluate and set the size of DEVICE_INTERFACE_DETAIL_DATA structure */
    {
      firstInit = FALSE;
      SetupDiGetDeviceInterfaceDetail(
        hidDevInfo, &deviceInfoData, NULL, 0, &structureDataSize,  NULL );
          // This is a pointer to the HID Device Information Structure //
      deviceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(structureDataSize); /*Allocating memory for the HidDeviceInfo structure*/
      deviceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
    }

    /*
        Calling the SetupDiGetDeviceInterfaceDetail function with the correct size parameter (*why do we need to set it up???)
        The function return the data from the INDEXED array member this will give us access to the attributes of the HID Devices
    */
    SetupDiGetDeviceInterfaceDetail(hidDevInfo, &deviceInfoData,
      deviceDetailData, structureDataSize, &RequiredSize, NULL);
    string NewPath(deviceDetailData->DevicePath);
    AllUSBPaths.push_back(NewPath);
    HANDLE hPipe = CreateFile( /*A handle to the pipe connected to the device.*/
      deviceDetailData->DevicePath, (GENERIC_READ | GENERIC_WRITE),
      (FILE_SHARE_READ | FILE_SHARE_WRITE), NULL, OPEN_EXISTING, 0, NULL);
    if (hPipe != INVALID_HANDLE_VALUE)
    {
      Device_attr.Size = sizeof(Device_attr);
      if (HidD_GetAttributes(hPipe, &Device_attr)) /*HID device attributes (PID AND VID)*/
      {
        AllUSBAttributes.push_back(Device_attr);
        temper_type_t * pKnown = pIds;
        while (pKnown->vendor_id)
        {
          if (Device_attr.VendorID == pKnown->vendor_id
            && Device_attr.ProductID == pKnown->product_id)
          {
            RESULT = HidD_GetPreparsedData(hPipe, &hidParsedData);
            if (RESULT)
            {
              RESULT = HidP_GetCaps(hidParsedData, &capabilities);
              if (RESULT)
              {
              }
              RESULT = HidD_FreePreparsedData(hidParsedData);
              char TypeAndVersion[50];
              DWORD dwTypeLen = 0;
              if (GetTypeAndVersion(hPipe, (UCHAR*)TypeAndVersion, sizeof(TypeAndVersion), dwTypeLen))
              {
                temper_type_t NewDevice;
                NewDevice.vendor_id = Device_attr.VendorID;
                NewDevice.product_id = Device_attr.ProductID;
                strcpy_s(NewDevice.product_name, sizeof(NewDevice.product_name), TypeAndVersion);
                ForFilling.push_back(NewDevice);
                break;
              }
            }
          }
          pKnown++;
        }
      }
      CloseHandle(hPipe);
    }
    index++; // Iterate to the next HID device
  } while (RESULT); /*While we did not reach the end of the structure array*/
  /*Free Resources*/
  if (deviceDetailData != NULL)
    free(deviceDetailData);
  SetupDiDestroyDeviceInfoList(hidDevInfo);
  return ForFilling.size() - uInitialSize ;
}

void decode_answer_fm75(unsigned char *answer, double *tempd, double *calibration) {
  int iVal;

  // temp C internal
//  iVal = ((signed char)answer[2] << 8) + (answer[3] & 0xFF);
  iVal = ((signed char)answer[3] << 8) + (answer[4] & 0xFF);
  tempd[0] = iVal * (125.0 / 32000.0);
  tempd[0] = tempd[0] * calibration[0] + calibration[1];

  // temp C external
  iVal = ((signed char)answer[5] << 8) + (answer[6] & 0xFF);
  tempd[1] = iVal * (125.0 / 32000.0);
  tempd[1] = tempd[1] * calibration[0] + calibration[1];
};

void decode_answer_sht1x(unsigned char *answer, double *tempd, double *calibration) {
  int iVal;

  // temp C
//  iVal = ((signed char)answer[2] << 8) + (answer[3] & 0xFF);
  iVal = ((signed char)answer[3] << 8) + (answer[4] & 0xFF);
  tempd[0] = -39.7 + 0.01 * iVal;
  tempd[0] = tempd[0] * calibration[0] + calibration[1];

  // relative humidity
  iVal = ((signed char)answer[5] << 8) + (answer[6] & 0xFF);
  tempd[1] = -2.0468 + 0.0367 * iVal - 1.5955e-6 * iVal * iVal;
  tempd[1] = (tempd[0] - 25) * (0.01 + 0.00008 * iVal) + tempd[1];
  if (tempd[1] < 0) tempd[1] = 0;
  if (tempd[1] > 99) tempd[1] = 100;
};

void decode_answer_TemperHum413d_2107(UCHAR *pData, UCHAR * pCorr, double& dTemp, double& dHum)
{
  double dTCorr = (double)((int)((char)(pCorr[3]))) / 10.;
  double dHumCorr = (double)((int)((char)(pCorr[4]))) / 10.;
  USHORT usVal1 = (((USHORT)pData[3]) << 8) + pData[4];
  USHORT usVal2 = (((USHORT)pData[5]) << 8) + pData[6];
  dTemp = (double)((short int)(usVal1)) / 100.;
  dHum = (double)((short int)(usVal2)) / 100.;
}

bool GetSensorValues(int VENDOR_ID, int PRODUCT_ID, 
  LPTSTR pBuffer, int iBufferSize , int iViewDebug )
{
  static GUID winGUID;                                /*Windows Globally Unique ID*/
  SP_DEVICE_INTERFACE_DATA deviceInfoData;            /*Holding information about the array of HID devices*/
  DWORD RequiredSize;							        /* size of device info data structure */
  unsigned int index = 0;                             /*index to iterate over the devices array */
  HDEVINFO hidDevInfo;                                /*Handle for each structure within the HID devices array*/
  PSP_DEVICE_INTERFACE_DETAIL_DATA deviceDetailData = NULL;  /*Holding the data within each device (PATH AND SIZE OF THE STRUCTURE)*/
  DWORD structureDataSize;                            /*The size of the structure*/
  bool RESULT = FALSE;   /*boolean to hold wether the desired device is located*/
  bool firstInit = TRUE; /*A variable to indicate if it is the first iteration in order to set deviceDetailData*/
  HANDLE hPipe;     /*Handle for opening a communication pipe with the device*/
  HIDD_ATTRIBUTES Device_attr;    /*HID device attributes up on acquiring the right Pid and Vid*/
  /*The command for a temperature report for the 0xC45 / 0x7401 device*/
  DWORD numOfBytesRead;                               /*Number of bytes read from the PIPE*/

  PHIDP_PREPARSED_DATA hidParsedData; /* Pointer to preparsed data retrieved from the device*/
  HIDP_CAPS capabilities; /*Device limitations and capabilities*/

  HidD_GetHidGuid(&winGUID); /*Obtain Windows GUID for HID Devices*/

  hidDevInfo = SetupDiGetClassDevs(        /*Receiving an array of HID device's structure with the infromation about the devices */
    &winGUID,
    NULL,
    NULL,
    DIGCF_PRESENT | DIGCF_INTERFACEDEVICE /*Flags controlling what is included in the device information set built*/
  );
  deviceInfoData.cbSize = sizeof(deviceInfoData); /*Setting the size of THE ARRAY of structure that holds the device info data*/
  bool bSuccess = false;
  vector<HIDD_ATTRIBUTES> AllUSBAttributes;

  do /*Looping over the HID Structure array and trying to open a pipe with the device */
  {
    RESULT = SetupDiEnumDeviceInterfaces( /*Aсquiring hanвle for a specifc HID device (indexed)*/
      hidDevInfo, 0, &winGUID, index, &deviceInfoData);

    if (!RESULT) // FINISHED ITERATING -> The Device was not found
    {
      /* free the memory allocated for DetailData -
          Memory is allocated in the next if scope*/
      if (deviceDetailData != NULL)
        free(deviceDetailData);

      /* free HID device info list resources */
      SetupDiDestroyDeviceInfoList(hidDevInfo);

      return FALSE;
    }

    if (firstInit) /*At this point we want to evaluate and set the size of DEVICE_INTERFACE_DETAIL_DATA structure */
    {
      firstInit = FALSE;
      SetupDiGetDeviceInterfaceDetail(
        hidDevInfo,
        &deviceInfoData,
        NULL,
        0,
        &structureDataSize,
        NULL
      );

      // This is a pointer to the HID Device Information Structure //
      deviceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(structureDataSize); /*Allocating memory for the HidDeviceInfo structure*/

      deviceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
    }

    /*
        Calling the SetupDiGetDeviceInterfaceDetail function with the correct size parameter (*why do we need to set it up???)
        The function return the data from the INDEXED array member this will give us access to the attributes of the HID Devices
    */
    SetupDiGetDeviceInterfaceDetail(hidDevInfo, &deviceInfoData,
      deviceDetailData, structureDataSize, &RequiredSize, NULL);
    hPipe = CreateFile( /*A handle to the pipe connected to the device.*/
      deviceDetailData->DevicePath, (GENERIC_READ | GENERIC_WRITE),
      (FILE_SHARE_READ | FILE_SHARE_WRITE), NULL, OPEN_EXISTING, 0, NULL);
    if (hPipe != INVALID_HANDLE_VALUE)
    {
      Device_attr.Size = sizeof(Device_attr);

      HidD_GetAttributes(hPipe, &Device_attr); /*HID device attributes (PID AND VID)*/
      AllUSBAttributes.push_back(Device_attr);

      if (Device_attr.ProductID == PRODUCT_ID && Device_attr.VendorID == VENDOR_ID)
      {
        //         std::cout << "Product ID: " << Device_attr.ProductID << "\n Vendor ID: " << Device_attr.VendorID << std::endl;
                /*Getting preparsed data to understand what are the capabilities of the device in hand*/
                /*NOTE - after this routine RESULT == TRUE we need to stop the iteration and change it to
                false at the end*/

                //SetupDiGetDeviceRegistryProperty( hidDevInfo )

        RESULT = HidD_GetPreparsedData(hPipe, &hidParsedData);
        if (!RESULT)
        {
          TRACE("Error retrieving preparsed data from device") ;
          return false;
        }

        RESULT = HidP_GetCaps(hidParsedData, &capabilities);
        if (!RESULT)
        {
          RESULT = HidD_FreePreparsedData(hidParsedData);
          TRACE("Error retrieving capabilities from device");
          return false;
        }

        RESULT = HidD_FreePreparsedData(hidParsedData);
        if (!RESULT)
        {
          TRACE("\nError free resources (PreParsedData)");
          return false;
        }

        UCHAR TypeAndVersion[50];
        DWORD dwTypeLen = 0;
        GetTypeAndVersion(hPipe, TypeAndVersion, sizeof(TypeAndVersion), dwTypeLen);
        /*Set the Calibration needed for the output -
            Assuming this is how we normalize the output and correct the measurements*/

        BYTE CorrInBuf[40]; // Buff for correction 
        RESULT = RequestAndReceive(hPipe, (BYTE*)CMD_GET_CALIBRATION, 
          sizeof(CMD_GET_CALIBRATION), numOfBytesRead,
          (UCHAR*)CorrInBuf, sizeof(CorrInBuf));

        if (!RESULT)
        {
          throw "Failed to read Calibration settings from the device";
        }
        /*Probably another variable to Normalize the output - Seems like we are returning the temperature
            without considering the humidity (CHECK IF WE NEED TO INCLUDE HUMIDITY)*/
        double dCorrections[2] = { 1., 0. };
        int iCorr1 = CorrInBuf[3] * 0x100 + CorrInBuf[4];
        int iCorr2 = CorrInBuf[5] * 0x100 + CorrInBuf[6];
        if (CorrInBuf[1] == 130)
        {
          if (CorrInBuf[2] < 128)
            dCorrections[1] = (double)(CorrInBuf[2] * 0x100 + CorrInBuf[3]) / 100.0;
          else
            dCorrections[1] = (double)-(0x10000 - CorrInBuf[2] * 0x100 - CorrInBuf[3]) / 100.0;
        }
        BYTE DataIn[40]; // Buff for data input
        RESULT = RequestAndReceive(hPipe, (BYTE*)CMD_TEMP_READ, sizeof(CMD_TEMP_READ), numOfBytesRead,
          (UCHAR*)DataIn, sizeof(DataIn));
        if (!RESULT)
        {
          throw "Failed to read Temperature from the device";
        }
        double dFirstVal ;
        double dSecondVal ;
        decode_answer_TemperHum413d_2107(DataIn, CorrInBuf, dFirstVal, dSecondVal);
        USHORT usVal1 = (((USHORT)DataIn[3]) << 8) + DataIn[4];
        USHORT usVal2 = (((USHORT)DataIn[5]) << 8) + DataIn[6];

        double dVals_fm75[2] = { 0., 0. };
        decode_answer_fm75(DataIn, dVals_fm75, dCorrections);
        double dVals_sht1x[2] = { 0., 0. };
        decode_answer_sht1x(DataIn, dVals_sht1x, dCorrections);
        char tracebuf[300];
        sprintf_s(tracebuf, "\n0x%04x: D=%d %d %d %d  %d %d %d %d %d  iVals=%d,%d"
          "C=%d %d %d %d  %d %d %d %d %d  iCorr=%d,%d\n"
          " FM(%.2f,%.2f) SH(%.2f,%.2f)\n", Device_attr.ProductID,
          DataIn[0],
          DataIn[1], DataIn[2], DataIn[3], DataIn[4],
          DataIn[5], DataIn[6], DataIn[7], DataIn[8],
          usVal1 , usVal2 , 
          CorrInBuf[0],
          CorrInBuf[1], CorrInBuf[2], CorrInBuf[3], CorrInBuf[4],
          CorrInBuf[5], CorrInBuf[6], CorrInBuf[7], CorrInBuf[8],
          iCorr1, iCorr2,
          dVals_fm75[0], dVals_fm75[1], dVals_sht1x[0], dVals_sht1x[1]);
//         TRACE("0x%04x: %d %d %d %d  %d %d %d %d %d  FM(%.2f,%.2f) SH(%.2f,%.2f)\n", Device_attr.ProductID ,
//           DataIn[0],
//           DataIn[1], DataIn[2], DataIn[3], DataIn[4],
//           DataIn[5], DataIn[6], DataIn[7], DataIn[8],
//           dVals_fm75[0],dVals_fm75[1],dVals_sht1x[0],dVals_sht1x[1]);

        if (pBuffer)
        {
          if ( iViewDebug )
          {
            sprintf_s(pBuffer, iBufferSize, "T=%.2f H=%.2f %s", dFirstVal, dSecondVal , tracebuf );

          }
          else
            sprintf_s(pBuffer, iBufferSize, "T=%.2f H=%.2f", dFirstVal, dSecondVal);
        }
        bSuccess = true;
        break;
      }
      CloseHandle(hPipe);
    }
    index++; // Iterate to the next HID device
  } while (RESULT); /*While we did not reach the end of the structure array*/
  /*Free Resources*/
  if (deviceDetailData != NULL)
    free(deviceDetailData);

  SetupDiDestroyDeviceInfoList(hidDevInfo);

  if (hPipe != INVALID_HANDLE_VALUE)
    CloseHandle(hPipe);
  return bSuccess ;
}
