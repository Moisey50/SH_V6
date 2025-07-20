// BMPRender.h: interface for the BMPRender class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BMPRender_H__B57B82B8_FFA2_434C_BE35_3974333F91CA__INCLUDED_)
#define AFX_BMPRender_H__B57B82B8_FFA2_434C_BE35_3974333F91CA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gadgets\gadbase.h>
#include <list>

using namespace std;

enum InsertLabel
{
  NoLabel = 0 ,
  NumberFirst ,
  LabelFirst ,
  LabelFirstWithPath,
  LabelIsPath
};

enum BMPFormat
{
  Headers_And_Data = 0 ,
  StandardBMP ,
  Tiff ,
  JPG ,
  PNG ,
  NoHeaders ,
  CSV
};

enum FileNameSuffix
{
  Counter,
  TimeStamp
};

#define PROP_NAME_FILE_NAME         ("FileName")
#define PROP_NAME_FORMAT_BMP        ("FormatBMP")
#define PROP_NAME_ADD_LABEL         ("AddLabel")
#define PROP_NAME_FOLDER_PATH       ("FolderPath")
#define PROP_NAME_FILE_NAME_NO_EXT  ("FileName_NoExt")
#define PROP_NAME_ADD_FN_SUFFIX     ("AddFileNameSuffix")

#define PROPS_NAMES                 {\
  FXString(PROP_NAME_FILE_NAME), FXString(PROP_NAME_FORMAT_BMP), \
  FXString(PROP_NAME_ADD_LABEL), FXString(PROP_NAME_FOLDER_PATH),  \
  FXString(PROP_NAME_FILE_NAME_NO_EXT), FXString(PROP_NAME_ADD_FN_SUFFIX) \
}

#define DEFAULT_FOLDER_PATH         (".\\BMP")


class BMPRender : public CRenderGadget
{
private:
  FXString m_PathName;

  FXString       m_FileNameWithoutExt;
  FXString       m_FileExt;
  FileNameSuffix m_FileNameSuffix;

  FXString m_FNTemplate;
  DWORD   m_FramesCntr;
  BMPFormat   m_Standard;
  InsertLabel m_AddLabel ;
  CDuplexConnector * m_pDuplex ;
  COutputConnector * m_pOutput;
  FXLockObject m_NameLock ;

  virtual int GetDuplexCount() { return ( m_pDuplex ) ? 1 : 0; }
  virtual CDuplexConnector* GetDuplexConnector( int n ) { return ( n == 0 ) ? m_pDuplex : NULL; }
  virtual int GetOutputsCount()
  {
    return (m_pOutput) ? 1 : 0;
  }
  virtual COutputConnector* GetOutputConnector(int n) { return (n == 0) ? m_pOutput : NULL; }

  const FXString* GetKey(const FXString& cmd) const
  {
    FXString* pRes = NULL;

    static list<FXString> keys = PROPS_NAMES;

    list<FXString>::const_iterator ci = keys.begin();

    for ( ; ci!=keys.end() && cmd.Find(*ci)<0; ci++)
    {

    }

    if ( ci != keys.end() )
      pRes = (FXString*)&*ci;

    return pRes;
  }

  bool HasFileName(const FXString& fn) const;
  FXString GetTokenFileExt(const FXString& fn) const;
  FXString GetTokenFileName(const FXString& fn, bool withExt ) const ;
  FXString GetTokenFolderPath(const FXString& filePath) const;
  int GetPathDelimiterLast(const FXString& filePath) const;

  FXString CombineFileName(const FXString& coreName, FileNameSuffix counterOrTimestmpTmp) const;
  FXString CombineFullFileName(const CVideoFrame* vf, const FXString& folderPath, const FXString& coreFileName, FileNameSuffix cntrOrTimestampTempl, DWORD framesCntr, InsertLabel lblPos, const FXString& extention) const;
  FXString GetSuffixFormatStr(FileNameSuffix fns)const;
  FXString GetFileExtention(BMPFormat fileFormat)const;

  BMPRender(const BMPRender&);
  BMPRender& operator=(const BMPRender&);

protected:
  const FXString& GetFolderPath();

public:
  BMPRender();
  virtual void ShutDown();
  virtual void Render( const CDataFrame* pDataFrame );
  virtual void AsyncTransaction( CDuplexConnector* pConnector , CDataFrame* pParamFrame );

  bool    ScanSettings( FXString& text );
  bool    ScanProperties( LPCTSTR text , bool& Invalidate );
  bool    PrintProperties( FXString& text );
  
  DECLARE_RUNTIME_GADGET( BMPRender );
};

#endif // !defined(AFX_BMPRender_H__B57B82B8_FFA2_434C_BE35_3974333F91CA__INCLUDED_)
