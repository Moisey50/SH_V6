//  $File : OcrBase.cpp - implementation odf OCR Base
//  (C) Copyright The File X Ltd 2002. 
//
//
//
//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 Firts release version, all followed changes must be listed below  (Andrey Chernushich)


#include "stdafx.h"
#include <malloc.h>
#include "OCRBASE.H"
#include <files\futils.h>
#include <imageproc\clusters\clusters.h>
#include <messages.h>
#include <imageproc\ocr\ocrmessages.h>

const char base_version[5]="2.00";
// Locals 

COcrBase::COcrBase(CWnd*Parent)
{
	CFile obFile;
    CWaitCursor wc;
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    m_hMem=HeapCreate(0,  0x166BD7CL/si.dwPageSize,0);
    BOOL  oldVersion=FALSE;
	m_FName=GetStartDir()+"OcrBase.dat";
	m_OBD=NULL;
	m_heap=NULL; m_heaptail=NULL;
	obFile.Open(m_FName,CFile::modeRead|CFile::modeCreate|CFile::modeNoTruncate);
	m_FileSize=obFile.GetLength();
	if ((m_FileSize)&&(obFile.Read(&m_OBH, sizeof(m_OBH))==sizeof(m_OBH)))
	{
		if ((m_OBH.signature!=OCRB_HEADER_MARKER)||(m_OBH.ocrbasesize%sizeof(OCRBASEDATA)))
		{
			m_FileSize=0;
			AfxMessageBox(ERROR_WRONG_OCRBASE);
			memset(&m_OBH,0,sizeof(m_OBH));
			obFile.Close();
			return;
		}
		nElem=m_OBH.ocrbasesize/sizeof(OCRBASEDATA);
		if (nElem)
		{
            oldVersion=strcmp(base_version,m_OBH.version);
			m_OBD=(OCRBASEDATA*)malloc(m_OBH.ocrbasesize);
			//m_heap=(unsigned char*)malloc(m_OBH.heapsize);
            m_heap=(unsigned char*)HeapAlloc(m_hMem,0,m_OBH.heapsize);
			if ((obFile.Read(m_OBD,m_OBH.ocrbasesize)!=m_OBH.ocrbasesize)||
				(obFile.Read(m_heap,m_OBH.heapsize)!=m_OBH.heapsize))
			{
				free(m_OBD);
				m_OBD=NULL;
				//free(m_heap);
                HeapFree(m_hMem,HEAP_NO_SERIALIZE,m_heap);
				m_heap=NULL;	m_heaptail=NULL;
				m_FileSize=0;
				AfxMessageBox(ERROR_OCRBASE_CANTREAD);
				memset(&m_OBH,0,sizeof(m_OBH));
				obFile.Close();
				return;
			}
			else 
            {
                m_heaptail=m_heap+m_OBH.heapsize;
                if (oldVersion)
                {
                    strcpy(m_OBH.version,base_version);
                    _convertOLD();
                }
            }
		}
	}
	else
	{
		m_FileSize=0;
		if(AfxMessageBox(PROMPT_CREATENEW_OCRBASE ,MB_ICONQUESTION|MB_OKCANCEL)==IDOK)
		{
			memset(&m_OBH,0,sizeof(m_OBH));
			nElem=0;
			m_OBH.signature=OCRB_HEADER_MARKER;	// must be "OC"
			m_OBH.size_of=sizeof(OCRBASEHEADER);			// size of the structure
            strcpy(m_OBH.version,base_version);
		}
		else
		memset(&m_OBH,0,sizeof(m_OBH));
	}
	
	obFile.Close();   
}

COcrBase::COcrBase(CWnd* Parent, CString BaseName)
{
	CFile obFile;
    BOOL oldVersion=FALSE;
    CWaitCursor wc;

    SYSTEM_INFO si;
    GetSystemInfo(&si);
    m_hMem=HeapCreate(0,  (0x166BD7CL/si.dwPageSize+1)*si.dwPageSize, 0);
	m_FName=BaseName;
	m_OBD=NULL;
	m_heap=NULL; m_heaptail=NULL;
	obFile.Open(m_FName,CFile::modeRead|CFile::modeCreate|CFile::modeNoTruncate);
	m_FileSize=obFile.GetLength();
	if ((m_FileSize)&&(obFile.Read(&m_OBH, sizeof(m_OBH))==sizeof(m_OBH)))
	{
		if ((m_OBH.signature!=OCRB_HEADER_MARKER)||(m_OBH.ocrbasesize%sizeof(OCRBASEDATA)))
		{
			m_FileSize=0;
			AfxMessageBox(ERROR_WRONG_OCRBASE);
			memset(&m_OBH,0,sizeof(m_OBH));
			obFile.Close();
			return;
		}
		nElem=m_OBH.ocrbasesize/sizeof(OCRBASEDATA);
		if (nElem)
		{
            oldVersion=strcmp(base_version,m_OBH.version);
			m_OBD=(OCRBASEDATA*)malloc(m_OBH.ocrbasesize);
            //m_heap=(unsigned char*)malloc(m_OBH.heapsize);
			m_heap=(unsigned char*)HeapAlloc(m_hMem,0,m_OBH.heapsize);
			if ((obFile.Read(m_OBD,m_OBH.ocrbasesize)!=m_OBH.ocrbasesize)||
				(obFile.Read(m_heap,m_OBH.heapsize)!=m_OBH.heapsize))
			{
				free(m_OBD);
				m_OBD=NULL;
				//free(m_heap);
                HeapFree(m_hMem,HEAP_NO_SERIALIZE,m_heap);
				m_heap=NULL;	m_heaptail=NULL;
				m_FileSize=0;
				AfxMessageBox(ERROR_OCRBASE_CANTREAD);
				memset(&m_OBH,0,sizeof(m_OBH));
				obFile.Close();
				return;
			}
			else
            {
                m_heaptail=m_heap+m_OBH.heapsize;
                if (oldVersion)
                {
                    _convertOLD();
                    strcpy(m_OBH.version,base_version);
                }
            }
		}
	}
	else
	{
		m_FileSize=0;
		if(AfxMessageBox(PROMPT_CREATENEW_OCRBASE, MB_ICONQUESTION|MB_OKCANCEL)==IDOK)
		{
			memset(&m_OBH,0,sizeof(m_OBH));
			nElem=0;
			m_OBH.signature=OCRB_HEADER_MARKER;	// must be "OC"
			m_OBH.size_of=sizeof(OCRBASEHEADER);			// size of the structure
            strcpy(m_OBH.version,base_version);
		}
		else
		memset(&m_OBH,0,sizeof(m_OBH));
	}
	
	obFile.Close();   
}


COcrBase::~COcrBase()
{
	if (m_OBH.signature==0) return;
	CFile obFile;
	obFile.Open(m_FName,CFile::modeWrite|CFile::modeCreate);
	obFile.Write(&m_OBH, sizeof(m_OBH));
	if (m_OBD)
	{	
		obFile.Write(m_OBD,m_OBH.ocrbasesize);
		free (m_OBD);
		if (m_heap)
		{
			obFile.Write(m_heap,m_OBH.heapsize);
			//free(m_heap);
            HeapFree(m_hMem,HEAP_NO_SERIALIZE,m_heap);
		}
	}
    HeapDestroy(m_hMem);
}

BOOL COcrBase::Add(LPBYTE data, int width, int depth, char a, short angle)
{
    if (CheckDupe(data,width,depth,a,angle)) return(FALSE);
	if (nElem==0) 
	{
		m_OBD=(OCRBASEDATA*)malloc(sizeof(OCRBASEDATA));
		m_OBH.ocrbasesize+=sizeof(OCRBASEDATA);
		m_OBD[nElem].width=width;
		m_OBD[nElem].depth=depth;
		m_OBD[nElem].symbol=a;
		m_OBD[nElem].bmoff=0;
		m_OBD[nElem].angle=angle;
		DWORD bmSize=width*depth;
        m_heap=(unsigned char*)HeapAlloc(m_hMem,0,bmSize);
		//m_heap=(unsigned char*)malloc(bmSize);
		memcpy(m_heap, data,bmSize);
		m_OBH.heapsize+=bmSize;
		m_heaptail=m_heap+bmSize;
		nElem++;
	}
	else
	{
		m_OBD=(OCRBASEDATA*)realloc(m_OBD, (nElem+1)*sizeof(OCRBASEDATA));
		m_OBH.ocrbasesize+=sizeof(OCRBASEDATA);
		m_OBD[nElem].width=width;
		m_OBD[nElem].depth=depth;
		m_OBD[nElem].symbol=a;
		m_OBD[nElem].angle=angle;
		m_OBD[nElem].bmoff=m_OBH.heapsize;
		DWORD bmSize=width*depth;
        unsigned char* oldHeap=m_heap;
        m_heap=(unsigned char*)HeapReAlloc(m_hMem,0,m_heap,m_OBH.heapsize+bmSize);
		//m_heap=(unsigned char*)realloc(m_heap, m_OBH.heapsize+bmSize);
        if (!m_heap)
        {
            m_heap=oldHeap;
            DWORD size = HeapSize(m_hMem,0,m_heap);
            AfxMessageBox(ERROR_MEMORY);
            return(FALSE);
        }
		m_heaptail=m_heap+m_OBH.heapsize;
		memcpy(m_heaptail, data,bmSize);
		m_OBH.heapsize+=bmSize;
		nElem++;
	}
	return(TRUE);
}

BOOL COcrBase::Get(DWORD& ItemNo, CharInfo *CI)
{
	if (!nElem) return(FALSE);
	if (ItemNo>nElem-1) ItemNo=nElem-1;

    CI->data=m_heap+m_OBD[ItemNo].bmoff;
    CI->width=m_OBD[ItemNo].width;
    CI->depth=m_OBD[ItemNo].depth;
	CI->symbol=m_OBD[ItemNo].symbol;
	CI->angle=m_OBD[ItemNo].angle;
	return(TRUE);
}

BOOL COcrBase::CheckDupe(CharInfo *CI)
{
	BOOL result=FALSE;
	if (!nElem) return(FALSE);
	for (DWORD ItemNo = 0; ItemNo < nElem; ItemNo++)
	{
		result=((m_OBD[ItemNo].angle==CI->angle) &&
				(m_OBD[ItemNo].symbol==CI->symbol) &&
				(m_OBD[ItemNo].width==CI->width) &&
				(m_OBD[ItemNo].depth==CI->depth) &&
				(!memcmp(m_heap+m_OBD[ItemNo].bmoff, CI->data, CI->depth*CI->width)));
		if (result) 
		{
			break;
		}
	}
	return(result);
} 

BOOL COcrBase::CheckDupe(LPBYTE data, int width, int depth, char a, short angle)
{
   CharInfo CI;
   CI.angle=angle;
   CI.data=data;
   CI.width=width;
   CI.depth=depth;
   CI.symbol=a;
   return(CheckDupe(&CI));
}


void COcrBase::Sort(void)
{
	BOOL Result=FALSE;
	if (nElem<=1) return;
    while (!Result)
	{
		Result=TRUE;
		for(DWORD i=0; i<nElem-1; i++)
		{
           if (m_OBD[i+1].symbol < m_OBD[i].symbol)
		   {
              Result=FALSE;
              OCRBASEDATA tmpOBD=m_OBD[i];
			  m_OBD[i]=m_OBD[i+1];
			  m_OBD[i+1]=tmpOBD;
		   }
           //TRACE("%c %c\n",m_OBD[i].symbol , m_OBD[i+1].symbol );
		}
	}
    AfxMessageBox("Completed!");
}

BOOL COcrBase::ChangeChar(char a, DWORD ItemNo)
{
	m_OBD[ItemNo].symbol=a;
	return(FALSE);
}

BOOL COcrBase::Delete(DWORD ItemNo)
{
	DWORD bmSize;
	OCRBASEDATA obd;

	memcpy(&obd,&m_OBD[ItemNo],sizeof(OCRBASEDATA));
	bmSize=obd.width*obd.depth;
	for (DWORD i = 0; i < nElem; i++)
	{
		if (m_OBD[i].bmoff > m_OBD[ItemNo].bmoff)
		{
			m_OBD[i].bmoff-=bmSize;
		}
	}
	if (m_OBH.heapsize-obd.bmoff-bmSize)
		memcpy(m_heap+obd.bmoff,m_heap+obd.bmoff+bmSize,m_OBH.heapsize-obd.bmoff-bmSize);
	if (ItemNo<nElem-1)
		memcpy(&m_OBD[ItemNo],&m_OBD[ItemNo+1],sizeof(OCRBASEDATA)*(nElem-ItemNo-1));

	m_OBH.heapsize-=bmSize;
	nElem--;	
	m_OBH.ocrbasesize-=sizeof(OCRBASEDATA);
	m_heaptail=m_heap+m_OBH.heapsize;
	return(FALSE);
}

void COcrBase::_convertOLD()
{
    CharInfo CI;
    for (unsigned long i=0; i<nElem; i++)
    {
        Get(i,&CI);
        LPBYTE tmpD=(LPBYTE)malloc(CI.width*CI.depth);
        memcpy(tmpD,CI.data,CI.width*CI.depth);
        for (int y=0; y<CI.depth; y++)
        {
            LPBYTE src, srce, dst;
            src=tmpD+(CI.depth-y-1)*CI.width;
            srce=tmpD+(CI.depth-y)*CI.width;
            dst=CI.data+y*CI.width;
            while (src<srce)
            {
                *dst=(*src==0)?ACTIVE_COLOR:0;
                dst++; src++;
            }
        }
        free(tmpD);
    }
}

