#ifndef __FXPARSER2_H 
#define __FXPARSER2_H
#include <fxfc/fxfc.h>
#include "math\Intf_sup.h"

#ifndef ToHex
#define FromHex(n)	(((n) >= 'A') ? ((n) + 10 - 'A') : ((n) - '0'))
#define ToHex(n)	((BYTE) (((n) > 9) ? ((n) - 10 + 'A') : ((n) + '0')))
#endif


extern const char * pSeparators ; 

inline BYTE ConvBytesToHex( LPBYTE pSrc , DWORD iLen , FXString& Dest )
{
  LPBYTE p = pSrc ;
  BYTE CRC = 0 ;
  while ( iLen-- > 0 )
  {
    BYTE Val = *( p++ ) ;
    CRC += Val ;
    char cHex = ( char ) ToHex( Val >> 4 ) ;
    Dest += cHex ;
    cHex = ( char ) ToHex( Val & 0xf ) ;
    Dest += cHex ;
  }
  return CRC ;
}
inline BYTE ConvBytesToHexSaveCRC( LPBYTE pSrc , DWORD iLen , FXString& Dest )
{
  BYTE CRC = ConvBytesToHex( pSrc , iLen , Dest ) ;
  char cHex = ( char ) ToHex( CRC >> 4 ) ;
  Dest += cHex ;
  cHex = ( char ) ToHex( CRC & 0xf ) ;
  Dest += cHex ;
  return CRC ;
}

inline BYTE ConvHexToBytes( LPCTSTR pSrc , DWORD iLen , BYTE * pDest , DWORD& iDestLen )
{
  LPCTSTR p = pSrc ;
  LPCTSTR pEnd = pSrc + iLen ;
  BYTE * pOut = pDest ;
  BYTE * pDestEnd = pDest + iDestLen ;
  BYTE CRC = 0 , PrevCRC = 0 ;
  while ( (p < pEnd) && ( pOut < pDestEnd) )
  {
    BYTE Val = *( p++ ) ;
    BYTE OutByte = FromHex( Val ) ;
    if ( p >= pEnd )
      break ;
    Val = *( p++ ) ;
    OutByte = ( OutByte << 4 ) + FromHex( Val ) ;
    *( pOut++ ) = OutByte ;
    PrevCRC = CRC ;
    CRC += OutByte ;
  }
  iDestLen = (DWORD)(pOut - pDest) ;
  return CRC ;
}


inline void ConvBinStringToView( LPCTSTR pSrc , FXString& Dest )
{
  int iLen = (int) strlen( pSrc ) ;
  LPCTSTR p = pSrc ;
  char val = *p ;
  while ( val != 0 )
  {
    if ( val >= 0x20 )
      Dest += val ;
    else
    {
      switch ( val )
      {
        case '\t': Dest += "\\t" ; break ;
        case '\v': Dest += "\\v" ; break ;
        case '\f': Dest += "\\f" ; break ;
        case '\n': Dest += "\\n" ; Dest += '\n' ; break ; // new string will be created
        case '\r': Dest += "\\r" ; break ;
        default:
        {
          Dest += "\\x" ;
          Dest += (char)ToHex( val >> 4 ) ;
          Dest += (char)ToHex( val & 0xf ) ;
        }
        break ;
      }
    }
    val = *(++p) ;
  }
}

inline void ConvViewStringToBin( LPCTSTR pSrc , FXString& Dest )
{
  int iLen = (int) strlen( pSrc ) ;
  LPCTSTR p = pSrc ;
  char val = *p ;
  while ( *p )
  {
    if ( val != '\\' )
    {
      if ( val != '\n' && val != '\r')
        Dest += val ;
    }
    else if ( val = *( ++p ) )
    {
      val = toupper( val ) ;
      switch ( val )
      {
        case 'B': Dest += ' ' ; break ;
        case 'T': Dest += '\t' ; break ;
        case 'V': Dest += '\v' ; break ;
        case 'F': Dest += '\f' ; break ;
        case 'N': Dest += '\n' ; break ;
        case 'R': Dest += '\r' ; break ;
        case 'X':
        {
          char Res = 0 ;
          if ( (val = *( ++p )) && isxdigit( val = toupper(val) ) )
          {
            Res = FromHex( val );
            if ( ( val = *( ++p ) ) && isxdigit( val ) )
            {
              Res = ( Res << 4 ) + FromHex( val = toupper( val ) ) ;
              Dest += Res ;
            }
            else
            {
              Dest += Res ;
              p-- ; // return to previous, below we will go to next
              break ;
            }
          }
          else
          {
            // nothing will be written, not hex digits
            p-- ; // return to previous, below we will go to next
            break ;
          }
        }
        break ;
      }
    }
    val = *( ++p ) ; // go to next symbol
  }
}
inline bool StrToCmplx( LPCTSTR pStr , cmplx& Result )
{
  while ( _tcschr( _T( " \t([{=" ) , *pStr ) )
    pStr++ ;
  return (sscanf_s( pStr , _T( "%lg,%lg" ) , &Result._Val[ 0 ] , &Result._Val[ 1 ] ) == 2) ;
}

inline bool StrToIPt( LPCTSTR pStr , CPoint& Result )
{
  while ( _tcschr( _T( " \t([{=" ) , *pStr ) )
    pStr++ ;
  return (sscanf_s( pStr , _T( "%d,%d" ) , &Result.x , &Result.y ) == 2) ;
}

inline int StrToCoord3( LPCTSTR pStr , CCoord3& Result )
{
  while ( _tcschr( _T( " \t([{=" ) , *pStr ) )
    pStr++ ;
  int iNScanned = sscanf_s( pStr , _T( "%lg,%lg,%lg" ) ,
    &Result.m_x , &Result.m_y , &Result.m_z ) ;
  if ( iNScanned < 3 )
  {
    Result.m_z = 0. ;
    if ( iNScanned < 2 )
    {
      Result.m_y = 0. ;
      if ( iNScanned < 1 )
        Result.m_x = 0. ;
    }
  }
  return iNScanned ;
}

inline bool StrToDbl( LPCTSTR pStr , double& Result )
{
  return (sscanf_s( pStr , _T( "%lg" ) , &Result ) == 1) ;
}

class FXParser2: public FXParser
{
public:
  FXParser2() {}

  FXParser2(const FXString& stringSrc) 
  {
    SetString(stringSrc);
  }
  const FXString& operator =( const FXString& stringSrc )
  {
    SetString(stringSrc);
    return *this;
  }
  bool FindElement(LPCTSTR query, FXParser2& result)
  {
    bool retVal=false;
    result.Empty();
    int pos = (int) this->Find(query);
    if (pos<0) return retVal;
    pos+= (int)_tcsclen(query);
    int brkcnt=0;
    while (pos<this->GetLength())
    {
      if (strchr(SEPARATORS,(*this)[pos])!=NULL) { pos++; continue; }
      if ((*this)[pos]=='(') { brkcnt++; if (brkcnt==1) { pos++; continue;} }
      if ((*this)[pos]==')') brkcnt--;
      if (brkcnt==0) { retVal=true; break; }
      result+=(*this)[pos];
      pos++;
    }
    return retVal;
  }
  bool GetParam(int& pos, FXString& wrd, char separator)
  {
    bool retVal=false;

    wrd.Empty();
    while (pos<this->GetLength())
    {
      if (strchr(SEPARATORS,(*this)[pos])!=NULL) { pos++; continue; }
      if ((*this)[pos]==separator) { pos++; retVal=true; break; }
      if (strchr(BRAKETS,(*this)[pos])!=NULL) { retVal=true; break; }
      wrd+=(*this)[pos]; retVal=true;
      pos++;
    }
    return retVal;
  };

  bool GetElementNo(
    int no, FXString& key, FXParser& param ,
    int& iBegin , int& iEnd )
  {
    bool retVal=false;
    FXSIZE pos = iBegin;
    int brkcnt=0;
    iBegin = iEnd = -1 ;
    for (int i=0; i<=no; i++)
    {
      key.Empty(); 
      param.Empty(); 
      retVal=false;
      if (!GetParamString(pos,key)) 
        return false;
      while (pos<this->GetLength())
      {
        iEnd = (int) pos ;
        if (strchr(SEPARATORS,(*this)[pos])!=NULL) 
        {
          pos++; 
          continue; 
        }
        if ((*this)[pos]=='(') 
        { 
          brkcnt++; 
          if (brkcnt==1) 
          {
            pos++; 
            continue;
          } 
        }
        if ((*this)[pos]==')')
          brkcnt--;
        if (brkcnt<0)
        { 
          retVal=false;
          break; 
        }
        if (brkcnt==0) 
        { 
          retVal=true; 
          break; 
        }
        if ( param.IsEmpty() )
          iBegin = (int) pos ;
        param+=(*this)[pos];
        pos++;
      } 
      pos++;
      TrimSeparators(pos);
      if ((pos<this->GetLength()) && ((*this)[pos]==',')) 
        pos++;
      TrimSeparators(pos);
    }
    return retVal; 
  }
  int FindCRLFAndRemoveMultiCRLF( int& iNRemoved , int iStart = 0 )
  {
    int iPos = (int) Find( "\r\n" , iStart ) ;
    while ( ((iPos + 3) < (int) GetLength()) &&
      (*this)[iPos+2] == _T('\r')  && (*this)[iPos+3] == _T('\n') )
    {
      Delete( iPos + 2 , 2 ) ;
      iNRemoved++ ;
    }
    return iPos ;
  }
  int FindWord( const FXString& Word , int& iPos )
  {
    int iWordPos ;
    while ( (iWordPos = (int) Find( Word , iPos )) >= 0 )
    {
      iPos = iWordPos + (int) Word.GetLength() ;
      if ( strchr( pSeparators , (*this)[iWordPos + (int) Word.GetLength()]) )
      {
        if ( iWordPos > 0 )
        {
          if ( !strchr( pSeparators , (*this)[iWordPos - 1]) )
            continue ;  // symbol before is not separator
        }
        return iWordPos ;
      }
      // here we are if symbol after is not separator
    }
    return -1 ;
  }
  int FindWord( LPCTSTR pWord , int& iPos )
  {
    FXString Word( pWord ) ;
    return FindWord( Word , iPos ) ;
  }
  int FindAndReplaceWord( const FXString& OldWord , const FXString& NewWord , int& iPos )
  {
    int iWordPos = FindWord( OldWord , iPos ) ;
    if ( iWordPos >= 0 ) //word is found
    {
      Delete( iWordPos , OldWord.GetLength() ) ;
      Insert( iWordPos , NewWord ) ;
      return ( iPos = iWordPos + (int) NewWord.GetLength() ) ;
    }
    return -1 ;
  }
  int FindAndReplaceWord( LPCTSTR OldWord , LPCTSTR NewWord , int& iPos )
  {
    LPCTSTR pString = (LPCTSTR)this ;
    const TCHAR * pFound = _tcsstr( (LPCTSTR)this , OldWord ) ;
    if ( pFound )
    {
      Delete( pFound - pString , _tcslen(OldWord) ) ;
      Insert( pFound - pString , NewWord ) ;
      return ( iPos = (int) (pFound - pString +  _tcslen(NewWord)) ) ;
    }
    return -1 ;
  }
  bool StrToCmplx( cmplx& Result )
  {
    LPCTSTR pStr = ((FXString*)this)->GetString() ;
    return ::StrToCmplx( pStr , Result ) ;
  }

  bool StrToIPt( CPoint& Result )
  {
    LPCTSTR pStr = ((FXString*) this)->GetString() ;
    return ::StrToIPt( pStr , Result ) ;
  }
  bool StrToDbl( LPCTSTR pStr , double& Result )
  {
    if ( sscanf_s( pStr , _T("%lg") , &Result ) == 1 )
      return true ;
    return false ;
  }
  bool StrToDbl( double& Result )
  {
    if ( sscanf_s( (LPCTSTR)this , _T("%lg") , &Result ) == 1 )
      return true ;
    return false ;
  }
  bool StrToCoord3( CCoord3& Result )
  {
    return ::StrToCoord3( (LPCTSTR)this , Result ) == 3 ;
  }
  
  void CmplxToStr( cmplx& Val )
  {
    Format( _T("%g,%g") , Val.real() , Val.imag() ) ;
  }
  void Coord3ToStr( CCoord3& Val , LPCTSTR pFormat = NULL )
  {
    Format( pFormat ? pFormat : _T( "%g,%g,%g" ) ,
      Val.m_x , Val.m_y , Val.m_z ) ;
  }
};

#endif //__FXPARSER2_H