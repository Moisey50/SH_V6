//  $File : OcrBase.h : header file OCRBase inteface class
//  (C) Copyright The File X Ltd 2002. 
//
//
//
//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 Firts release version, all followed changes must be listed below  (Andrey Chernushich)


#ifndef OCRBASE_INC
#define OCRBASE_INC

/////////////////////////////////////////////////////////////////////////////
// OcrBase Class

#include <vfw.h>
#include <imageproc\ocr\ocrmessages.h>

#define OCRB_HEADER_MARKER    ((WORD) ('C' << 8) | 'O')

typedef struct tagOCRBASEHEADER
{
	short  signature;		// must be "OC"
	DWORD  size_of;			// size of the structure
	DWORD  ocrbasesize;		// size in bytes
	DWORD  heapsize;		// size in bytes
	long   startdata;		// data offset from the begin of file
	long   recnum;			// alive records number
    char   version[5];      // current version
	char   reserved[5];
}OCRBASEHEADER,pOCRBASEHEADER;

typedef struct tagOCRBASEDATA //descriptor's element
{
	unsigned char   symbol; // ascii code of symbol
	short			angle;
	unsigned char	width,depth; // size of bitmap
	unsigned int    bmoff; // bitmap offset from begin of data
}OCRBASEDATA,pOCRBASEDATA;

typedef struct tagCharInfo
{
	unsigned char   symbol; // ascii code of symbol
	short			angle;
	unsigned char	width,depth; // size of bitmap
    LPBYTE          data;
}CharInfo,pCharInfo;

class COcrBase
{
protected:
	OCRBASEDATA*  m_OBD;
	OCRBASEHEADER m_OBH;
	FSIZE         m_FileSize;
    HANDLE        m_hMem;
	unsigned char *m_heap, *m_heaptail;
	CString       m_FName;
public:
    COcrBase(CWnd*Parent);
	COcrBase(CWnd*Parent, CString BaseName);
    ~COcrBase();
	BOOL Add(LPBYTE data, int width, int depth, char a, short angle);
	BOOL Get(DWORD& ItemNo, CharInfo *CI);
	BOOL CheckDupe(CharInfo *CI);
    BOOL CheckDupe(LPBYTE data, int width, int depth, char a, short angle);
	void Sort(void);
	BOOL Delete(DWORD ItemNo);
	BOOL ChangeChar(char a, DWORD ItemNo);
	BOOL OcrBaseValid(void) {return(m_OBH.signature==OCRB_HEADER_MARKER);};
	DWORD GetBaseSize(void) {return (nElem);};
	DWORD nElem; //number of chars, stored in the base 
    char  GetChar(DWORD ItemNo) { return(m_OBD[ItemNo].symbol); };
private:
	void _convertOLD();
};

#endif //OCRBASE_INC
