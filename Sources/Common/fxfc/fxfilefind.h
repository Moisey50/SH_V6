#ifndef FXFILEFIND_INCLUDE
#define FXFILEFIND_INCLUDE
// fxfilefind.h 
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class FXFC_EXPORT FXFileDataInfo
{
public:
    bool        m_IsNew;
    int         m_iNumberPos;
    /// copy of WIN32_FIND_DATA
    DWORD       m_dwFileAttributes;
    FILETIME    m_ftCreationTime;
    FILETIME    m_ftLastAccessTime;
    FILETIME    m_ftLastWriteTime;
    DWORD       m_nFileSizeHigh;
    DWORD       m_nFileSizeLow;
//     FXString    m_cFileName;
//     FXString    m_cAlternateFileName;
    TCHAR       m_cFileName[MAX_PATH];
    TCHAR       m_cAlternateFileName[14];

    FXFileDataInfo();
    FXFileDataInfo(WIN32_FIND_DATA& fd);
    const FXFileDataInfo& operator=(const FXFileDataInfo& fdiSrc);
    const FXFileDataInfo& operator=(WIN32_FIND_DATA& fd);
    friend FXFC_EXPORT bool operator==(const FXFileDataInfo& fdi1, const FXFileDataInfo& fdi2);
    friend FXFC_EXPORT int CompareTimes(const FXFileDataInfo * fdi1, const FXFileDataInfo * fdi2);
    friend FXFC_EXPORT int CompareASCIINumbers(const FXFileDataInfo * fdi1, const FXFileDataInfo * fdi2);
};

class FXFC_EXPORT FXFileFind
{
protected:
    FXArray<FXFileDataInfo>* m_Files;
    FXString m_SearchRequest;
private:
    FXString m_Drive;
    FXString m_Path;
    int      m_NextPos;
    bool     m_FindNew;
public:
    FXFileFind();
    virtual ~FXFileFind();
    virtual BOOL FindFile(LPCTSTR pstrName = NULL, DWORD dwUnused = 0);
    virtual BOOL FindNextFile( );
    void Close( );
    virtual FXString GetFileName( ) const;
    virtual FXString GetFilePath( ) const;
    virtual FXString GetFileTitle() const;
    virtual FXString GetFileExt() const;
    virtual FXString GetFileURL( ) const;
    virtual BOOL GetCreationTime(FILETIME* pTimeStamp) const;
    virtual BOOL GetLastAccessTime(FILETIME* pTimeStamp) const;
    virtual BOOL GetLastWriteTime(FILETIME* pTimeStamp ) const;
    ULONGLONG GetLength( ) const;
    virtual FXString GetRoot( ) const;
    BOOL IsArchived( ) const;
    BOOL IsCompressed( ) const;
    BOOL IsDirectory( ) const;
    virtual BOOL IsDots( ) const;
    BOOL IsHidden( ) const;
    BOOL IsNormal( ) const;
    BOOL IsReadOnly( ) const;
    BOOL IsSystem( ) const;
    BOOL IsTemporary( ) const;
    virtual BOOL MatchesMask(DWORD dwMask) const;
    /// Extra functions. They have no prototipes in MFC
    virtual BOOL FindNew(); // use it instead of FindFirst to get list of only new files
    FXFileDataInfo * GetCurrentFileInfo() const
    {
      if (m_NextPos >= 0 && m_NextPos < m_Files->GetCount())
        return &(*m_Files)[m_NextPos];
      return NULL;
    }
};

enum FFIterationMode
{
  FFIM_Native = 0,
  FFIM_Time,
  FFIM_Number
};

class FXFC_EXPORT FXFileFindEx: public FXFileFind
{
  FFIterationMode m_IterationMode;
  int             m_iNumberPosition;
public:
  FXFileFindEx();
  FFIterationMode GetIterationMode() { return m_IterationMode; }
  void SetIterationMode(FFIterationMode IterMode) { m_IterationMode = IterMode; }

  virtual BOOL FindFile(LPCTSTR pstrName = NULL, FFIterationMode IterMode = FFIM_Native) ;
  //virtual BOOL FindNextFile();
  virtual FILETIME GetFileTime() const;
};

FXFC_EXPORT FXString FXGetExeFilePath() ;
FXFC_EXPORT FXString FXGetExeDirectory() ;
#endif //#ifndef FXFILEFIND_INCLUDE