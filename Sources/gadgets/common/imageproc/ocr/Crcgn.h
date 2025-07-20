//  $File : Crcgn.h - OCR's functions primitives
//  (C) Copyright The File X Ltd 2002. 
//
//
//
//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 Firts release version, all followed changes must be listed below  (Andrey Chernushich)


#ifndef _CRCGN_INC
#define _CRCGN_INC

#include <imageproc\clusters\clusters.h>

#if  (defined BERKUT_TUNER_APPL)
  #define SHOW_RECG_STATUS
  #define OCR_ALGORITHM_TIMING
#endif

#ifdef OCR_LEARNER_APPL
  #define OCR_ADD_DEBUG_MODULE
#endif

#define OCR_MAXWIDTH (2*CHAR_MAX_WIDTH)
#define OCR_MAXHEIGHT (2*CHAR_MAX_HEIGHT)
#define OCR_MINWIDTH (CHAR_MIN_WIDTH)
#define OCR_MINHEIGHT (CHAR_MIN_HEIGHT)


// charsets sets
#define ALPHA_ANY     0
#define ALPHA_DIGIT   1
#define ALPHA_SMALL   2
#define ALPHA_CYR     3
#define ALPHA_CHARANY 4
typedef struct _tagPattern
{
	unsigned char cogs[26];
	unsigned char ch;
}Pattern,*pPattern;

BOOL        OCR_Init(const char* Path);      // load defaul named library from the path
#ifdef UNICODE
    BOOL OCR_Init(HMODULE hModule, LPCWSTR lpName, LPCWSTR lpType );
#else
    BOOL OCR_Init(HMODULE hModule, LPCSTR lpName, LPCSTR lpType );
#endif

BOOL        OCR_Load(const char* PathName);  // load any name library from the path file name
void        OCR_CreateNewLibrary();
void        OCR_Free();                      // close module
void        OCR_Save();                      // save updated library
BOOL        OCR_IsReady();                   // is library loaded?
const char* OCR_GetStatus(void);             // get current status
void        OCR_RecognizeChar(pCluster cluster, int alpha, Pattern& pat); // recognize a char
BOOL        OCR_AddCharToLib(Pattern& pat);

#ifdef OCR_ALGORITHM_TIMING
double      OCR_GetTimeSpent();
#endif

#ifdef OCR_ADD_DEBUG_MODULE
void OCR_DebugInfoRst(void);
void OCR_DebugInfoWrite(CStdioFile *lf);
void OCR_DebugAdd(unsigned char a, Cluster& cluster,Pattern& pat);
#endif

#endif _CRCGN_INC
