#ifndef FXPARSER_INCLUDE
#define FXPARSER_INCLUDE
// fxparser.h : header file
//
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class FXFC_EXPORT FXParser :
    public FXString
{
private:
    FXString    m_LastError;
public:
    FXParser(void);
    FXParser(LPCTSTR stringSrc);
    ~FXParser(void);
///////////////////////////////////////////
    const FXParser& operator =(LPCTSTR stringSrc);
    bool    Load(LPCTSTR fileName);                  // loads file to this
    FXSIZE     TrimSeparators(FXSIZE& pos);                 // shifts pos right while separators
    bool    GetWord(FXSIZE& pos, FXString& wrd);        // gets any elements of text terminated with separators or terminators
    bool    GetString(FXSIZE& pos, FXString& wrd);      // gets string till semicolon
    bool    GetParamString(FXSIZE& pos, FXString& wrd); // gets text block in brakets
    bool    IsEOL(FXSIZE pos);                          // check if pos points to end of line
    bool    GetElementNo(FXSIZE no, FXString& key, FXParser& param); // get no-th element and parameters in set of "element(param)" 
    bool    FindElementNo( LPCTSTR query , int no , FXParser& result ); // get no-th element with 
                                                         // name query and parameters in set of "element(param)" 
    bool    GetNextElement( LPCTSTR query , FXSIZE& iPos , FXParser& result ); // get next element with 
                                          // name query and parameters in set of "element(param)" 
    bool    GetNextElement( FXSIZE& iPos , FXString& key , FXParser& param ) ; // function gives 
                                         //  next element from position iPos 
    LPCTSTR GetErrorMessage() { return m_LastError; }
};

FXFC_EXPORT FXString fe2errmessage( FXFileException &ex ) ;
FXFC_EXPORT void skipTextInQuotas( FXSIZE& pos , FXString& org ) ;
FXFC_EXPORT bool ExtractElementName( FXParser& src , FXSIZE& pos , FXString& Name ) ;
FXFC_EXPORT FXSIZE TrimSeparatorsAndCommas( FXParser& that , FXSIZE&pos ) ;

#endif // #ifndef FXPARSER_INCLUDE