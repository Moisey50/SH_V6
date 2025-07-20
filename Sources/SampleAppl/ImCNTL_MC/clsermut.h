


#ifndef __CLSERMUT_H_
#define __CLSERMUT_H_

#define CL_MANU_NAME						"MuTech Corporation"
#define CL_MANU_NAME_LENGTH					19
#define CL_PORT_ID_NAME						"MV-2500(0)"
#define CL_PORT_ID_LENGTH					11

#define CL_ERR_NO_ERR						0
#define CL_ERR_BUFFER_TOO_SMALL				-10001
#define CL_ERR_MANU_DOES_NOT_EXIST			-10002
#define CL_ERR_PORT_IN_USE					-10003
#define CL_ERR_TIMEOUT						-10004
#define CL_ERR_INVALID_INDEX				-10005
#define CL_ERR_INVALID_REFERENCE			-10006
#define CL_ERR_ERROR_NOT_FOUND				-10007
#define CL_ERR_BAUD_RATE_NOT_SUPPORTED		-10008
#define CL_ERR_FUNCTION_NOT_FOUND			-10009

#define CL_ERR_NO_ERR_txt					"Function retuned successfully."
#define CL_ERR_BUFFER_TOO_SMALL_txt			"User buffer not large enough to hold data."
#define CL_ERR_MANU_DOES_NOT_EXIST_txt		"The requested manufacturer's DLL does not exist on your system."
#define CL_ERR_PORT_IN_USE_txt				"Port is valid but cannot be opened because it is in use."
#define CL_ERR_TIMEOUT_txt					"Operation not completed within specified timeout period."
#define CL_ERR_INVALID_INDEX_txt			"Not a valid index."
#define CL_ERR_INVALID_REFERENCE_txt		"The serial reference is not valid."
#define CL_ERR_ERROR_NOT_FOUND_txt			"Could not find the error desciption for this error code."
#define CL_ERR_BAUD_RATE_NOT_SUPPORTED_txt	"Requested baud rate not supported by this interface."
#define CL_ERR_FUNCTION_NOT_FOUND_txt		"Function does not exist in the manufacturer's library."

#define CL_DLL_VERSION_NO_VERSION			0x01
#define CL_DLL_VERSION_1_0					0x02
#define CL_DLL_VERSION_1_1					0x03
#define CL_BAUDRATE_9600					0x01
#define CL_BAUDRATE_19200					0x02
#define CL_BAUDRATE_38400					0x04
#define CL_BAUDRATE_57600					0x08
#define CL_BAUDRATE_115200					0x10
#define CL_BAUDRATE_230400					0x20
#define CL_BAUDRATE_460800					0x40
#define CL_BAUDRATE_921600					0x80

#if defined(CPLUSPLUS) || defined(__cplusplus)
extern "C" {
#endif

int clFlushPort(void *serialRef);
int clGetErrorText(int errorCode, char *errorText, unsigned long *errorTextSize);
int clGetManufacturerInfo(char *ManufacturerName, unsigned long *bufferSize, unsigned int *version);
int clGetNumBytesAvail(void *serialRef, unsigned long *numBytes);
int clGetNumSerialPorts(unsigned int *numSerialPorts);
int clGetSerialPortIdentifier(unsigned long serialIndex, char *portID, unsigned long *bufferSize);
int clGetSupportedBaudRates(void *serialRef, unsigned int *baudRates);
//void clSerialClose(void *serialRef);
//int clSerialInit(unsigned long serialIndex, void **serialRefPtr);
//int clSerialRead(void *serialRef, char *buffer, unsigned long *numBytes, unsigned long serialTimeout);
//int clSerialWrite(void *serialRef, char *buffer, unsigned long * bufferSize, unsigned long serialTimeout);
int clSetBaudRate(void *serialRef, unsigned int baudRate);
typedef int(*ptclSerInitProc)(unsigned long, void**);
typedef int(*ptclSerReadProc)(void*, char*, unsigned long*,	unsigned long);
typedef int(*ptclSerWriteProc)(void*, char*, unsigned long*, unsigned long);
typedef int(*ptclSerCloseProc)(void*);


#if defined(CPLUSPLUS) || defined(__cplusplus)
}
#endif

#endif
