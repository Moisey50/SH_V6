#include "stdafx.h"
#include "TxtGadgets.h"
#include "TextFormat.h"
#include <gadgets\textframe.h>
#include <gadgets\QuantityFrame.h>

#define THIS_MODULENAME "TextFormat"

#define TYPE_UNDEF  0
#define TYPE_CHAR   1
#define TYPE_DECINT 2
#define TYPE_OCTINT 3
#define TYPE_HEXINT 4
#define TYPE_UINT   5
#define TYPE_FLOAT  6
#define TIPE_STRING 7

class Parameter
{
public:
  int     type;
  void*   data;
public:
  Parameter()
  {
    type = TYPE_UNDEF;
    data = NULL;
  }
  ~Parameter()
  {
    type = TYPE_UNDEF;
    if ( data ) free( data ); data = NULL;
  }
  Parameter& operator= ( Parameter& Original )
  {
    type = Original.type;
    data = Original.data;
    Original.data = NULL;
    return *this;
  }
};

class ParamArray : public FXArray<Parameter , Parameter&>
{
public:
  ~ParamArray()
  {
    while ( GetSize() )
    {
      free( GetAt( 0 ).data ); GetAt( 0 ).data = NULL;
      RemoveAt( 0 );
    }
  }
};

__forceinline FXString SPrint( const char* Format , ParamArray* pa )
{
  FXString res;
  const char* scnr = Format;
  while ( *scnr != 0 )
  {
    if ( *scnr != '%' )
    {
      res += *scnr;
      scnr++;
    }
    else
    {
      scnr++;
      if ( *scnr == 0 )
        break;
      else if ( *scnr == '%' )
      {
        res += *scnr;
        scnr++;
        continue ;
      }
      char* eptr;
      long pID = strtol( scnr , &eptr , 10 );
      if ( scnr == eptr )
        break;
      if ( pID >= pa->GetSize() )
        break;
      scnr = eptr;
      switch ( pa->GetAt( pID ).type )
      {
        case TYPE_CHAR:
        {
          res += *((char*) pa->GetAt( pID ).data);
          break;
        }
        case TYPE_DECINT:
        {
          FXString r;
          long l = *((long*) pa->GetAt( pID ).data);
          r.Format( "%d" , l );
          res += r;
          break;
        }
        case TYPE_OCTINT:
        {
          FXString r;
          unsigned long l = *((unsigned long*) pa->GetAt( pID ).data);
          r.Format( "0%o" , l );
          res += r;
          break;
        }
        case TYPE_HEXINT:
        {
          FXString r;
          unsigned long l = *((unsigned long*) pa->GetAt( pID ).data);
          r.Format( "0x%x" , l );
          res += r;
          break;
        }
        case TYPE_UINT:
        {
          FXString r;
          unsigned long l = *((unsigned long*) pa->GetAt( pID ).data);
          r.Format( "%u" , l );
          res += r;
          break;
        }
        case TYPE_FLOAT:
        {
          FXString r;
          double d = *((double*) pa->GetAt( pID ).data);
          r.Format( "%g" , d );
          res += r;
          break;
        }
        case TIPE_STRING:
        {
          res += (char*) pa->GetAt( pID ).data;
          break;
        }
        default:
        {
          SENDERR_0( "Wrong format of OutputFormat" );
          break;
        }
      }
    }
  }
  return res;
}

__forceinline ParamArray* SScan( FXString format , const char* Input )
{
  ParamArray* res = new ParamArray;
  const char* scnr = Input;
  for ( int i = 0; i < format.GetLength(); i++ )
  {
    char a = format[ i ];
    if ( a != '%' )
    {
      char l = (isupper( a )) ? _tolower( a ) : a;
      char u = (isupper( a )) ? a : _toupper( a );
      if ( (l == *scnr) || (u == *scnr) )
      {
        scnr++;
        continue;
      }
      delete res;
      res = NULL;
      break;
    }
    else
    {
      i++;
      if ( i >= format.GetLength() )
      {
        SENDERR_0( "Wrong format of InputFormat" );
        delete res;
        res = NULL;
        break;
      }
      char b = format[ i ];
      char c = ((i + 1) >= format.GetLength()) ? 0 : format[ i + 1 ];
      char d = ((i + 2) >= format.GetLength()) ? 0 : format[ i + 2 ];
      switch ( b )
      {
        case 'c':
        case 'C':
        {
          Parameter p;
          p.type = TYPE_CHAR;
          p.data = malloc( sizeof( char ) );
          *((char*) p.data) = *scnr;
          scnr++;
          res->Add( p );
          break;
        }
        case 'd':
        {
          char* eptr;
          Parameter p;
          p.type = TYPE_DECINT;
          p.data = malloc( sizeof( long ) );
          *((long*) p.data) = strtol( scnr , &eptr , 10 );
          scnr = eptr;
          res->Add( p );
          break;
        }
        case 'i':
        {
          char* eptr;
          int base = 10;
          if ( c == '0' )
            base = 8;
          if ( (d == 'x') || (d == 'X') )
            base = 16;
          Parameter p;
          p.type = TYPE_DECINT;
          p.data = malloc( sizeof( long ) );
          *((long*) p.data) = strtol( scnr , &eptr , base );
          scnr = eptr;
          res->Add( p );
          break;
        }
        case 'o':
        {
          char* eptr;
          Parameter p;
          p.type = TYPE_OCTINT;
          p.data = malloc( sizeof( unsigned long ) );
          *((unsigned long*) p.data) = strtoul( scnr , &eptr , 8 );
          scnr = eptr;
          res->Add( p );
          break;
        }
        case 'u':
        {
          char* eptr;
          Parameter p;
          p.type = TYPE_UINT;
          p.data = malloc( sizeof( unsigned long ) );
          *((unsigned long*) p.data) = strtoul( scnr , &eptr , 10 );
          scnr = eptr;
          res->Add( p );
          break;
        }
        case 'x':
        {
          char* eptr;
          Parameter p;
          p.type = TYPE_HEXINT;
          p.data = malloc( sizeof( unsigned long ) );
          *((unsigned long*) p.data) = strtoul( scnr , &eptr , 16 );
          scnr = eptr;
          res->Add( p );
          break;
        }
        case 'e':
        case 'E':
        case 'f':
        case 'g':
        case 'G':
        {
          char* eptr;
          Parameter p;
          p.type = TYPE_FLOAT;
          p.data = malloc( sizeof( double ) );
          *((double*) p.data) = strtod( scnr , &eptr );
          scnr = eptr;
          res->Add( p );
          break;

        }
        case 's':
        case 'S':
        {
          FXString str;
          Parameter p;
          while ( *scnr != c )
          {
            str += *scnr; scnr++;
          }
          p.type = TIPE_STRING;
          p.data = malloc( sizeof( char )*(str.GetLength() + 1) );
          strcpy( (char*) p.data , str );
          res->Add( p );
          break;
        }
        default:
          SENDERR_0( "Wrong format of InputFormat" );
          delete res;
          res = NULL;
          break;
      }
    }
  }
  if ( (res) && (res->GetSize() == 0) )
  {
    delete res;
    res = NULL;
  }
  return res;
}

IMPLEMENT_RUNTIME_GADGET_EX( TextFormat , CFilterGadget , "Text.conversion" , TVDB400_PLUGIN_NAME );

TextFormat::TextFormat( void )
{
  m_pOutput = new COutputConnector( text );
  m_pInput = new CInputConnector( text );
  Resume();
}

void TextFormat::ShutDown()
{
  CFilterGadget::ShutDown();
  delete m_pInput; m_pInput = NULL;
  delete m_pOutput; m_pOutput = NULL;
}

CDataFrame* TextFormat::DoProcessing( const CDataFrame* pDataFrame )
{
  CTextFrame *result = NULL;
  const CTextFrame *tf = pDataFrame->GetTextFrame( DEFAULT_LABEL );
  if ( !tf ) return NULL;
  ParamArray* pa = SScan( m_InputFormat , tf->GetString() );
  if ( pa )
  {
    FXString out = SPrint( m_OutputFormat , pa );
    result = CTextFrame::Create( out );
    result->CopyAttributes( pDataFrame );
  }
  delete pa;
  return result;
}

bool TextFormat::PrintProperties( FXString& text )
{
  FXPropertyKit pk;
  pk.WriteString( "InputFormat" , m_InputFormat );
  pk.WriteString( "OutputFormat" , m_OutputFormat );
  text = pk;
  return true;
}

bool TextFormat::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXPropertyKit pk( text );
  pk.GetString( "InputFormat" , m_InputFormat );
  pk.GetString( "OutputFormat" , m_OutputFormat );
  return true;
}

bool TextFormat::ScanSettings( FXString& text )
{
  text.Format( "template(EditBox(InputFormat),EditBox(OutputFormat))" );
  return true;
}
