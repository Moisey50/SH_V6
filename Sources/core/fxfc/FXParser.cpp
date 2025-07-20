#include "StdAfx.h"
#include <fxfc\fxfc.h>

#undef SEPARATORS
#undef BRAKETS
#undef TERMINATORS
#undef NEWLINE

#define FXPARSER_SEPARATORS " \t\n\r"
#define FXPARSER_BRAKETS    "()"
#define FXPARSER_TERMINATORS ",;()"
#define FXPARSER_COMMAS     ",;"
#define FXPARSER_NEWLINE    "\r\n"
#define FXPARSER_ENDSEPARATORS " \t\n\r,;"

FXString fe2errmessage(FXFileException &ex)
{
    FXString retV;
    ex.GetErrorMessage(retV.GetBufferSetLength(1024),1024);
    return retV;
}

void skipTextInQuotas(FXSIZE& pos, FXString& org)
{
	if (org[pos] != _T('\"'))return;
	while (org[++pos])
	{
		if ((org[pos] == _T('\"')) && (org[pos-1] != _T('\\')))
			break;
	}
    ++pos;
	return;
}

bool ExtractElementName(FXParser& src, FXSIZE& pos, FXString& Name)
{
    src.TrimSeparators(pos);
    Name.Empty();
    FXSIZE b=src.Find(_T('('),pos);
    if (b<0) return false;
    Name=src.Mid(pos,b-pos);
    pos=b;
    return true;
}

FXSIZE TrimSeparatorsAndCommas(FXParser& that, FXSIZE&pos)
{
    FXSIZE retV=0;
    FXSIZE len=that.GetLength();
    while ( (pos<len) && (strchr(FXPARSER_SEPARATORS FXPARSER_COMMAS,that[pos])!=NULL)) 
    {
        retV++;
        pos++;
    }
    return retV;
}

FXParser::FXParser(void)
{
}

FXParser::FXParser(LPCTSTR stringSrc)
{
    SetString(stringSrc);
}

FXParser::~FXParser(void)
{
}

const FXParser& FXParser::operator =( LPCTSTR stringSrc )
{
    SetString(stringSrc);
    return *this;
}

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

bool FXParser::Load(LPCTSTR fileName)
{
	bool res = false;
	string line, lines;
	LPTSTR pBuffer = NULL;

	ostringstream ofs;
	ifstream myfile(fileName);
	if (!myfile.is_open())
		m_LastError.Format(_T("Can't open file '%s'!"), fileName);
	else
	{
		while (getline(myfile, line))
		{
			ofs << line << std::endl;
		}

		lines = ofs.str();
		myfile.close();
		FXSIZE lengthExpect = lines.length() + 5 ;
		pBuffer = GetBufferSetLength(lengthExpect);

		if (!pBuffer)
			m_LastError.Format(_T("Can't allocate buffer for file reading ('%s')"), fileName);
		else
		{
			strncat_s(pBuffer, lengthExpect , lines.c_str(), lengthExpect);
			ReleaseBuffer();
			FXSIZE lengthAct = GetLength();
			if (lengthAct != lines.length())
				m_LastError.Format(_T("Unexpected error during file reading. CAUSE: Actual text length (%d) does not equal to expected one (%d)!"), lengthAct, lengthExpect);
			res = true;
		}
	}
	return res;
	
	/*
	FXFile file;
    FXFileException e;
	if (!file.Open(fileName, FXFile::modeRead,&e))
    {
        m_LastError.Format(_T("Can't open file %s, the reason: '%s'"),fileName,fe2errmessage(e));
		return false;
    }
	ULONGLONG length = file.GetLength();
    if (length==0)
    {
         m_LastError.Format("File %s is empty",fileName);
         file.Close();
         return false;
    }
	LPTSTR pBuffer = GetBufferSetLength((int)length); //(LPTSTR)realloc(m_pBuffer, (DWORD)(length + 1)*sizeof(_TCHAR));
	if (!pBuffer)
    {
        m_LastError.Format(_T("Can't allocate buffer for file reading"));
        file.Close();
		return false;
    }
	if (file.Read(pBuffer, (DWORD)length) != length)
    {
        m_LastError.Format(_T("Error during file reading."));
		return false;
    }
	file.Close();
	pBuffer[length] = _T('\0');
    ReleaseBuffer();
	if (GetLength() != length)
    {
        m_LastError.Format(_T("Unexpected error during file reading."));
		return false;
    }
	return true;
	*/
}

bool FXParser::IsEOL(FXSIZE pos)
{
    return (GetAt(pos)==_T('\0'));
}

FXSIZE FXParser::TrimSeparators(FXSIZE&pos)
{
    FXSIZE retV=0;
    FXSIZE len=this->GetLength();
    while ( (pos<len) && (strchr(FXPARSER_SEPARATORS,(*this)[pos])!=NULL)) 
    {
        retV++;
        pos++;
    }
    return retV;
}

bool FXParser::GetWord(FXSIZE& pos, FXString& wrd)
{
    TrimSeparatorsAndCommas(*this, pos);
    wrd.Empty();
    FXSIZE len=GetLength();
    while (pos<len)
    {
        if (strchr(FXPARSER_SEPARATORS,(*this)[pos])!=NULL)  break;
        if (strchr(FXPARSER_TERMINATORS,(*this)[pos])!=NULL) break;
        wrd+=(*this)[pos];
        pos++;
    }
    return (wrd.GetLength()!=0);
}

bool FXParser::GetString(FXSIZE& pos, FXString& wrd)
{
    bool retVal=false;

    wrd.Empty();
    FXSIZE len=GetLength();
    while (pos<len)
    {
        if (';'==(*this)[pos])
        { 
            wrd+=(*this)[pos];
            pos++;
            retVal=true; break; 
        }
        wrd+=(*this)[pos];
        pos++;
    }
    wrd.TrimLeft(FXPARSER_SEPARATORS);
    wrd.TrimRight(FXPARSER_SEPARATORS);
    return retVal;
}

bool FXParser::GetParamString(FXSIZE& pos, FXString& wrd)
{
    bool retVal=true;
    TrimSeparators(pos);
    wrd.Empty();
    FXSIZE brkcnt=0;
    if (GetAt(pos)!=FXPARSER_BRAKETS[0]) return false;
    brkcnt++; pos++;
    FXSIZE len=GetLength();
    while (pos<len)
    {
/*        if (GetAt(pos)==_T('\"')) 
            skipTextInQuotas(pos,*this);
        else */if (GetAt(pos)==FXPARSER_BRAKETS[0]) 
            brkcnt++;
        else if (GetAt(pos)==FXPARSER_BRAKETS[1]) 
            brkcnt--;
        if (brkcnt==0) 
        {
            pos++;
            break;
        }
        wrd+=(*this)[pos];
        pos++;
    }
    wrd.TrimLeft(FXPARSER_SEPARATORS);
    wrd.TrimRight(FXPARSER_SEPARATORS);
    retVal&=(wrd.GetLength()!=0);
    return retVal;
}

 // function passes n-1 elements and gives element #n
bool FXParser::GetElementNo( FXSIZE no , FXString& key , FXParser& param )
{
  FXSIZE pos = 0;
  for ( FXSIZE i = 0; i <= no; i++ )
  {
    key.Empty(); param.Empty();
    if ( !ExtractElementName( *this , pos , key ) ) return false;
    if ( !GetParamString( pos , param ) ) return false;
    TrimSeparators( pos );
    if ( _tcschr( FXPARSER_COMMAS , GetAt( pos ) ) != NULL )
      pos++;
  }
  return true;
}
 // function gives next element from position iPos
bool FXParser::GetNextElement( FXSIZE& pos , FXString& key , FXParser& param )
{
  //int pos = iPos ;
  key.Empty(); param.Empty();
  if ( !ExtractElementName( *this , pos , key ) ) 
    return false;
  if ( !GetParamString( pos , param ) ) 
    return false;
  TrimSeparators( pos );
  if ( _tcschr( FXPARSER_COMMAS , GetAt( pos ) ) != NULL )
    pos++;
  //iPos = pos ;
  return true;
}

bool FXParser::FindElementNo( LPCTSTR query , int no , FXParser& result )
{
  result.Empty();
  FXSIZE pos = 0;
  FXSIZE i = 0;
  FXString Element;
  FXParser Param;

  while ( ExtractElementName( *this , pos , Element ) )
  {
    if ( !GetParamString( pos , Param ) )
      return false;
    if ( Element == query )
    {
      if ( i == no )
      {
        result = Param;
        break;
      }
      i++;
    }
  }
  return (result.GetLength() != 0);
}
bool FXParser::GetNextElement( LPCTSTR query , FXSIZE& iPos , FXParser& result )
{
  result.Empty();
  FXSIZE pos = iPos;
  FXSIZE iLen = 0;
  FXString Element;
  FXParser Param;

  while ( ExtractElementName( *this , pos , Element ) )
  {
    if ( !GetParamString( pos , Param ) )
      return false;
    if ( Element == query )
    {
        result = Param;
        iLen = result.GetLength() ;
        iPos += iLen + strlen( query ) + 2 ;
        if ( iPos < GetLength() - 1 )
        {
          if ( _tcschr( FXPARSER_ENDSEPARATORS , GetAt( iPos ) ) != NULL )
            iPos++;
        }
        else
          iPos = -1 ;
        break;
    }
  }
  return (iLen != 0);
}
