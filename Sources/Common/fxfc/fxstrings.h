#ifndef FXSTRINGS_INCLUDE
#define FXSTRINGS_INCLUDE
// FXStrings.h : header file
//
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// EOL for output in text file
#define EOL _T("\n")
// EOL for output in binary file
#define EOLB _T("\r\n")

//#pragma pack(push, 1)
typedef struct _tagfxsExt
{
    FXSIZE Length;
    volatile bool Locked;
}fxsExt,*pfxsExt;
//#pragma pack(pop)

class FXFC_EXPORT FXString 
{
private:
    LPTSTR m_str;
protected:
    pfxsExt _getExt() const;
    bool    _islocked();
    FXSIZE   _buflen();
    bool    _alloc(FXSIZE size);
    bool    _erase();
    bool    _resetbuflen();
    void _copy(LPCSTR  lpsz);
    void _copy(LPCWSTR lpsz);
public:
    void    Preallocate(FXSIZE nLength);
    FXSIZE     GetAllocLength( ) const;
    FXSIZE     GetLength() const;
    FXSIZE     size() const { return GetLength(); };
    FXSIZE     Size() const { return GetLength(); };
    BOOL    IsEmpty() const;
    void    Empty();
    void    FreeExtra();
    LPCTSTR GetString( ) const;
    operator LPCTSTR() const;
///////////////////////////////////////////////////////////////
    FXString();
    FXString(const FXString& stringSrc);
    FXString(CHAR ansi, FXSIZE nRepeat = 1);
    FXString(WCHAR unicode, FXSIZE nRepeat = 1);
    FXString(LPCSTR  lpsz);
    FXString(LPCWSTR lpsz);
    FXString(LPCSTR lpsz, FXSIZE nLength);
    FXString(LPCWSTR lpsz, FXSIZE nLength);
    FXString(const unsigned char* lpsz);
    ~FXString();

    TCHAR GetAt(FXSIZE nIndex) const;
    TCHAR operator[](FXSIZE nIndex) const;
    void SetAt(FXSIZE nIndex, TCHAR ch);
    int Compare(LPCTSTR lpsz) const;
    int CompareNoCase(LPCTSTR lpsz) const;
    int Collate      (LPCTSTR lpsz) const;
    int CollateNoCase(LPCTSTR lpsz) const;
    const FXString& operator=(const FXString& stringSrc);
    const FXString& operator=(WCHAR ch);
    const FXString& operator=(CHAR ch);
    const FXString& operator=(LPCSTR  lpsz);
    const FXString& operator=(LPCWSTR lpsz);
    const FXString& operator=(const unsigned char* lpsz);
    void SetString(LPCTSTR pszSrc);
    void Append(const FXString& strSrc);
    void Append(LPCTSTR pszSrc,FXSIZE nLength);
    void Append(LPCTSTR pszSrc);
    void AppendChar(WCHAR ch);
    void AppendChar(CHAR ch);
    const FXString& operator+=(const FXString& strSrc);
    const FXString& operator+=(WCHAR ch);
    const FXString& operator+=(CHAR ch);
    const FXString& operator+=(LPCTSTR pszSrc);
    FXString Mid(FXSIZE nFirst, FXSIZE nCount) const;
    FXString Mid(FXSIZE nFirst) const;
    FXString Left(FXSIZE nCount) const;
    FXString Right(FXSIZE nCount) const;
    FXString SpanIncluding(LPCTSTR lpszCharSet) const;
    FXString SpanExcluding(LPCTSTR lpszCharSet) const;
    FXString& TrimRight(TCHAR chTarget);
    FXString& TrimRight(LPCTSTR lpszTargets);
    FXString& TrimLeft(TCHAR chTarget);
    FXString& TrimLeft(LPCTSTR lpszTargets);
    FXString& TrimRight();
    FXString& TrimLeft ();
    FXString& Trim(TCHAR chTarget);
    FXString& Trim(LPCTSTR pszTargets);
    FXString& Trim();
    void Truncate(FXSIZE nNewLength);
    FXString& MakeUpper();
    FXString& MakeLower();
    FXString& MakeReverse();
    FXSIZE Find(TCHAR ch) const;
    FXSIZE ReverseFind(TCHAR ch) const;
    FXSIZE Find(TCHAR ch, FXSIZE nStart) const;
    FXSIZE FindOneOf(LPCTSTR lpszCharSet) const;
    FXSIZE Find(LPCTSTR lpszSub) const;
    FXSIZE Find(LPCTSTR lpszSub, FXSIZE nStart) const;
    FXSIZE Replace(TCHAR chOld, TCHAR chNew);
    FXSIZE Replace(LPCTSTR lpszOld, LPCTSTR lpszNew);
    FXString Tokenize(LPCTSTR pszTokens,FXSIZE& iStart) const;
    FXSIZE Remove(TCHAR chRemove);
    FXSIZE Insert(FXSIZE nIndex, TCHAR ch);
    FXSIZE Insert(FXSIZE nIndex, LPCTSTR pstr);
    FXSIZE Delete(FXSIZE nIndex, FXSIZE nCount = 1);
    BOOL LoadString(UINT nID);
    BOOL LoadString(HMODULE hMod, UINT nID); // not presents in CString
    void FormatV(LPCTSTR lpszFormat, va_list argList);
    void Format(LPCTSTR lpszFormat, ...);
    void Format(UINT nFormatID, ...);
    void FormatMessage(LPCTSTR lpszFormat, ...);
    void FormatMessage(UINT nFormatID, ...);
    LPTSTR GetBuffer(FXSIZE nMinBufLength);
    LPTSTR GetBuffer();
    void ReleaseBuffer(FXSIZE nNewLength = -1);
    LPTSTR GetBufferSetLength(FXSIZE nNewLength);
    LPTSTR LockBuffer();
    void UnlockBuffer();
    FXSIZE GetOneOfCnt( LPCTSTR lpszCharSet ) const;
    FXSIZE FindOpenBracket( FXSIZE iStart = 0 , FXSIZE iEnd = -1 ) const ;
    FXSIZE FindClosingBracket( TCHAR Bracket , FXSIZE iPos , FXSIZE iEnd = -1 ) const ;
    FXSIZE FindClosingBracket( FXSIZE iPos , FXSIZE iEnd = -1 ) const ;
    bool GetSubStringInBrackets( FXString& Result , FXSIZE iPos = 0 ) const ;

    #ifndef _UNICODE
        void AnsiToOem();
        void OemToAnsi();
    #endif
    #ifndef _AFX_NO_BSTR_SUPPORT
        BSTR AllocSysString() const;
        BSTR SetSysString(BSTR* pbstr) const;
    #endif
 //friends /////////////////////////////////////////////////////////////
    friend FXFC_EXPORT bool operator==(const FXString& str1, const FXString& str2);
    friend FXFC_EXPORT bool operator==(const FXString& str1, LPCTSTR lpsz2);
    friend FXFC_EXPORT bool operator==(LPCTSTR lpsz1, const FXString& str2);
    friend FXFC_EXPORT bool operator==(const FXString& str1, TCHAR ch2);
    friend FXFC_EXPORT bool operator==(TCHAR ch1, const FXString& str2);
    friend FXFC_EXPORT bool operator!=(const FXString& str1, const FXString& str2);
    friend FXFC_EXPORT bool operator!=(const FXString& str1, LPCTSTR lpsz2);
    friend FXFC_EXPORT bool operator!=(LPCTSTR lpsz1, const FXString& str2);
    friend FXFC_EXPORT bool operator!=(const FXString& str1, TCHAR ch2);
    friend FXFC_EXPORT bool operator!=(TCHAR ch1, const FXString& str2);
    friend FXFC_EXPORT bool operator<(const FXString& str1, const FXString& str2);
    friend FXFC_EXPORT bool operator<(const FXString& str1, LPCTSTR lpsz2);
    friend FXFC_EXPORT bool operator<(LPCTSTR lpsz1, const FXString& str2);
    friend FXFC_EXPORT bool operator<(const FXString& str1, TCHAR ch2);
    friend FXFC_EXPORT bool operator<(TCHAR ch1, const FXString& str2);
    friend FXFC_EXPORT bool operator>(const FXString& str1, const FXString& str2);
    friend FXFC_EXPORT bool operator>(const FXString& str1, LPCTSTR lpsz2);
    friend FXFC_EXPORT bool operator>(LPCTSTR lpsz1, const FXString& str2);
    friend FXFC_EXPORT bool operator>(const FXString& str1, TCHAR ch2);
    friend FXFC_EXPORT bool operator>(TCHAR ch1, const FXString& str2);
    friend FXFC_EXPORT bool operator<=(const FXString& str1, const FXString& str2);
    friend FXFC_EXPORT bool operator<=(const FXString& str1, LPCTSTR lpsz2);
    friend FXFC_EXPORT bool operator<=(LPCTSTR lpsz1, const FXString& str2);
    friend FXFC_EXPORT bool operator<=(const FXString& str1, TCHAR ch2);
    friend FXFC_EXPORT bool operator<=(TCHAR ch1, const FXString& str2);
    friend FXFC_EXPORT bool operator>=(const FXString& str1, const FXString& str2);
    friend FXFC_EXPORT bool operator>=(const FXString& str1, LPCTSTR lpsz2);
    friend FXFC_EXPORT bool operator>=(LPCTSTR lpsz1, const FXString& str2);
    friend FXFC_EXPORT bool operator>=(const FXString& str1, TCHAR ch2);
    friend FXFC_EXPORT bool operator>=(TCHAR ch1, const FXString& str2);
    friend FXFC_EXPORT FXString operator+(const FXString& string1, const FXString& string2);
    friend FXFC_EXPORT FXString operator+(const FXString& string, WCHAR ch);
    friend FXFC_EXPORT FXString operator+(WCHAR ch, const FXString& string);
    friend FXFC_EXPORT FXString operator+(const FXString& string, char ch);
    friend FXFC_EXPORT FXString operator+(char ch, const FXString& string);
    friend FXFC_EXPORT FXString operator+(const FXString& string, LPCTSTR lpsz);
    friend FXFC_EXPORT FXString operator+(LPCTSTR lpsz, const FXString& string);
};

inline int FXStringCompareAscending( const void *a , const void *b )
{
  FXString *pA = (FXString*) a;
  FXString *pB = (FXString*) b;
  return (pA->CompareNoCase( *pB ));
}

#endif //#ifndef FXSTRINGS_INCLUDE