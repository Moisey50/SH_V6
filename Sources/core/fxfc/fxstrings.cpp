#include "stdafx.h"
#include "fxmodules.h"
#include <fxfc\fxfc.h>


#ifdef _DEBUG
    //#define CHK_HEAP
    #ifdef CHK_HEAP
        #include <malloc.h>
        #define TEST_HEAP_NOW ASSERT(_heapchk()==_HEAPOK)
    #else
        #define TEST_HEAP_NOW
    #endif

    #ifndef ASSERT
        #define ASSERT(a)
    #endif
#else
    #define TEST_HEAP_NOW
    #ifndef ASSERT
        #define ASSERT(a)
    #endif
#endif

__forceinline WCHAR ansi2unicode(CHAR a)
{
    CHAR ansi[2]={a,0};
    WCHAR unicode[2]={0,0};
    MultiByteToWideChar(CP_ACP, 0, ansi, -1, unicode, 2);
    TEST_HEAP_NOW;
    return unicode[0];
}

__forceinline CHAR unicode2ansi(WCHAR a)
{
    BOOL unkchrs(FALSE);
    CHAR ansi[2]={0,0};
    WCHAR unicode[2]={a,0};
    WideCharToMultiByte(CP_ACP,0,unicode,-1,ansi,2,NULL,&unkchrs);
    TEST_HEAP_NOW;
    return ansi[0];
}

pfxsExt FXString::_getExt() const     
{ 
    if (!m_str) 
        return NULL; 
    return (pfxsExt)(((LPBYTE)m_str)-sizeof(fxsExt)); 
}

bool    FXString::_islocked()    
{ 
    if (!m_str) 
        return false; 
    return _getExt()->Locked; 
}

FXSIZE   FXString::_buflen()      
{ 
    if (!m_str) 
        return 0; 
    return _getExt()->Length; 
}

bool FXString::_alloc(FXSIZE size)
{
    TEST_HEAP_NOW;
    if (_islocked()) return false;
    if (size<_buflen()) return true;
    if (m_str)
    {
        LPBYTE ptr=(LPBYTE)realloc(_getExt(),(size+1)*sizeof(TCHAR)+sizeof(fxsExt));
        #ifdef _DEBUG
            FXSIZE err=errno;
            ASSERT(ptr!=NULL);
        #endif
        if (ptr)
        {
            m_str=(LPTSTR)(ptr+sizeof(fxsExt));
            _getExt()->Length=size+1;
            _getExt()->Locked=false;
        }
        else
        {
            TEST_HEAP_NOW;
            free(_getExt());
            m_str=NULL;
            return false;
        }
    }
    else
    {
        LPBYTE ptr=(LPBYTE)malloc((size+1)*sizeof(TCHAR)+sizeof(fxsExt));
        ASSERT(ptr!=NULL);
        if (ptr)
        {
            m_str=(LPTSTR)(ptr+sizeof(fxsExt));
            *m_str=_T('\0');
            _getExt()->Length=size+1;
            _getExt()->Locked=false;
        }
        else
        {
            TEST_HEAP_NOW;
            return false;
        }
    }
    return true;
    TEST_HEAP_NOW;
}

bool    FXString::_erase()
{
    TEST_HEAP_NOW;
    if (_islocked()) return false;
    if (m_str)
    {
        LPBYTE ptr=(LPBYTE)realloc(_getExt(),sizeof(TCHAR)+sizeof(fxsExt));
        ASSERT(ptr!=NULL);
        if (ptr)
        {
            m_str=(LPTSTR)(ptr+sizeof(fxsExt));
            _getExt()->Length=1;
            _getExt()->Locked=false;
        }
        else
        {
            TEST_HEAP_NOW;
            free(_getExt());
            m_str=NULL;
            return false;
        }
    }
    else
    {
        LPBYTE ptr=(LPBYTE)malloc(sizeof(TCHAR)+sizeof(fxsExt));
        ASSERT(ptr!=NULL);
        if (ptr)
        {
            m_str=(LPTSTR)(ptr+sizeof(fxsExt));
            _getExt()->Length=1;
            _getExt()->Locked=false;
        }
        else
        {
            TEST_HEAP_NOW;
            return false;
        }
    }
    *m_str=_T('\0');
    TEST_HEAP_NOW;
    return true;
}

bool FXString::_resetbuflen()
{
    TEST_HEAP_NOW;
    if (!m_str) return false;
    if (_islocked()) return false;
    FXSIZE size=(m_str)?_tcslen(m_str):0;
    if (size==0) 
        return _erase(); 
    if (size+1==_buflen()) return true;
    LPBYTE ptr=(LPBYTE)realloc(_getExt(),(size+1)*sizeof(TCHAR)+sizeof(fxsExt));
    ASSERT(ptr!=NULL);
    if (ptr)
    {
        m_str=(LPTSTR)(ptr+sizeof(fxsExt));
        _getExt()->Length=size+1;
        _getExt()->Locked=false;
    }
    else
    {
        TEST_HEAP_NOW;
        m_str=NULL;
        return false;
    }
    TEST_HEAP_NOW;
    return true;
}

void FXString::_copy(LPCSTR  lpsz)
{
    size_t len=(lpsz)?strlen(lpsz):0;
    _alloc(len);
    if (m_str)
    {
        if (len)
        {
            #ifdef _UNICODE
                MultiByteToWideChar(CP_ACP, 0, lpsz, -1, m_str, _buflen());
            #else
                strcpy_s(m_str,_buflen(),lpsz);
            #endif
        }
        else
            *m_str=_T('\0');
    }
    TEST_HEAP_NOW;
}

void FXString::_copy(LPCWSTR lpsz)
{
    size_t len=wcslen(lpsz);
    _alloc(len);
    if (m_str)
    {
        if (len)
        {
            #ifdef _UNICODE
                wcscpy_s(m_str,_buflen(),lpsz);
            #else
                BOOL unkchrs(FALSE);
                WideCharToMultiByte(CP_ACP,0,lpsz,-1,m_str,(int)_buflen(),NULL,&unkchrs);
            #endif
        }
        else
            *m_str=_T('\0');
    }
    TEST_HEAP_NOW;
}

void FXString::Preallocate(FXSIZE nLength)
{
    _alloc(nLength);
}

FXSIZE FXString::GetAllocLength( ) const
{
    if (m_str==NULL) 
      return 0;
    FXSIZE len=(_getExt())->Length;
    return len;
}

FXSIZE FXString::GetLength() const 
{
    if (!m_str) 
      return 0;
    return _tcslen(m_str);
}

BOOL FXString::IsEmpty() const 
{
    return ( !m_str || (m_str[0]==_T('\0')) );
}

void FXString::Empty() 
{
   _erase();
}

void FXString::FreeExtra()
{
    _resetbuflen();
}

LPCTSTR FXString::GetString( ) const
{
    return m_str;
}

FXString::operator LPCTSTR() const 
{
    return GetString();
}

FXString::FXString():
    m_str(NULL)
{
    _alloc(0);
    ASSERT(m_str!=NULL);
    TEST_HEAP_NOW;
}

FXString::FXString(const FXString& stringSrc):
    m_str(NULL)
{
    if (stringSrc.GetLength()!=0)
    {
        _alloc(stringSrc.GetLength());
        if (m_str)
            _tcscpy_s(m_str,GetAllocLength(),stringSrc);
    }
    else
        _alloc(0);
    ASSERT(m_str!=NULL);
    TEST_HEAP_NOW;
}

FXString::FXString(CHAR ansi, FXSIZE nRepeat):
    m_str(NULL)
{
    if (nRepeat>0)
    {
        _alloc(nRepeat);
        TCHAR ch;
        #ifdef _UNICODE
            ch=ansi2unicode(ansi);
        #else
            ch=ansi;
        #endif
        if (nRepeat>GetAllocLength()-1) nRepeat=GetAllocLength()-1;
        m_str[nRepeat]=_T('\0');
        while (nRepeat)
        {
            m_str[nRepeat-1]=ch;
            nRepeat--;
        }
    }
    else
        _alloc(0);
    ASSERT(m_str!=NULL);
    TEST_HEAP_NOW;
}

FXString::FXString(WCHAR unicode, FXSIZE nRepeat):
    m_str(NULL)
{
    if (nRepeat>0)
    {
        _alloc(nRepeat);
        TCHAR ch;
        #ifdef _UNICODE
            ch=unicode;
        #else
            ch=unicode2ansi(unicode);
        #endif
        if (nRepeat>GetAllocLength()-1) nRepeat=GetAllocLength()-1;
        m_str[nRepeat]=_T('\0');
        while (nRepeat)
        {
            m_str[nRepeat-1]=ch;
            nRepeat--;
        }
    }
    else
        _alloc(0);
    ASSERT(m_str!=NULL);
    TEST_HEAP_NOW;
}

FXString::FXString(LPCSTR  lpsz):
    m_str(NULL)
{
    _copy(lpsz);
}

FXString::FXString(LPCWSTR lpsz):
    m_str(NULL)
{
    _copy(lpsz);
}

FXString::FXString(LPCSTR lpsz, FXSIZE nLength):
    m_str(NULL)
{
    nLength=((DWORD)nLength>strlen(lpsz))?strlen(lpsz):nLength;
    _alloc(nLength);
    ASSERT(m_str!=NULL);
    if (m_str)
    {
        if (nLength>0)
        {
            #ifdef _UNICODE
                MultiByteToWideChar(CP_ACP, 0, lpsz, nLength, m_str, GetAllocLength());
            #else
                memcpy_s(m_str,GetAllocLength()*sizeof(TCHAR),lpsz,nLength*sizeof(TCHAR));
            #endif
            m_str[nLength]=_T('\0');
        }
    }
    TEST_HEAP_NOW;
}

FXString::FXString(LPCWSTR lpsz, FXSIZE nLength):
    m_str(NULL)
{
    nLength=((DWORD)nLength>wcslen(lpsz))?wcslen(lpsz):nLength;
    _alloc(nLength);
    ASSERT(m_str!=NULL);
    if (m_str)
    {
        if (nLength>0)
        {
            #ifdef _UNICODE
                memcpy_s(m_str,GetAllocLength()*sizeof(TCHAR),lpsz,nLength*sizeof(WCHAR));
            #else
                BOOL unkchrs(FALSE);
                WideCharToMultiByte(CP_ACP,0,lpsz,(int)nLength,m_str,
                  (int)GetAllocLength(),NULL,&unkchrs);
            #endif
            m_str[nLength]=_T('\0');
        }
    }
    TEST_HEAP_NOW;
}

FXString::FXString(const unsigned char* lpsz):
    m_str(NULL)
{
    FXSIZE len=strlen((LPCSTR)lpsz);
    _alloc(len);
    ASSERT(m_str!=NULL);
    if (m_str)
    {
        if (len)
        {
            #ifdef _UNICODE
                MultiByteToWideChar(CP_ACP, 0, (LPCSTR)lpsz, -1, m_str, GetAllocLength());
            #else
                strcpy_s(m_str,GetAllocLength(),(LPCSTR)lpsz);
            #endif
        }
    }
    TEST_HEAP_NOW;
}

FXString::~FXString() 
{
    TEST_HEAP_NOW;
    if (m_str)
        free(_getExt()); m_str=NULL; 
}

TCHAR FXString::GetAt(FXSIZE nIndex) const 
{
    ASSERT(nIndex>=0);
    ASSERT(nIndex<=GetLength());
    if ((nIndex<0) || (nIndex>GetAllocLength())) return _T('\0');
    return m_str[nIndex];
}

TCHAR FXString::operator[](FXSIZE nIndex) const 
{
    ASSERT(nIndex>=0);
    ASSERT(nIndex<=GetLength());
    if ((nIndex<0) || (nIndex>GetAllocLength())) return _T('\0');
    return m_str[nIndex];
}

void FXString::SetAt(FXSIZE nIndex, TCHAR ch) 
{
    ASSERT(nIndex>=0);
    ASSERT(nIndex<=GetLength());
    if ((nIndex<0) || (nIndex>GetAllocLength())) return;
    m_str[nIndex] = ch;
    TEST_HEAP_NOW;
}

int FXString::Compare(LPCTSTR lpsz) const 
{
    ASSERT(m_str);
    FXSIZE len=GetLength();
    if ((len==0) && (lpsz==NULL)) return 0;
    if ((len==0) && (_tcslen(lpsz)==0)) return 0;
    if (lpsz!=NULL)
        return _tcscmp(m_str, lpsz);
    return 1;
}

bool operator==(const FXString& str1, const FXString& str2)
{
    return (str1.Compare(str2)==0);
} 

bool operator==(const FXString& str1, LPCTSTR lpsz2)
{
    return (str1.Compare(lpsz2)==0);
}

bool operator==(LPCTSTR lpsz1, const FXString& str2)
{
    return (str2.Compare(lpsz1)==0);
}

bool operator==(const FXString& str1, TCHAR ch2)
{
    TCHAR lpsz[2]={ch2,_T('\0')};
    return (str1.Compare(lpsz)==0);
}

bool operator==(TCHAR ch1, const FXString& str2)
{
    TCHAR lpsz[2]={ch1,_T('\0')};
    return (str2.Compare(lpsz)==0);
}

bool operator!=(const FXString& str1, const FXString& str2)
{
    return (str1.Compare(str2)!=0);
} 

bool operator!=(const FXString& str1, LPCTSTR lpsz2)
{
    return (str1.Compare(lpsz2)!=0);
}

bool operator!=(LPCTSTR lpsz1, const FXString& str2)
{
    return (str2.Compare(lpsz1)!=0);
}

bool operator!=(const FXString& str1, TCHAR ch2)
{
    TCHAR lpsz[2]={ch2,_T('\0')};
    return (str1.Compare(lpsz)!=0);
}

bool operator!=(TCHAR ch1, const FXString& str2)
{
    TCHAR lpsz[2]={ch1,_T('\0')};
    return (str2.Compare(lpsz)!=0);
}

bool operator<(const FXString& str1, const FXString& str2)
{
    return (str1.Compare(str2)<0);
} 

bool operator<(const FXString& str1, LPCTSTR lpsz2)
{
    return (str1.Compare(lpsz2)<0);
}

bool operator<(LPCTSTR lpsz1, const FXString& str2)
{
    return (str2.Compare(lpsz1)>0);
}

bool operator<(const FXString& str1, TCHAR ch2)
{
    TCHAR lpsz[2]={ch2,_T('\0')};
    return (str1.Compare(lpsz)<0);
}

bool operator<(TCHAR ch1, const FXString& str2)
{
    TCHAR lpsz[2]={ch1,_T('\0')};
    return (str2.Compare(lpsz)>0);
}

bool operator>(const FXString& str1, const FXString& str2)
{
    return (str1.Compare(str2)>0);
} 

bool operator>(const FXString& str1, LPCTSTR lpsz2)
{
    return (str1.Compare(lpsz2)>0);
}

bool operator>(LPCTSTR lpsz1, const FXString& str2)
{
    return (str2.Compare(lpsz1)<0);
}

bool operator>(const FXString& str1, TCHAR ch2)
{
    TCHAR lpsz[2]={ch2,_T('\0')};
    return (str1.Compare(lpsz)>0);
}

bool operator>(TCHAR ch1, const FXString& str2)
{
    TCHAR lpsz[2]={ch1,_T('\0')};
    return (str2.Compare(lpsz)<0);
}

bool operator<=(const FXString& str1, const FXString& str2)
{
    return (str1.Compare(str2)<=0);
} 

bool operator<=(const FXString& str1, LPCTSTR lpsz2)
{
    return (str1.Compare(lpsz2)<=0);
}

bool operator<=(LPCTSTR lpsz1, const FXString& str2)
{
    return (str2.Compare(lpsz1)>=0);
}

bool operator<=(const FXString& str1, TCHAR ch2)
{
    TCHAR lpsz[2]={ch2,_T('\0')};
    return (str1.Compare(lpsz)<=0);
}

bool operator<=(TCHAR ch1, const FXString& str2)
{
    TCHAR lpsz[2]={ch1,_T('\0')};
    return (str2.Compare(lpsz)>=0);
}

bool operator>=(const FXString& str1, const FXString& str2)
{
    return (str1.Compare(str2)>=0);
} 

bool operator>=(const FXString& str1, LPCTSTR lpsz2)
{
    return (str1.Compare(lpsz2)>=0);
}

bool operator>=(LPCTSTR lpsz1, const FXString& str2)
{
    return (str2.Compare(lpsz1)<=0);
}

bool operator>=(const FXString& str1, TCHAR ch2)
{
    TCHAR lpsz[2]={ch2,_T('\0')};
    return (str1.Compare(lpsz)>=0);
}

bool operator>=(TCHAR ch1, const FXString& str2)
{
    TCHAR lpsz[2]={ch1,_T('\0')};
    return (str2.Compare(lpsz)<=0);
}

int FXString::CompareNoCase(LPCTSTR lpsz) const 
{
   return _tcsicmp(m_str, lpsz);
}

int FXString::Collate(LPCTSTR lpsz) const 
{
   return _tcscoll (m_str, lpsz);
}

int FXString::CollateNoCase(LPCTSTR lpsz) const 
{
   return _tcsicoll(m_str, lpsz);
}

const FXString& FXString::operator=(const FXString& stringSrc) 
{
   if (&stringSrc != this) 
   {
       FXSIZE len=stringSrc.GetLength();
       if (len==0)
           _erase();
       else
       {
           _alloc(len);
           _tcscpy_s(m_str,GetAllocLength(),stringSrc);
       }
   } 
   TEST_HEAP_NOW;
   return *this;
}

const FXString& FXString::operator=(WCHAR ch) 
{
    _alloc(2);
    #ifdef _UNICODE
        m_str[0] = ch;
    #else
        m_str[0] = unicode2ansi(ch);
    #endif
    m_str[1] = _T('\0');
    TEST_HEAP_NOW;
    return *this;
}

const FXString& FXString::operator=(CHAR ch) 
{
    _alloc(2);
    #ifdef _UNICODE
        m_str[0] = ansi2unicode(ch);
    #else
        m_str[0] = ch;
    #endif
    m_str[1] = _T('\0');
    TEST_HEAP_NOW;
    return *this;
}

const FXString& FXString::operator=(LPCSTR  lpsz) 
{
    if ((!lpsz) || (strlen(lpsz)==0))
        _erase();
    else
        _copy(lpsz); 
    TEST_HEAP_NOW;
    return *this;
}

const FXString& FXString::operator=(LPCWSTR lpsz) 
{
    if ((!lpsz) || (wcslen(lpsz)==0))
        _erase();
    else
        _copy(lpsz); 
    TEST_HEAP_NOW;
    return *this;
}

const FXString& FXString::operator=(const unsigned char* lpsz) 
{
    if ((!lpsz) || (strlen((CHAR*)lpsz)==0))
        _erase();
    else
        _copy((CHAR*)lpsz); 
    TEST_HEAP_NOW;
    return *this;
}

void FXString::SetString(LPCTSTR pszSrc)
{
    if ((!pszSrc) || (_tcslen(pszSrc)==0))
        _erase();
    else
        _copy(pszSrc); 
    TEST_HEAP_NOW;
}

void FXString::Append(const FXString& strSrc)
{
    if (strSrc.IsEmpty()) return;
    FXSIZE newlen=GetLength()+strSrc.GetLength();
    _alloc(newlen);
    _tcscat_s(m_str,GetAllocLength(),strSrc);
    TEST_HEAP_NOW;
    return;
}

void FXString::Append(LPCTSTR pszSrc,FXSIZE nLength)
{
    if ((!pszSrc) || (_tcslen(pszSrc)==0)) return;
    nLength=( (size_t)nLength > _tcslen(pszSrc) )?_tcslen(pszSrc):nLength;
    FXSIZE len=GetLength();
    FXSIZE newlen=len+nLength;
    _alloc(newlen);
    memcpy_s(m_str+len,(GetAllocLength()-len)*sizeof(TCHAR),pszSrc,nLength*sizeof(TCHAR));
    m_str[newlen]=_T('\0');
    TEST_HEAP_NOW;
}

void FXString::Append(LPCTSTR pszSrc)
{
    if ((!pszSrc) || (_tcslen(pszSrc)==0)) return;
    FXSIZE newlen=GetLength()+_tcslen(pszSrc);
    _alloc(newlen);
    _tcscat_s(m_str,GetAllocLength(),pszSrc);
    TEST_HEAP_NOW;
}

void FXString::AppendChar(WCHAR ch)
{
    FXSIZE len=GetLength();
    _alloc(len+1);
    #ifdef _UNICODE
        m_str[len] = ch;
    #else
        m_str[len] = unicode2ansi(ch);
    #endif
    m_str[len+1] = _T('\0');
    TEST_HEAP_NOW;
}

void FXString::AppendChar(CHAR ch)
{
    FXSIZE len=GetLength();
    _alloc(len+1);
    #ifdef _UNICODE
        m_str[len] = ansi2unicode(ch);
    #else
        m_str[len] = ch;
    #endif
    m_str[len+1] = _T('\0');
    TEST_HEAP_NOW;
}

const FXString& FXString::operator+=(const FXString& strSrc) 
{
    Append(strSrc);
    return *this;
}

const FXString& FXString::operator+=(WCHAR ch) 
{
    AppendChar(ch);
    return *this;
}

const FXString& FXString::operator+=(CHAR ch) 
{
    AppendChar(ch);
    return *this;
}

const FXString& FXString::operator+=(LPCTSTR pszSrc) 
{
    Append(pszSrc);
    return *this;
}

FXString operator+(const FXString& string1, const FXString& string2) 
{
   FXString str(string1); 
   return (str += string2);
}

FXString operator+(const FXString& string, WCHAR ch) 
{
   FXString str(string); return (str += ch);
}

FXString operator+(WCHAR ch, const FXString& string) 
{
   FXString str(ch); 
   return (str += string);
}

FXString operator+(const FXString& string, char ch) 
{
   FXString str(string); 
   return (str += ch);
}

FXString operator+(char ch, const FXString& string) 
{
   FXString str(ch); 
   return (str += string);
}

FXString operator+(const FXString& string, LPCTSTR lpsz) 
{
   FXString str(string); 
   return (str += lpsz);
}

FXString operator+(LPCTSTR lpsz, const FXString& string) 
{
   FXString str(lpsz); 
   return (str += string);
}

FXString FXString::Mid(FXSIZE nFirst, FXSIZE nCount) const 
{
    FXString str;
    if ((nFirst+nCount)>GetLength()) nCount=GetLength()-nFirst;
    if (nCount>0)
    {
        str._alloc(nCount);
        memcpy_s(str.m_str,str.GetAllocLength()*sizeof(TCHAR),m_str+nFirst,nCount*sizeof(TCHAR));
        str.m_str[nCount]=_T('\0');
    }
    TEST_HEAP_NOW;
    return str;
}

FXString FXString::Mid(FXSIZE nFirst) const 
{
    FXString str;
    FXSIZE nCount=GetLength()-nFirst;
    if (nCount>0)
    {
        str._alloc(nCount);
        memcpy_s(str.m_str,str.GetAllocLength()*sizeof(TCHAR),m_str+nFirst,nCount*sizeof(TCHAR));
        str.m_str[nCount]=_T('\0');
    }
    TEST_HEAP_NOW;
    return str;
}

FXString FXString::Left(FXSIZE nCount) const 
{
    FXString str;
    if (nCount>GetLength()) nCount=GetLength();
    if (nCount>0)
    {
        str._alloc(nCount);
        memcpy_s(str.m_str,str.GetAllocLength()*sizeof(TCHAR),m_str,nCount*sizeof(TCHAR));
        str.m_str[nCount]=_T('\0');
    }
    TEST_HEAP_NOW;
    return str;
}

FXString FXString::Right(FXSIZE nCount) const 
{
    FXString str;
    FXSIZE nFirst=GetLength()-nCount;
    if (nFirst<0) { nFirst=0; nCount=GetLength(); }
    if (nCount>0)
    {
        str._alloc(nCount);
        memcpy_s(str.m_str,str.GetAllocLength()*sizeof(TCHAR),m_str+nFirst,nCount*sizeof(TCHAR));
        str.m_str[nCount]=_T('\0');
    }
    TEST_HEAP_NOW;
    return str;
}

FXString FXString::SpanIncluding(LPCTSTR lpszCharSet) const 
{
   FXSIZE i=0,j=0;
   while(i<GetLength())
   {
       if(_tcschr(lpszCharSet, m_str[i]) != NULL) 
           j++;
       else 
           break; 
       i++;
   }
   TEST_HEAP_NOW;
   return Left(j);
}

FXString FXString::SpanExcluding(LPCTSTR lpszCharSet) const 
{
   FXSIZE i=0,j=0;
   while(i<GetLength())
   {
       if(_tcschr(lpszCharSet, m_str[i]) == NULL) 
           j++;
       else 
           break; 
       i++;
   }
   TEST_HEAP_NOW;
   return Left(j);
}

FXString& FXString::TrimRight(TCHAR chTarget) 
{
    FXSIZE i=GetLength()-1;
    while(i>=0)
    {
        if (m_str[i] != chTarget) 
            break;
        i--;
    }
    if (i>=0)
        m_str[i+1]=_T('\0');
    else 
        _erase();
    TEST_HEAP_NOW;
    return (*this);
}

FXString& FXString::TrimRight(LPCTSTR lpszTargets) 
{
    FXSIZE i=GetLength()-1;
    while(i>=0)
    {
        if (_tcschr(lpszTargets, m_str[i]) == NULL) 
            break;
        i--;
    }
    if (i>=0)
        m_str[i+1]=_T('\0');
    else 
        _erase();
    TEST_HEAP_NOW;
    return (*this);
}

FXString& FXString::TrimLeft(TCHAR chTarget) 
{
    FXSIZE i=0;
    FXSIZE len=GetLength();
    while(i<len) 
    {
        if (GetAt(i) != chTarget) 
            break;
        i++;
    }
    if (i<len)
    {
        memmove_s(m_str,GetAllocLength()*sizeof(TCHAR),m_str+i,(len-i+1)*sizeof(TCHAR));
    }
    else
        _erase();
    TEST_HEAP_NOW;
    return (*this);
}

FXString& FXString::TrimLeft(LPCTSTR lpszTargets) 
{
    FXSIZE i=0;
    FXSIZE len=GetLength();
    while(i<len) 
    {
        if (_tcschr(lpszTargets, m_str[i]) == NULL)
            break;
        i++;
    }
    if (i<len)
    {
        memmove_s(m_str,GetAllocLength()*sizeof(TCHAR),m_str+i,(len-i+1)*sizeof(TCHAR));
    }
    else
        _erase();
    TEST_HEAP_NOW;
    return (*this);
}

FXString& FXString::TrimRight() 
{
   TrimRight(_T(' '));
   return (*this);
}

FXString& FXString::TrimLeft () 
{
   TrimLeft (_T(' '));
   return (*this);
}

FXString& FXString::Trim(TCHAR chTarget)
{
   TrimLeft (chTarget);
   TrimRight(chTarget);
   return (*this);
}

FXString& FXString::Trim(LPCTSTR pszTargets)
{
   TrimLeft (pszTargets);
   TrimRight(pszTargets);
   return (*this);
}

FXString& FXString::Trim() 
{
   TrimLeft (_T(' '));
   TrimRight(_T(' '));
   return (*this);
}

void FXString::Truncate(FXSIZE nNewLength)
{
    if ((nNewLength>=0) && (nNewLength<GetLength()))
    {
        m_str[nNewLength]=_T('\0');
    }
}

FXString& FXString::MakeUpper() 
{
    ::CharUpper(m_str);
    TEST_HEAP_NOW;
    return (*this);
}

FXString& FXString::MakeLower() 
{
    ::CharLower(m_str);
    TEST_HEAP_NOW;
    return (*this);
}

FXString& FXString::MakeReverse() 
{
    if (GetLength()==0) return (*this);
    _tcsrev(m_str);
    TEST_HEAP_NOW;
    return (*this);
}

FXSIZE FXString::Find(TCHAR ch) const 
{
    TCHAR *chptr=_tcschr(m_str,ch);
    if (chptr==NULL)
        return -1;
    return (chptr - m_str);
}

FXSIZE FXString::ReverseFind(TCHAR ch) const 
{
    if (GetLength()==0) return -1;
    TCHAR *chptr=_tcsrchr(m_str,ch);
    if (chptr==NULL)
        return -1;
    return (chptr - m_str);
}

FXSIZE FXString::Find(TCHAR ch, FXSIZE nStart) const 
{
    if (nStart>=GetLength()) return -1;
    TCHAR *chptr=_tcschr(m_str+nStart,ch);
    if (chptr==NULL)
        return -1;
    return (chptr - m_str);
}

FXSIZE FXString::FindOneOf(LPCTSTR lpszCharSet) const 
{
    FXSIZE i=0,len=GetLength();
    while(i<len)
    {
        if (_tcschr(lpszCharSet, m_str[i]) != NULL)
            return i;
        i++;
    }
    return (-1);
}

FXSIZE FXString::Find(LPCTSTR lpszSub) const 
{
    if ((GetLength()==0) || (lpszSub==NULL) || (_tcslen(lpszSub)==0)) return -1;
    TCHAR *chptr=_tcsstr(m_str,lpszSub);
    if (chptr==NULL)
        return -1;
    return (chptr - m_str);
}

FXSIZE FXString::Find(LPCTSTR lpszSub, FXSIZE nStart) const 
{
    if (nStart>=GetLength()) return -1;
    if ((GetLength()==0) || (lpszSub==NULL) || (_tcslen(lpszSub)==0)) return -1;
    TCHAR *chptr=_tcsstr(m_str+nStart,lpszSub);
    if (chptr==NULL)
        return -1;
    return (chptr - m_str);
}

FXSIZE FXString::Replace(TCHAR chOld, TCHAR chNew) 
{
  FXSIZE count = 0;
  FXSIZE i=0;
  FXSIZE len=GetLength();
  while(i<len) 
  {
     if (m_str[i] == chOld) 
     {
        m_str[i]=chNew;
        count++;
     }
     i++;
  }
  TEST_HEAP_NOW;
  return count;
}

FXSIZE FXString::Replace(LPCTSTR lpszOld, LPCTSTR lpszNew) 
{
    FXSIZE count=0;
    FXSIZE len=GetLength();
    if (len==0) return count;
    if (lpszOld==NULL) return count;
    FXSIZE lenold = _tcslen(lpszOld);
    FXSIZE lennew = (lpszNew)?_tcslen(lpszNew):0;
    // count replacement nmb
    LPTSTR sc=m_str;
    while((sc=_tcsstr(sc,lpszOld))!=NULL)
    {
        sc+=lenold;
        count++;
    }
    // do replacement if any
    if (count)
    {
        if (lenold<lennew)
            _alloc(len+count*(lennew-lenold)*sizeof(TCHAR));
        sc=m_str;
        while((sc=_tcsstr(sc,lpszOld))!=NULL)
        {
            FXSIZE lentomove=len-(sc-m_str)-lenold+1;
            memmove(sc+lennew,sc+lenold,lentomove*sizeof(TCHAR));
            memcpy(sc,lpszNew,lennew*sizeof(TCHAR));
            sc+=lennew;
            len+=(lennew-lenold);
        }
        ASSERT(m_str[len]==_T('\0'));
    }
    TEST_HEAP_NOW;
    return count;
}

FXString FXString::Tokenize(LPCTSTR pszTokens,FXSIZE& iStart) const
{
	if(iStart >= 0)
    {			
	    if( (pszTokens == NULL) || (*pszTokens == _T('\0')) )
	    {
		    if (iStart < GetLength())
		    {
			    return FXString(m_str+iStart);
		    }
	    }
	    else
	    {
		    LPTSTR pszPlace = m_str+iStart;
		    LPTSTR pszEnd   = m_str+GetLength();
		    if( pszPlace < pszEnd )
		    {
			    FXSIZE nIncluding = _tcsspn(pszPlace, pszTokens);

			    if( (pszPlace+nIncluding) < pszEnd )
			    {
				    pszPlace += nIncluding;
				    FXSIZE nExcluding = _tcscspn(pszPlace, pszTokens);
				    FXSIZE iFrom = iStart+nIncluding;
				    FXSIZE nUntil = nExcluding;
				    iStart = iFrom+nUntil+1;
				    return( Mid( iFrom, nUntil ) );
			    }
		    }
	    }
    }
	iStart = -1;
	return FXString("");
}

FXSIZE FXString::Remove(TCHAR chRemove) 
{
    FXSIZE count = 0;
    FXSIZE i=0;
    FXSIZE len=GetLength();
    while (i<len) 
    {
        if (m_str[i] == chRemove) 
        {
            memmove(m_str+i,m_str+i+1,(len-i)*sizeof(TCHAR));
            len--;
            count++;
        } 
        else 
            i++;
    }
    TEST_HEAP_NOW;
    return count;
}

FXSIZE FXString::Insert(FXSIZE nIndex, TCHAR ch) 
{
    FXSIZE len=GetLength();
    if (len==0) return len;
    if (nIndex>len) 
        nIndex=len;
    _alloc(len+1);
    memmove(m_str+nIndex+1,m_str+nIndex,(len-nIndex+1)*sizeof(TCHAR));
    m_str[nIndex]=ch;
    len++;
    TEST_HEAP_NOW;
    return len;
}

FXSIZE FXString::Insert(FXSIZE nIndex, LPCTSTR pstr) 
{
    FXSIZE len=GetLength();
    if (len==0) return len;
    if (nIndex>len) 
        nIndex=len;
    if (pstr) 
    {
        FXSIZE ilen=_tcslen(pstr);
        if (ilen!=0)
        {
            FXSIZE lentomove=len-nIndex+1;
            len+=ilen;
            _alloc(len);
            memmove(m_str+nIndex+ilen,m_str+nIndex,lentomove*sizeof(TCHAR));
            memcpy(m_str+nIndex,pstr,ilen*sizeof(TCHAR));
        }
    }
    TEST_HEAP_NOW;
    return len;
}

FXSIZE FXString::Delete(FXSIZE nIndex, FXSIZE nCount) 
{
    FXSIZE len=GetLength();
    if (len==0) return len;
    if (nIndex>=len) return len;
    if (nIndex+nCount>len) nCount=len-nIndex;
    if (nCount > 0) 
    {
        FXSIZE lentomove=len-nIndex-nCount+1;
        memmove(m_str+nIndex,m_str+nIndex+nCount,lentomove*sizeof(TCHAR));
    }
    TEST_HEAP_NOW;
    return len;
}

BOOL FXString::LoadString(HMODULE hMod, UINT nID)
{
    FXSIZE sizeBuff = 64;
    bool bRepeat;
    FXSIZE iRes = 0;
    if (!hMod) return FALSE;
    TCHAR *szFormat = NULL;
    do 
    {
        _alloc(sizeBuff*=2);
        iRes = ::LoadString(hMod, nID, m_str, (UINT)GetAllocLength());
        bRepeat = (iRes+1 >= sizeBuff);
    }while (bRepeat);
    _resetbuflen();
    TEST_HEAP_NOW;
    return (iRes > 0);
}

BOOL FXString::LoadString(UINT nID) 
{
    HMODULE hModule = NULL;
    GetParentModuleHandle(&hModule);
    return LoadString(hModule, nID);
}

void FXString::FormatV(LPCTSTR lpszFormat, va_list argList) 
{
    FXSIZE sizeBuff = 512;
    FXSIZE writen = -1;
    do 
    {
        _alloc(sizeBuff*=2);
        if (m_str)
            writen = _vsntprintf_s(m_str, GetAllocLength(), GetAllocLength()-1, lpszFormat, argList);
    } while ((writen < 0) && (m_str!=NULL));
    _resetbuflen();
    TEST_HEAP_NOW;
}

void FXString::Format(LPCTSTR lpszFormat, ...) 
{
    va_list arglist;
    va_start(arglist, lpszFormat);
    FormatV(lpszFormat, arglist);
    va_end(arglist);
}

void FXString::Format(UINT nFormatID, ...) 
{
    va_list arglist;
    va_start(arglist, nFormatID);
    FXString strLoad;
    BOOL bRes = strLoad.LoadString(nFormatID);
    if (bRes) 
    {
        FormatV(strLoad, arglist);
    }
    else 
    {
        Empty();
    }
    va_end(arglist);
    TEST_HEAP_NOW;
}

void FXString::FormatMessage(LPCTSTR lpszFormat, ...) 
{
    va_list argList;
    va_start(argList, lpszFormat);
    LPTSTR lpszTemp;
    if ((::FormatMessage(FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ALLOCATE_BUFFER,lpszFormat, 0, 0, (LPTSTR)&lpszTemp, 0, &argList) == 0) && (lpszTemp == NULL))
    {
        *this = lpszTemp;
        LocalFree(lpszTemp);
    }
    va_end(argList);
    TEST_HEAP_NOW;
}

void FXString::FormatMessage(UINT nFormatID, ...) 
{
    FXString strFormat;
    if (strFormat.LoadString(nFormatID)) 
    {
        va_list argList;
        va_start(argList, nFormatID);
        LPTSTR lpszTemp;
        if ((::FormatMessage(FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ALLOCATE_BUFFER, strFormat, 0, 0, (LPTSTR)&lpszTemp, 0, &argList) == 0) && (lpszTemp == NULL))
        {
            *this = lpszTemp;
            LocalFree(lpszTemp);
        }
        va_end(argList);
    }
    TEST_HEAP_NOW;
}

LPTSTR FXString::GetBuffer(FXSIZE nMinBufLength) 
{
    _alloc(nMinBufLength);
    return m_str;
}

LPTSTR FXString::GetBuffer() 
{
    return m_str;
}

void FXString::ReleaseBuffer(FXSIZE nNewLength) 
{
    if (nNewLength >0) 
    {
        nNewLength = (nNewLength<GetAllocLength())?nNewLength:GetAllocLength()-1;
        m_str[nNewLength]=_T('\0');
    }
    else if (nNewLength==0)
        _erase();
    FreeExtra();
    TEST_HEAP_NOW;
}

LPTSTR FXString::GetBufferSetLength(FXSIZE nNewLength) 
{ 
    return GetBuffer(nNewLength);
}

// the simplest way of locking 
LPTSTR FXString::LockBuffer()
{
    _getExt()->Locked=true;
    TEST_HEAP_NOW;
    return m_str;
}
void FXString::UnlockBuffer()
{
    _getExt()->Locked=false;
    TEST_HEAP_NOW;
}
FXSIZE FXString::GetOneOfCnt( LPCTSTR lpszCharSet ) const
{
  FXSIZE iCnt = 0 ;
  FXSIZE i = 0 , len = GetLength();
  while ( i < len )
  {
    if ( _tcschr( lpszCharSet , m_str[ i ] ) != NULL )
      iCnt++ ;
    i++;
  }
  return iCnt;
}

FXSIZE FXString::FindOpenBracket( FXSIZE iStart , FXSIZE iEnd) const
{
  if ( iEnd == -1 )
    iEnd = GetLength() ;
  for ( FXSIZE i = iStart ; i < iEnd ; i++ )
  {
    TCHAR ch = GetAt( i ) ;
    if ( ch == _T( '(' ) || ch == _T( '{' ) || ch == _T( '[' ) )
      return i ;
  }

  return -1 ;
}
FXSIZE FXString::FindClosingBracket( TCHAR Bracket , FXSIZE iPos , FXSIZE iEnd ) const
{
  int iNotClosedBrackets[ 3 ] = { 0 , 0 , 0 } ; // for ( { and [
  switch ( Bracket )
  {
  case _T( ')' ): ++iNotClosedBrackets[ 0 ] ; break ;
  case _T( '}' ): ++iNotClosedBrackets[ 1 ] ; break ;
  case _T( ']' ): ++iNotClosedBrackets[ 2 ] ; break ;
  default: ASSERT( 0 ) ;  break ;
  }
  FXSIZE iLen = GetLength() ;
  if ( iLen > iEnd && iEnd > -1 )
    iLen = iEnd ;
  for ( FXSIZE i = iPos ; i < iLen ; i++ )
  {
    TCHAR ch = FXString::GetAt( i ) ;
    switch ( ch )
    {
    case _T( '(' ): ++iNotClosedBrackets[ 0 ] ; break ;
    case _T( ')' ): --iNotClosedBrackets[ 0 ] ; break ;
    case _T( '{' ): ++iNotClosedBrackets[ 1 ] ; break ;
    case _T( '}' ): --iNotClosedBrackets[ 1 ] ; break ;
    case _T( '[' ): ++iNotClosedBrackets[ 2 ] ; break ;
    case _T( ']' ): --iNotClosedBrackets[ 2 ] ; break ;
    default: break ;
    }
    if ( !iNotClosedBrackets[ 0 ] && !iNotClosedBrackets[ 1 ] && !iNotClosedBrackets[ 2 ] )
      return i ;
  }
  return -1 ;
}
FXSIZE FXString::FindClosingBracket( FXSIZE iStart = 0 , FXSIZE iEnd ) const
{
  int iNotClosedBrackets[ 3 ] = { 0 , 0 , 0 } ; // for ( { and [
  switch ( GetAt( iStart ) )
  {
  case _T( '(' ): ++iNotClosedBrackets[ 0 ] ; break ;
  case _T( '{' ): ++iNotClosedBrackets[ 1 ] ; break ;
  case _T( '[' ): ++iNotClosedBrackets[ 2 ] ; break ;
  default: ASSERT( 0 ) ;  break ;
  }
  FXSIZE iLen = GetLength() ;
  if ( iLen > iEnd && iEnd > -1 )
    iLen = iEnd ;
  for ( FXSIZE i = iStart + 1 ; i < iLen ; i++ )
  {
    TCHAR ch = FXString::GetAt( i ) ;
    switch ( ch )
    {
    case _T( '(' ): ++iNotClosedBrackets[ 0 ] ; break ;
    case _T( ')' ): --iNotClosedBrackets[ 0 ] ; break ;
    case _T( '{' ): ++iNotClosedBrackets[ 1 ] ; break ;
    case _T( '}' ): --iNotClosedBrackets[ 1 ] ; break ;
    case _T( '[' ): ++iNotClosedBrackets[ 2 ] ; break ;
    case _T( ']' ): --iNotClosedBrackets[ 2 ] ; break ;
    default: break ;
    }
    if ( !iNotClosedBrackets[ 0 ] && !iNotClosedBrackets[ 1 ] && !iNotClosedBrackets[ 2 ] )
      return (int) i ;
  }
  return -1 ;
}

bool FXString::GetSubStringInBrackets( FXString& Result , FXSIZE iPos ) const
{
  if ( iPos >= 0 )
  {
    FXSIZE iOpenBr = FindOpenBracket( iPos ) ;
    if ( iOpenBr >= 0 )
    {
      FXSIZE iCloseBr = FindClosingBracket( iOpenBr ) ;
      if ( iCloseBr > 0 )
      {
        Result += Mid( iOpenBr + 1 , iCloseBr - iOpenBr - 1 ) ;
        return true ;
      }
    }
  }
  return false ;
}



#ifndef _UNICODE
    // ANSI <-> OEM support (convert string in place)
    // convert string from ANSI to OEM in-place
    void FXString::AnsiToOem() 
    {
       ::CharToOem(m_str, m_str);
    }
    // convert string from OEM to ANSI in-place
    void FXString::OemToAnsi() 
    {
       ::OemToChar(m_str, m_str);
    }
#endif
#ifndef _AFX_NO_BSTR_SUPPORT
    BSTR FXString::AllocSysString() const 
    {
        BSTR ret=NULL;
        WCHAR *tmps = new WCHAR[GetAllocLength()];
        #ifndef _UNICODE
            MultiByteToWideChar(CP_ACP, 0, m_str, -1, tmps, (UINT)GetAllocLength());
        #else
            wcscpy_s(tmps,GetAllocLength(),m_str);
        #endif
        ret =::SysAllocString(tmps);
        delete [] tmps;
        return ret;
    }
    BSTR FXString::SetSysString(BSTR* pbstr) const 
    {
       BSTR szRes = AllocSysString(); 
       if (pbstr) {::SysFreeString(*pbstr); *pbstr=szRes;} 
       return szRes;
    }
#endif




 