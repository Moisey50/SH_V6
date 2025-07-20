// UserExampleGadget.h : Declaration of the UserExampleGadget class



#ifndef __INCLUDE__SensTypes_H__
#define __INCLUDE__SensTypes_H__

#pragma once
#include <vector>
class temper_type_t 
{
public:
  int vendor_id;
  int product_id;
  char product_name[256];
  int check_product_name; // if set, check product name in forward match
  int has_sensor; // number of temperature sensor
  int has_humid;  // flag for humidity sensor
  void(*decode_func)();

  temper_type_t() { memset(this, 0, sizeof(*this)); }
  temper_type_t(int VID, int PID, LPCTSTR Name,
    BOOL bCheckName, int iNSensors, int iHumidIndex, void * pDecodeFunc)
  {
    vendor_id = VID; product_id = PID;
    strcpy_s(product_name, Name);
    check_product_name = bCheckName;
    has_sensor = iNSensors;
    has_humid = iHumidIndex;
    decode_func = (void(*)())pDecodeFunc;
  }
} ;

typedef struct {
  void  *handle;
  temper_type_t *type;
} temper_device_t;

// temper_type_t tempers[*] = {
//   { 0x0c45, 0x7401, "TEMPer2", 1, 2, 0, decode_answer_fm75 }, // TEMPer2* eg. TEMPer2V1.3
//   { 0x0c45, 0x7401, "TEMPer1", 0, 1, 0, decode_answer_fm75 }, // other 0c45:7401 eg. TEMPerV1.4
//   { 0x0c45, 0x7402, "TEMPerHUM", 0, 1, 1, decode_answer_sht1x },
// };

typedef vector< temper_type_t> Sensors;
/* memo: TEMPer2 cannot be distinguished with VID:PID,
   thus product name (like TEMPer2V1.3) should be checked. */

   /* global variables */

#define MAX_DEV 8

#define INTERFACE1 0x00
#define INTERFACE2 0x01

size_t EnumerateSensors(temper_type_t * pIds, Sensors& ForFilling);
bool GetSensorValues(int VENDOR_ID, int PRODUCT_ID, LPTSTR pBuffer, int iBufferSize , int iViewDebug = 0 );

#endif	//__INCLUDE__SensTypes_H__

