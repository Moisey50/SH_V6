//  $File : Crcgn.cpp - implementing of OCR engine
//  (C) Copyright The File X Ltd 2002. 
//
//
//
//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 Firts release version, all followed changes must be listed below  (Andrey Chernushich)


#include "stdafx.h"
#include <imageproc\ocr\ocrmessages.h>
#include <imageproc\ocr\crcgn.h>
#include <imageproc\ocr\charmap.h>

// Update recognition status, during recognition 
// if defined (error handling processed anyway)

#ifdef SHOW_RECG_STATUS
#pragma message("... OCR status infoline included.")
#endif
// Structures
#define Cog unsigned char

#ifdef OCR_ALGORITHM_TIMING
    #pragma message("... OCR TIMING TURN ON.")
    double time2rec;
    #pragma comment(lib,"winmm.lib")
#endif

typedef struct tagOCRLIBHEADER
{
	char   signature[2];  // must be "HO"
	DWORD  size_of;       // size of the structure
    DWORD  libsize;
	char   VerInfo[10];
	char   reserved[30];
}OCRLIBHEADER,pOCRLIBHEADER;

#ifdef OCR_ADD_DEBUG_MODULE
#pragma message("... OCR Debug module included.")
typedef struct tagOCRDebugInfo
{
	unsigned int minWidth, maxWidth, minDepth, maxDepth;
	unsigned int CharDistr[256];
	unsigned int CharNmb;
	Cog          cogs[26][256];
}OCRDebugInfo;

OCRDebugInfo odi;
#endif

// Constants
const char  vInfo[]="2.03";
const char alpha_digits[]="o123456789";
const char alpha_small[]="abekmhopctyxd";
const char alpha_cyr[]="\x61\x62\x65\x6b\x6d\x68\x6f\x70\x63\x74\x79\x78\x64\xF6\xe3\xF8\xF9\x33\xF4\xEF\xEB\xE4\xE6\xFD\xFF\x34\xE8\xFC\xE1\xFE";
const char alpha_any[]="\x61\x62\x65\x6b\x6d\x68\x6f\x70\x63\x74\x79\x78\x64\xF6\xe3\xF8\xF9\x33\xF4\xEF\xEB\xE4\xE6\xFD\xFF\x34\xE8\xFC\xE1\xFE";
const char STATUS_NOTLOADED[] = "Library file not loaded yet."; 

// Variables
char        libfilename[512]="";
Pattern*    ocrlib=NULL;
int         bmax=0;
BOOL        updatelib=FALSE;
BOOL        Ask4Save=TRUE;
LPCTSTR		RecognitionStatus=STATUS_NOTLOADED;
//inlines and internal functions

#ifdef UNICODE
    inline int _ocr_getlib(HMODULE hModule, LPCWSTR lpName, LPCWSTR lpType )
#else
    inline int _ocr_getlib(HMODULE hModule, LPCSTR lpName, LPCSTR lpType )
#endif
{
    HRSRC hSrc=FindResource(hModule,lpName,lpType);
    int size=0;
    if (!hSrc)
    {
        RecognitionStatus="Can't load library from resource";
        TRACE("!!! Error! Can't find resource '%s'-'%s'\n",lpName,lpType);
        return 0;
    }

    HGLOBAL gocrlib=LoadResource(hModule,hSrc);
    if (!gocrlib)
    {
        RecognitionStatus="Can't load library from resource";
        TRACE("!!! Error! Can't load resource '%s'-'%s'\n",lpName,lpType);
        return 0;
    }

    LPBYTE ocrlibr =(LPBYTE)GlobalLock(gocrlib);
    if (!ocrlibr)
    {
        DeleteObject(gocrlib);
        RecognitionStatus="Can't load library from resource";
        TRACE("!!! Error! Can't lock resource '%s'-'%s'\n",lpName,lpType);
        return 0;
    }

    OCRLIBHEADER* lh=(OCRLIBHEADER*)ocrlibr;

    if ((lh->signature[0]!='H') || (lh->signature[1]!='Î') || (lh->size_of!=sizeof(OCRLIBHEADER)))
    {
        RecognitionStatus="Wrong FORMAT of the OCR library file";
        GlobalUnlock(ocrlibr);
        DeleteObject(gocrlib);
        return(0);
    }

    if (strcmp(lh->VerInfo,vInfo))
    {
        GlobalUnlock(ocrlibr);
        DeleteObject(gocrlib);
        RecognitionStatus="Wrong Version of the OCR library file";
        return(0);
    }
    if (lh->libsize)
    {
        ocrlib=(pPattern)malloc(lh->libsize);
        memcpy(ocrlib,ocrlibr+sizeof(OCRLIBHEADER),lh->libsize);
/*		if (fread(ocrlib,1,lh.libsize,libFilePtr)!=lh.libsize)
			{
                free(ocrlib); ocrlib=NULL;
				RecognitionStatus="Wrong FORMAT of the OCR library file";
				fclose(libFilePtr);
				return(0);
			}
		} */
		size=lh->libsize/sizeof(Pattern);
	    RecognitionStatus="Library sucessfully loaded";
	}
	else 
        RecognitionStatus="Library loading is failed";

    GlobalUnlock(ocrlibr);
    DeleteObject(gocrlib);
	return size; 
}

inline int _ocr_getlib(const char *libfile)
{
	strcpy(libfilename,libfile);
	FILE  *libFilePtr;
	int b=0;
	if((libFilePtr=fopen(libfile,"rb"))!=NULL)
	{
		OCRLIBHEADER lh;
		fread(&lh,4+sizeof(DWORD),1,libFilePtr);
		DWORD size=sizeof(OCRLIBHEADER);
		if ((lh.signature[0]!='H') || (lh.signature[1]!='Î') || (lh.size_of!=sizeof(OCRLIBHEADER)))
		{
            RecognitionStatus="Wrong FORMAT of the OCR library file";
			fclose(libFilePtr);
			return(0);
		}
		fseek(libFilePtr,0,SEEK_SET);
		fread(&lh,sizeof(OCRLIBHEADER),1,libFilePtr);
		if (strcmp(lh.VerInfo,vInfo))
		{
            RecognitionStatus="Wrong Version of the OCR library file";
			fclose(libFilePtr);
			return(0);
		}
		if (lh.libsize)
		{
            ocrlib=(pPattern)malloc(lh.libsize);
			if (fread(ocrlib,1,lh.libsize,libFilePtr)!=lh.libsize)
			{
                free(ocrlib); ocrlib=NULL;
				RecognitionStatus="Wrong FORMAT of the OCR library file";
				fclose(libFilePtr);
				return(0);
			}
		}
		b=lh.libsize/sizeof(Pattern);
		fclose(libFilePtr);
	    RecognitionStatus="Library sucessfully loaded";
	}
	else RecognitionStatus="Library loading is failed";
	return b;
}

inline void _ocr_calc_cogs(int h,int w,char letter[][OCR_MAXWIDTH],unsigned char cogs[])
{
	int x,y;
	unsigned int t=0,p=0;
	const int k=250;

//  Cognition coefficient 0
	for(y=0;y<h/5;y++)
	{
		for(x=0;x<w/5;x++)
		{
			if(letter[y][x])
			{
				t++;
			}
			p++;
		}
	}
	cogs[0]=(p!=0)?(k*t)/p:0;
//  Cognition coefficient 1
	t=0;p=0;
	for(y=0;y<h/5;y++)
	{
		for(x=w/5;x<2*w/5;x++)
		{
			if(letter[y][x])
			{
				t++;
			}
			p++;
		}
	}
	cogs[1]=(p!=0)?(k*t)/p:0;
//  Cognition coefficient 2
	t=0;p=0;
	for(y=0;y<h/5;y++)
	{
		for(x=2*w/5;x<w-2*w/5;x++)
		{
			if(letter[y][x])
			{
				t++;
			}
			p++;
		}
	}
	cogs[2]=(p!=0)?(k*t)/p:0;
//  Cognition coefficient 3
	t=0;p=0;
	for(y=0;y<h/5;y++)
	{
		for(x=w-2*w/5;x<w-w/5;x++)
		{
			if(letter[y][x])
			{
				t++;
			}
			p++;
		}
	}
	cogs[3]=(p!=0)?(k*t)/p:0;
//  Cognition coefficient 4
	t=0;p=0;
	for(y=0;y<h/5;y++)
	{
		for(x=w-w/5;x<w;x++)
		{
			if(letter[y][x])
			{
				t++;
			}
			p++;
		}
	}
	cogs[4]=(p!=0)?(k*t)/p:0;

//  Cognition coefficient 5
	t=0;p=0;
	for(y=h/5;y<2*h/5;y++)
	{
		for(x=0;x<w/5;x++)
		{
			if(letter[y][x])
			{
				t++;
			}
			p++;
		}
	}
	cogs[5]=(p!=0)?(k*t)/p:0;
//  Cognition coefficient 6
	t=0;p=0;
	for(y=h/5;y<2*h/5;y++)
	{
		for(x=w/5;x<2*w/5;x++)
		{
			if(letter[y][x])
			{
				t++;
			}
			p++;
		}
	}
	cogs[6]=(p!=0)?(k*t)/p:0;
//  Cognition coefficient 7
	t=0;p=0;
	for(y=h/5;y<2*h/5;y++)
	{
		for(x=2*w/5;x<w-2*w/5;x++)
		{
			if(letter[y][x])
			{
				t++;
			}
			p++;
		}
	}
	cogs[7]=(p!=0)?(k*t)/p:0;
//  Cognition coefficient 8
	t=0;p=0;
	for(y=h/5;y<2*h/5;y++)
	{
		for(x=w-2*w/5;x<w-w/5;x++)
		{
			if(letter[y][x])
			{
				t++;
			}
			p++;
		}
	}
	cogs[8]=(p!=0)?(k*t)/p:0;
//  Cognition coefficient 9
	t=0;p=0;
	for(y=h/5;y<2*h/5;y++)
	{
		for(x=w-w/5;x<w;x++)
		{
			if(letter[y][x])
			{
				t++;
			}
			p++;
		}
	}
	cogs[9]=(p!=0)?(k*t)/p:0;
//  Cognition coefficient 10
	t=0;p=0;
	for(y=2*h/5;y<h-2*h/5;y++)
	{
		for(x=0;x<w/5;x++)
		{
			if(letter[y][x])
			{
				t++;
			}
			p++;
		}
	}
	cogs[10]=(p!=0)?(k*t)/p:0;
//  Cognition coefficient 11
	t=0;p=0;
	for(y=2*h/5;y<h-2*h/5;y++)
	{
		for(x=w/5;x<2*w/5;x++)
		{
			if(letter[y][x])
			{
				t++;
			}
			p++;
		}
	}
	cogs[11]=(p!=0)?(k*t)/p:0;
//  Cognition coefficient 12
	t=0;p=0;
	for(y=2*h/5;y<h-2*h/5;y++)
	{
		for(x=2*w/5;x<w-2*w/5;x++)
		{
			if(letter[y][x])
			{
				t++;
			}
			p++;
		}
	}
	cogs[12]=(p!=0)?(k*t)/p:0;
//  Cognition coefficient 13
	t=0;p=0;
	for(y=2*h/5;y<h-2*h/5;y++)
	{
		for(x=w-2*w/5;x<w-w/5;x++)
		{
			if(letter[y][x])
			{
				t++;
			}
			p++;
		}
	}
	cogs[13]=(p!=0)?(k*t)/p:0;
//  Cognition coefficient 14
	t=0;p=0;
	for(y=2*h/5;y<h-2*h/5;y++)
	{
		for(x=w-w/5;x<w;x++)
		{
			if(letter[y][x])
			{
				t++;
			}
			p++;
		}
	}
	cogs[14]=(p!=0)?(k*t)/p:0;
//  Cognition coefficient 15
	t=0;p=0;
	for(y=h-2*h/5;y<h-h/5;y++)
	{
		for(x=0;x<w/5;x++)
		{
			if(letter[y][x])
			{
				t++;
			}
			p++;
		}
	}
	cogs[15]=(p!=0)?(k*t)/p:0;
//  Cognition coefficient 16
	t=0;p=0;
	for(y=h-2*h/5;y<h-h/5;y++)
	{
		for(x=w/5;x<2*w/5;x++)
		{
			if(letter[y][x])
			{
				t++;
			}
			p++;
		}
	}
	cogs[16]=(p!=0)?(k*t)/p:0;
//  Cognition coefficient 17
	t=0;p=0;
	for(y=h-2*h/5;y<h-h/5;y++)
	{
		for(x=2*w/5;x<w-2*w/5;x++)
		{
			if(letter[y][x])
			{
				t++;
			}
			p++;
		}
	}
	cogs[17]=(p!=0)?(k*t)/p:0;
//  Cognition coefficient 18
	t=0;p=0;
	for(y=h-2*h/5;y<h-h/5;y++)
	{
		for(x=w-2*w/5;x<w-w/5;x++)
		{
			if(letter[y][x])
			{
				t++;
			}
			p++;
		}
	}
	cogs[18]=(p!=0)?(k*t)/p:0;
//  Cognition coefficient 19
	t=0;p=0;
	for(y=h-2*h/5;y<h-h/5;y++)
	{
		for(x=w-w/5;x<w;x++)
		{
			if(letter[y][x])
			{
				t++;
			}
			p++;
		}
	}
	cogs[19]=(p!=0)?(k*t)/p:0;
//  Cognition coefficient 20
	t=0;p=0;
	for(y=h-h/5;y<h;y++)
	{
		for(x=0;x<w/5;x++)
		{
			if(letter[y][x])
			{
				t++;
			}
			p++;
		}
	}
	cogs[20]=(p!=0)?(k*t)/p:0;
//  Cognition coefficient 21
	t=0;p=0;
	for(y=h-h/5;y<h;y++)
	{
		for(x=w/5;x<2*w/5;x++)
		{
			if(letter[y][x])
			{
				t++;
			}
			p++;
		}
	}
	cogs[21]=(p!=0)?(k*t)/p:0;
//  Cognition coefficient 22
	t=0;p=0;
	for(y=h-h/5;y<h;y++)
	{
		for(x=2*w/5;x<w-2*w/5;x++)
		{
			if(letter[y][x])
			{
				t++;
			}
			p++;
		}
	}
	cogs[22]=(p!=0)?(k*t)/p:0;
//  Cognition coefficient 23
	t=0;p=0;
	for(y=h-h/5;y<h;y++)
	{
		for(x=w-2*w/5;x<w-w/5;x++)
		{
			if(letter[y][x])
			{
				t++;
			}
			p++;
		}
	}
	cogs[23]=(p!=0)?(k*t)/p:0;
//  Cognition coefficient 24
	t=0;p=0;
	for(y=h-h/5;y<h;y++)
	{
		for(x=w-w/5;x<w;x++)
		{
			if(letter[y][x])
			{
				t++;
			}
			p++;
		}
	}
	cogs[24]=(p!=0)?(k*t)/p:0;
//  Cognition coefficient 25
	cogs[25]=(int)((25*h)/w);
}

inline int _ocr_calc_distance(unsigned char *cogs1, unsigned char *cogs2)
{
	int terror=0;
//	int i;
    int d;
/*	for(i=0;i<25;i++)
	{
        d=cogs1[i]-cogs2[i];
		terror+=d*d;
	}*/
    d=cogs1[0]-cogs2[0];
    terror+=d*d;
    d=cogs1[1]-cogs2[1];
    terror+=d*d;
    d=cogs1[2]-cogs2[2];
    terror+=d*d;
    d=cogs1[3]-cogs2[3];
    terror+=d*d;
    d=cogs1[4]-cogs2[4];
    terror+=d*d;
    d=cogs1[5]-cogs2[5];
    terror+=d*d;
    d=cogs1[6]-cogs2[6];
    terror+=d*d;
    d=cogs1[7]-cogs2[7];
    terror+=d*d;
    d=cogs1[8]-cogs2[8];
    terror+=d*d;
    d=cogs1[9]-cogs2[9];
    terror+=d*d;
    d=cogs1[10]-cogs2[10];
    terror+=d*d;
    d=cogs1[11]-cogs2[11];
    terror+=d*d;
    d=cogs1[12]-cogs2[12];
    terror+=d*d;
    d=cogs1[13]-cogs2[13];
    terror+=d*d;
    d=cogs1[14]-cogs2[14];
    terror+=d*d;
    d=cogs1[15]-cogs2[15];
    terror+=d*d;
    d=cogs1[16]-cogs2[16];
    terror+=d*d;
    d=cogs1[17]-cogs2[17];
    terror+=d*d;
    d=cogs1[18]-cogs2[18];
    terror+=d*d;
    d=cogs1[19]-cogs2[19];
    terror+=d*d;
    d=cogs1[20]-cogs2[20];
    terror+=d*d;
    d=cogs1[21]-cogs2[21];
    terror+=d*d;
    d=cogs1[22]-cogs2[22];
    terror+=d*d;
    d=cogs1[23]-cogs2[23];
    terror+=d*d;
    d=cogs1[24]-cogs2[24];
    terror+=d*d;
    d=cogs1[25]-cogs2[25];
    terror+=((d*d)<<2);
	return terror;
}

inline int _ocr_seek_char(Pattern& patp)
{
	int e,emin=-1;
	int dmin=-1;
	int b;
	unsigned char ch=0;
	if(bmax)
	{
		//Searching for char
		emin=dmin=_ocr_calc_distance(patp.cogs,ocrlib[0].cogs);
		ch=ocrlib[0].ch;
		for(b=1;b<bmax;b++)
		{
			e=_ocr_calc_distance(patp.cogs,ocrlib[b].cogs);
			if(e<emin) {emin=e; ch=ocrlib[b].ch;}
			if((ch!=ocrlib[b].ch)&&(e<dmin)) {dmin=e;}
		}
	}
	patp.ch=ch;
	if (dmin==0) return(100);
	int ret=100*emin/dmin;
	return (ret);
}

inline int _ocr_seek_char_alpha(Pattern& patp, int alpha)
{
	int e,emin=0xFFFFFFF;
	int dmin  =0xFFFFFFF;
	int b;
	char *char_set;
	unsigned char ch='?';
	switch (alpha)
	{
	case ALPHA_DIGIT:
		char_set=(char *)alpha_digits;
		break;
	case ALPHA_SMALL:
		char_set=(char *)alpha_small;
		break;
    case ALPHA_CYR:
		char_set=(char *)alpha_cyr;
		break;
    default:
		char_set=NULL;
	}
	if(bmax)
	{
		for(b=0;b<bmax;b++)
		{
			e=_ocr_calc_distance(patp.cogs,ocrlib[b].cogs);
			if((e<emin) && strchr(char_set,ocrlib[b].ch)) 
            {
                emin=e; ch=ocrlib[b].ch;
            }
			if((ch!=ocrlib[b].ch) && (e<dmin) && strchr(char_set,ocrlib[b].ch)) 
            {
                dmin=e;
            }
		}
	}
	patp.ch=ch;
	if (dmin==0) return(100);
	int ret=100*emin/dmin;
	return (ret);
}

inline void _ocr_add_char_to_lib(Pattern& pat)
{
    ocrlib=(pPattern)realloc(ocrlib,(bmax+1)*sizeof(Pattern));
	memcpy(&ocrlib[bmax++],&pat,sizeof(Pattern));
	updatelib=1;
}

//////////////////////////////////////////////////////////////
// Interface functions
//////////////////////////////////////////////////////////////

BOOL OCR_IsReady(void)
{
	return(bmax);
}

LPCTSTR OCR_GetStatus(void)
{
    return(RecognitionStatus);
}

#ifdef UNICODE
    BOOL OCR_Init(HMODULE hModule, LPCWSTR lpName, LPCWSTR lpType )
#else
    BOOL OCR_Init(HMODULE hModule, LPCSTR lpName, LPCSTR lpType )
#endif
{
	if (!bmax) bmax=_ocr_getlib(hModule, lpName, lpType);
    return(bmax!=0);
}

BOOL OCR_Init(LPCTSTR Path)
{
    char path[256];
    strcpy(path,Path);
    strcat(path,"ocr.lib");
	if (!bmax) bmax=_ocr_getlib(LPCTSTR(path));
    return(bmax);
}

BOOL OCR_Load(LPCTSTR PathName)
{
	if (!bmax) bmax=_ocr_getlib(PathName);
    return(bmax);
}

void OCR_Free()
{
	if ((ocrlib) && (updatelib))
	{
          OCR_Save();
	}
    if (ocrlib)
    {
        free(ocrlib);
        ocrlib=NULL;
    }
    bmax=0;
    #ifdef SHOW_RECG_STATUS
	RecognitionStatus="Engine free";
    #endif
}

void OCR_Save()
{
	if ((!Ask4Save) || (AfxMessageBox(PROMPT_SAVEOCRLIB,MB_YESNO)==IDYES))
	{
		FILE  *libFilePtr;
		if((libFilePtr=fopen(libfilename,"wb"))!=NULL)
		{
			OCRLIBHEADER lh;
			lh.signature[0]='H';
			lh.signature[1]='Î';
	        lh.size_of=sizeof(OCRLIBHEADER);
            lh.libsize=sizeof(Pattern)*bmax;
			memset(lh.VerInfo,0x00,10);
			strcpy(lh.VerInfo,vInfo);
	        memset(lh.reserved,0xAF,30);
			fwrite(&lh,sizeof(lh),1,libFilePtr);
			if (bmax) fwrite(ocrlib,sizeof(Pattern),bmax,libFilePtr);
			fclose(libFilePtr);
            #ifdef SHOW_RECG_STATUS
			RecognitionStatus="Library sucessfully saved";
            #endif
			updatelib=FALSE;
		}
	}
	else
	{
		updatelib=FALSE;
	}
}


void OCR_CreateNewLibrary()
{
	FILE  *libFilePtr;
	if((libFilePtr=fopen(libfilename,"wb"))!=NULL)
	{
			OCRLIBHEADER lh;
			bmax=0;
			lh.signature[0]='H';
			lh.signature[1]='Î';
			memset(lh.VerInfo,0x00,10);
			strcpy(lh.VerInfo,vInfo);
	        lh.size_of=sizeof(OCRLIBHEADER);
            lh.libsize=sizeof(Pattern)*bmax;
	        memset(lh.reserved,0xAF,30);
			fwrite(&lh,sizeof(lh),1,libFilePtr);
			// ??? if (bmax) fwrite(lib,sizeof(Pattern),bmax,libFilePtr);
			fclose(libFilePtr);
            #ifdef SHOW_RECG_STATUS
			RecognitionStatus="Library sucessfully created";
            #endif
			updatelib=FALSE;
	}
}

char mes1[100]="Suggested Character ' ', with error =                   ";
char mes2[100]="Perfect Match Character = ' '.";

void OCR_RecognizeChar(pCluster cluster, int alpha, Pattern& pat)
{
	unsigned h = cluster->size.cy,w = cluster->size.cx;
	char letter[OCR_MAXHEIGHT][OCR_MAXWIDTH];

#ifdef OCR_ALGORITHM_TIMING
	int ts, te;
    ts=GetTickCount();
#endif
	if  (
		(h>=OCR_MAXHEIGHT)
		||
		(w>=OCR_MAXWIDTH)
        ||
		(h<OCR_MINHEIGHT)
		||
		(w<OCR_MINWIDTH)

		)
    {
        cluster->rec_error=101;
		cluster->rec_as='^';
        return;
    }
    
	for (unsigned i=0; i<h*w; i++) 
	{
		letter[i/w][i%w]=(cluster->clstBM[i]==ACTIVE_COLOR)?1:0;
	}

	if ((!h) || (!w))
	{
        #ifdef SHOW_RECG_STATUS
	    RecognitionStatus="Char matrix is empty, can't make any guesses.";
        #endif
        cluster->rec_error=101;
		cluster->rec_as='^';
        return;
	}

	_ocr_calc_cogs(h,w,letter,pat.cogs);
	pat.ch='.'; //Default character

	if (alpha==ALPHA_ANY)
		cluster->rec_error=_ocr_seek_char(pat);
	else
		cluster->rec_error=_ocr_seek_char_alpha(pat,alpha);
    #ifdef SHOW_RECG_STATUS
	    if(cluster->rec_error)
	    {
			    if(cluster->rec_error==-1)
				    RecognitionStatus="Error: Library file is empty.";
			    else
			    {
                    RecognitionStatus=mes1;
                    *(strchr(RecognitionStatus,'\'')+1)=(pat.ch==0)?'!':pat.ch;
				    char tmpS[20];
				    itoa(cluster->rec_error,tmpS,10);
				    strcpy((strchr(RecognitionStatus,'=')+1),tmpS);
			    }
		    }
	    else
	    {
            RecognitionStatus=mes2;
		    *(strchr(RecognitionStatus,'\'')+1)=pat.ch;
	    }
    #endif
    cluster->rec_as=_char_id_to_bestviewcap(pat.ch);
#ifdef OCR_ALGORITHM_TIMING
	te=GetTickCount();
	time2rec=((double)(te-ts))/1000.0;
#endif
}

BOOL OCR_AddCharToLib(Pattern& pat)
{
    unsigned char a=pat.ch; // Save org char
	if (_ocr_seek_char(pat)==0) return(FALSE); // if error ==0 we can make any addition
    pat.ch=a;
    _ocr_add_char_to_lib(pat);
	OCR_Save();
	return(TRUE);
}

#ifdef OCR_ALGORITHM_TIMING
double OCR_GetTimeSpent()
{
    return(time2rec);
}
#endif

#ifdef OCR_ADD_DEBUG_MODULE

void OCR_DebugInfoRst(void)
{
	
	odi.minWidth=255;
	odi.maxWidth=0;
	odi.minDepth=255;
	odi.maxDepth=0;
	memset(odi.CharDistr,0,sizeof(int)*256);
	odi.CharNmb=0;
	memset(odi.cogs,0,9*256*sizeof(Cog));
}

void OCR_DebugInfoWrite(CStdioFile *lf)
{
   CString tmpS;
   tmpS.Format("Char min width=%d\n",odi.minWidth); lf->WriteString(tmpS);
   tmpS.Format("Char max width=%d\n",odi.maxWidth); lf->WriteString(tmpS);
   tmpS.Format("Char min depth=%d\n",odi.minDepth); lf->WriteString(tmpS);
   tmpS.Format("Char max depth=%d\n",odi.maxDepth); lf->WriteString(tmpS);
   tmpS.Format("Chars processed=%d\n",odi.CharNmb); lf->WriteString(tmpS);
   unsigned i;
   for (i=' '; i<256; i++)
   {
	   if (odi.CharDistr[i]) 
	   {
		   tmpS.Format("Char '%c' was added %d times\n",i,odi.CharDistr[i]); lf->WriteString(tmpS);
	   }
   }
   lf->WriteString("Cognition info: \n");
   for (i=' '; i<256; i++)
   {
		if (odi.CharDistr[i]) 
		{
			odi.cogs[0][i]/=odi.CharDistr[i];
		    odi.cogs[1][i]/=odi.CharDistr[i];
  		    odi.cogs[2][i]/=odi.CharDistr[i];
		    odi.cogs[3][i]/=odi.CharDistr[i];
		    odi.cogs[4][i]/=odi.CharDistr[i];
		    odi.cogs[5][i]/=odi.CharDistr[i];
		    odi.cogs[6][i]/=odi.CharDistr[i];
		    odi.cogs[7][i]/=odi.CharDistr[i];
		    odi.cogs[8][i]/=odi.CharDistr[i];
			odi.cogs[9][i]/=odi.CharDistr[i];
		    odi.cogs[10][i]/=odi.CharDistr[i];
  		    odi.cogs[11][i]/=odi.CharDistr[i];
		    odi.cogs[12][i]/=odi.CharDistr[i];
		    odi.cogs[13][i]/=odi.CharDistr[i];
		    odi.cogs[14][i]/=odi.CharDistr[i];
		    odi.cogs[15][i]/=odi.CharDistr[i];
		    odi.cogs[16][i]/=odi.CharDistr[i];
		    odi.cogs[17][i]/=odi.CharDistr[i];
			odi.cogs[18][i]/=odi.CharDistr[i];
		    odi.cogs[19][i]/=odi.CharDistr[i];
		    odi.cogs[20][i]/=odi.CharDistr[i];
			odi.cogs[21][i]/=odi.CharDistr[i];
		    odi.cogs[22][i]/=odi.CharDistr[i];
		    odi.cogs[23][i]/=odi.CharDistr[i];
		    odi.cogs[24][i]/=odi.CharDistr[i];
		    odi.cogs[25][i]/=odi.CharDistr[i];

			tmpS.Format("%c\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
				i,odi.cogs[0][i], odi.cogs[1][i], odi.cogs[2][i], odi.cogs[3][i], odi.cogs[4][i], odi.cogs[5][i],odi.cogs[6][i], odi.cogs[7][i],odi.cogs[8][i], odi.cogs[9][i],odi.cogs[10][i], odi.cogs[11][i],odi.cogs[12][i], odi.cogs[13][i],odi.cogs[14][i],odi.cogs[15][i],odi.cogs[16][i],odi.cogs[17][i],odi.cogs[18][i],odi.cogs[19][i],odi.cogs[20][i],odi.cogs[21][i],odi.cogs[22][i],odi.cogs[23][i],odi.cogs[24][i],odi.cogs[25][i]);
		    lf->WriteString(tmpS);
		}
   }
}

void OCR_DebugAdd(unsigned char a, Cluster& cluster,Pattern& pat)
{
	unsigned int h=cluster.size.cx,w=cluster.size.cy;
    ASSERT(w!=0);
	
    if (h<odi.minDepth) odi.minDepth=h;
	if (w<odi.minWidth) odi.minWidth=w;
	if (h>odi.maxDepth) odi.maxDepth=h;
	if (w>odi.maxWidth) odi.maxWidth=w;
    ASSERT(w!=0);
    odi.CharDistr[a]++;
	odi.CharNmb++;

	char letter[OCR_MAXHEIGHT][OCR_MAXWIDTH];
	int xx=10,yy;
	yy=410;
    
//    TRACE("___ %d %d\n",h,w);
	for (unsigned i=0; i<h*w; i++) 
	{
		letter[i/w][i%w]=(cluster.clstBM[i]==ACTIVE_COLOR)?1:0;
        ASSERT(w!=0);
	}
    ASSERT(w!=0);
	_ocr_calc_cogs(h,w,letter,pat.cogs);
	odi.cogs[0][a]+=pat.cogs[0];
	odi.cogs[1][a]+=pat.cogs[1];
	odi.cogs[2][a]+=pat.cogs[2];
	odi.cogs[3][a]+=pat.cogs[3];
	odi.cogs[4][a]+=pat.cogs[4];
	odi.cogs[5][a]+=pat.cogs[5];
	odi.cogs[6][a]+=pat.cogs[6];
	odi.cogs[7][a]+=pat.cogs[7];
	odi.cogs[8][a]+=pat.cogs[8];
	odi.cogs[9][a]+=pat.cogs[9];
	odi.cogs[10][a]+=pat.cogs[10];
	odi.cogs[11][a]+=pat.cogs[11];
	odi.cogs[12][a]+=pat.cogs[12];
	odi.cogs[13][a]+=pat.cogs[13];
	odi.cogs[14][a]+=pat.cogs[14];
	odi.cogs[15][a]+=pat.cogs[15];
	odi.cogs[16][a]+=pat.cogs[16];
	odi.cogs[17][a]+=pat.cogs[17];
	odi.cogs[18][a]+=pat.cogs[18];
	odi.cogs[19][a]+=pat.cogs[19];
	odi.cogs[20][a]+=pat.cogs[20];
	odi.cogs[21][a]+=pat.cogs[21];
	odi.cogs[22][a]+=pat.cogs[22];
	odi.cogs[23][a]+=pat.cogs[23];
	odi.cogs[24][a]+=pat.cogs[24];
	odi.cogs[25][a]+=pat.cogs[25];
}

#endif
